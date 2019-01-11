
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++) {
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++) {
				if(p->sleep_milli_seconds>0){
					if(((get_ticks()-p->sleep_tick)* 1000 / HZ)>p->sleep_milli_seconds){
						p->sleep_milli_seconds=0;
						p->ticks = p->priority;
					}
				}
				else{
					p->ticks = p->priority;
				}
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_process_sleep(int milli_seconds, PROCESS* p_proc){
	p_proc->ticks=0;
	p_proc->sleep_tick=get_ticks();
	p_proc->sleep_milli_seconds=milli_seconds;
	schedule();
}

PUBLIC  void    sys_sem_p(SEMAPHORE* s, PROCESS* p_proc){
	s->value--;
	if(s->value<0){
		s->queue[-(s->value)-1]=p_proc;
		sys_process_sleep(100000, p_proc);
	}
}

PUBLIC  void    sys_sem_v(SEMAPHORE* s, PROCESS* p_proc){
	s->value++;
	if(s->value<=0){
		PROCESS* p=s->queue[0];
		p->sleep_milli_seconds=0;
		p->ticks = p->priority;
		for(int i=0;i<-(s->value);i++){
			s->queue[i]=s->queue[i+1];
		}
		p_proc_ready = p;
	}
}

PUBLIC  void    cuthair(int cur_id, unsigned int cur_color){
	if(cur_id==1)
		my_disp_str("Barber Online...\n");
	my_disp_str("The barber start to cut hair...\n");
	process_sleep(2000);
	tty_table[p_proc_ready->nr_tty].p_console->char_color=BRIGHT_BLUE;
	my_disp_str("The barber haircut done...\n");
	get_haircut(cur_id, cur_color);
}

PUBLIC  void    come_and_wait(int index,unsigned int color){
	char number[256];
	tty_table[p_proc_ready->nr_tty].p_console->char_color=color;
	itoa(number,index);
	my_disp_str("Customer ");
	my_disp_str(number);
	my_disp_str(" come and start waiting...\n");
}
PUBLIC  void    get_haircut(int index,unsigned int color){
	char number[256];
	tty_table[p_proc_ready->nr_tty].p_console->char_color=color;
	itoa(number,index);
	my_disp_str("Customer ");
	my_disp_str(number);
	my_disp_str(" get a haircut, leave...\n");
}

PUBLIC  void    leave(int index,unsigned int color){
	char number[256];
	tty_table[p_proc_ready->nr_tty].p_console->char_color=color;
	itoa(number,index);
	my_disp_str("Customer ");
	my_disp_str(number);
	my_disp_str(" come, no chair, leave...\n");
}