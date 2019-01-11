%include "io.inc"

section .data
msg1: db 'OS_Lab_1 sample Fibonacci Sequence',0Ah,0

msg2: db 'Please input x and y: ',0

Fib0: db '0',0Ah,0

Fib1: db '1',0Ah,0

flag: db 0

newline:db 0Ah

color_red:      db  1Bh, '[31;1m', 0

color_green:    db  1Bh, '[32;1m', 0

color_purple:   db  1Bh, '[35;1m', 0

color_blue:     db  1Bh, '[34;1m', 0

color_default:  db  1Bh, '[37;0m', 0


section .bss
input_buff:     resb 16
start:          resb 4
end:            resb 4

n:   resb 404
n_1: resb 404
n_2: resb 404

section .text
global  CMAIN
 
CMAIN:
;PRINT PROMPT
    mov eax, msg1
    call strprint
    mov eax, msg2
    call strprint

;Input x and y
    mov eax , 3
    mov ebx , 0
    mov ecx , input_buff
    mov edx , 16
    int 80h
    
;HANDLE with inputs
    xor eax , eax
    mov ebx , input_buff
    xor dh  , dh
iter:  
       mov dl , [ebx]
       cmp dl , 48
       jb  reset
       mov cl , 10
       mul cl
       sub dl , 48
       add ax , dx
       inc ebx
       jmp iter
reset: 
       cmp dl , 32
       je  start_res
       mov [end],eax
       jmp print_nums
start_res:
       mov [start],eax
       xor eax,eax
       inc ebx
       jmp iter
print_nums:
    mov ebx,[start]
    mov edi,[end]
init:
    xor ecx,ecx
set_zero:
    shl ecx,2
    mov dword[n_2+ecx],0
    mov dword[n_1+ecx],0
    mov dword[n+ecx],0
    shr ecx,2
    inc ecx
    cmp ecx,100
    jna set_zero
    mov byte[n_1],1
    mov ecx,1
zero_one:
    cmp  ebx,1
    ja   pre_loop
    test ebx,ebx
    jnz  one
    call change_color
    mov  eax,Fib0
    call strprint
    inc  ebx
    jmp  zero_one
one:
    test edi,edi
    jz   quit
    call change_color
    mov  eax,Fib1
    call strprint
    inc  ebx
    cmp  ebx,edi
    ja   quit
pre_loop:
    inc  ecx
    call Fibo_pre
    cmp  ebx,ecx
    ja   pre_loop
Fibo_loop:
    cmp  ebx,edi
    ja   quit
    call printN
    call Fibo_pre
    inc  ebx
    jmp  Fibo_loop
    
;PRINT n in the stack
printN:
  call change_color
  call find_head
loop_digs:
      add  byte[n+esi],48
      lea  eax,[n+esi]
      call dig_print
      dec  esi
      cmp  esi,0
      jnl  loop_digs
  call nl
  ret

find_head:
      mov  esi, 404
sub_loop:  
      dec  esi
      mov  al ,byte[n+esi] 
      test al ,al
      jz   sub_loop
      ret       
                              
;n=n_1+n_2,n-2=n-1,n-1=n
Fibo_pre:
    xor  esi,esi
    xor  dh,dh
Fibo_op:    
    mov al,byte[n_1+esi]
    add al,byte[n_2+esi]
    add al,dh
    xor dh,dh
    cmp al,10
    jb  next_step
    sub al,10
    mov dh,1
next_step:
    cmp  esi,400
    ja   update
    mov  byte[n+esi],al
    inc  esi
    jmp  Fibo_op
update:
    mov  al,byte[n_1+esi]
    mov  byte[n_2+esi],al 
    mov  al,byte[n+esi] 
    mov  byte[n_1+esi],al
    dec  esi
    cmp  esi,0
    jnl  update
    ret   

;do something with flag
change_flag:
    cmp byte[flag],3
    je  back
    inc byte[flag]
    ret
back:
    mov byte[flag],0
    ret
   
;select color
change_color:
    mov  dl,[flag]
    test dl,dl
    jz   red
    cmp  dl,1
    je   green
    cmp  dl,2
    je   purple
    cmp  dl,3
    je   blue
red:
    mov  eax,color_red
    call strprint
    call change_flag
    ret

green:
    mov  eax,color_green
    call strprint
    call change_flag
    ret

purple:
    mov  eax,color_purple
    call strprint
    call change_flag
    ret

blue:
    mov  eax,color_blue
    call strprint
    call change_flag
    ret

;Print a string
strprint:
    push    edx
    push    ecx
    push    ebx
    push    eax        
    call    strlen          
    mov     edx, eax  
    pop     eax	
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     ebx
    pop     ecx
    pop     edx
    ret

;Print a dig
dig_print:
    push    edx
    push    ecx
    push    ebx                
    mov     edx, 1 	
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     ebx
    pop     ecx
    pop     edx
    ret
    
;Print a newline
nl:
    push    edx
    push    ecx
    push    ebx                
    mov     edx, 1 	
    mov     ecx, newline
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     ebx
    pop     ecx
    pop     edx
    ret
;EXIT SYSTEM CALL
quit:
    mov     ebx, 0
    mov     eax, 1
    int     80h

;获取字符串长度
strlen:                     
    push    ebx
    mov     ebx, eax
 
nextchar:
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar
 
finished:
    sub     eax, ebx
    pop     ebx 
    ret
