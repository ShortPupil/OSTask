
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	struct proc	proc_table[NR_TASKS + NR_PROCS];

PUBLIC	struct task	task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "TTY"},
	{task_sys, STACK_SIZE_SYS, "SYS"},
        {TestA, STACK_SIZE_TESTA, "PROCA"}};

PUBLIC	struct task	user_proc_table[NR_PROCS] = {
	{TestB, STACK_SIZE_TESTC, "CustomerA"},
        {TestC,STACK_SIZE_TESTD,"CustomerB"},
        {TestD,STACK_SIZE_TESTE,"CustomerC"},
	{TestE, STACK_SIZE_TESTB, "Barber"}
        };
 
PUBLIC int current_proc = -1;
PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_printx,
						       sys_sendrec,sys_process_sleep,sys_tem_p,sys_tem_v};


PUBLIC int waiting = 0;
PUBLIC int CHAIR = 3;

PUBLIC struct semaphore mutex;
PUBLIC struct semaphore customers;
PUBLIC struct semaphore barbers;