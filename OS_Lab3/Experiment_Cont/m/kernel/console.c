
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);


/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
    p_tty->p_console->search_flag        = 0;
	p_tty->p_console->key_len            = 0;
	p_tty->p_console->clear_flag         = 0;

	for(int i=0;i<20;i++)
		p_tty->p_console->key[i]='\0';
	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		disp_pos=0;
		for(int i=0;i<80*25;i++)
			disp_str(" ");
		disp_pos = 0;
		p_tty->p_console->cursor = p_tty->p_console->original_addr;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
	p_tty->p_console->start_tick         = get_ticks();
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
	if(p_con->search_flag==0){
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
			*p_vmem = '\0';
			*(p_vmem+1) = DEFAULT_CHAR_COLOR;
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
	}}
	else if(p_con->search_flag==1){
		p_con->search_flag=2;
        display_key(p_con,1);		
	}
		break;
	case '\b':
	    if(p_con->search_flag!=2){
		if (p_con->cursor > p_con->original_addr) {
			if(*(p_vmem-1) == HIDE_CHAR_COLOR){
				for(int i=0;i<8;i++){
					p_con->cursor--;
			        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
					p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
					if(*(p_vmem-1) != HIDE_CHAR_COLOR)
						break;
				}
			}
			else if((p_con->cursor-p_con->original_addr)%SCREEN_WIDTH!=0){
				p_con->cursor--;
			    *(p_vmem-2) = ' ';
			    *(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}
			else{
				int index=checkLine(p_con,(p_con->cursor - p_con->original_addr) /SCREEN_WIDTH - 1);
				if(index==-1){
					p_con->cursor--;
			        *(p_vmem-2) = ' ';
			        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
				}
				else{
					u8* end = (u8*)(V_MEM_BASE + index * 2);
					*end = ' ';
			        *(end+1) = DEFAULT_CHAR_COLOR;
					p_con->cursor=index;
				}
			}
		}}
		break;
	case '\t':
		if(p_con->search_flag==0){
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit) {
				int hole = 8 - (p_con->cursor - p_con->original_addr) % 8;
				for(int i=0;i<hole;i++){
					p_con->cursor++;
					*p_vmem = ' ';
			        *(p_vmem+1) = HIDE_CHAR_COLOR;
					p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
				}				
		}}
		break;
	case '\e':
		if(p_con->search_flag==0)
			p_con->search_flag=1;
		else{
			if(p_con->search_flag==2)
				display_key(p_con,0);
			p_con->search_flag=0;
			for(int i=0;i<p_con->key_len;i++){
				p_con->cursor--;
			    *(p_vmem-2) = ' ';
			    *(p_vmem-1) = DEFAULT_CHAR_COLOR;
				p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
				p_con->key[i]='\0';
			}
			p_con->key_len=0;
		}
		break;
	default:
	if(p_con->search_flag==0){
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
		}
	}
	else if(p_con->search_flag==1){
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = BRIGHT_BLUE;
			p_con->cursor++;
			p_con->key[p_con->key_len]=ch;
			p_con->key_len++;
		}
	}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}
PUBLIC int checkLine(CONSOLE* p_con,int line)
{
	int index = p_con->original_addr + SCREEN_WIDTH * line;
	for(int i=0;i<SCREEN_WIDTH;i++){
		u8* p_vmem = (u8*)(V_MEM_BASE + index * 2);
		if(*p_vmem == '\0')
			return index;
		index++;
	}
	return -1;
}

PUBLIC void display_key(CONSOLE* p_con,int status){
	if(status==1){
	int match=1;
	u8* head;
	for(int i=p_con->original_addr;i<p_con->cursor-p_con->key_len*2;i++){
		head = (u8*)(V_MEM_BASE + i * 2);
		for(int j=0;j<p_con->key_len;j++){
			if(*(head+j*2)!=p_con->key[j]){
				match=0;
			}
		}
		if(match==1){
			for(int j=0;j<p_con->key_len;j++){
				*(head+j*2+1)=BRIGHT_BLUE;
			}
		}else{
			match=1;
		}
	}}
	else if(status==0){
	int match=1;
	u8* head;
	for(int i=p_con->original_addr;i<p_con->cursor-p_con->key_len*2;i++){
		head = (u8*)(V_MEM_BASE + i * 2);
		for(int j=0;j<p_con->key_len;j++){
			if(*(head+j*2)!=p_con->key[j]){
				match=0;
			}
		}
		if(match==1){
			for(int j=0;j<p_con->key_len;j++){
				*(head+j*2+1)=DEFAULT_CHAR_COLOR;
			}
		}else{
			match=1;
		}
	}}
}

PUBLIC void clear_screen(CONSOLE* p_con){
	p_con->clear_flag=0;
	if(p_con->search_flag==0){
		u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
		while(p_con->cursor!=p_con->original_addr){
			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
		}
		flush(p_con);
	}
}