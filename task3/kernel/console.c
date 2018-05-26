
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
                                                    Zhong, 2018
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

extern int loc[1024];	//按下tab或enter的末尾位置
extern int spaces[1024];	//tab和esc包含的空格数
extern int index;	//索引
extern char recordEsc;		//n为正常状态，s为查找状态，l为屏蔽状态

extern void EscFindString(TTY* p_tty, int finder, int length);	//搜索字符串

extern void FinishFindString(TTY* p_tty, int finder);		//结束搜索


void EscFindString(TTY* p_tty, int finder, int length){
	int thecursor = p_tty->p_console->original_addr;//当前光标
	u8* p_vmemesc = (u8*)(V_MEM_BASE + finder * 2);

	while(thecursor<=finder-length){
		u8* p_vmem = (u8*)(V_MEM_BASE + thecursor * 2);
		//是否找到,用j表示
		int j=1;
		for(int i=0;i<length;i++){
			if(*(p_vmemesc+2*i)!=*(p_vmem+2*i)){
				j=0;
				break;
			}
		}
		//找到后处理
		if(j){
			for(int i=0;i<length;i++)  *(p_vmem+1+2*i) = BLUE;			
		}
		thecursor++;
	}
}

void FinishFindString(TTY* p_tty, int finder){
        //删除查找的finder位置后的字母 变回原来颜色
	while (p_tty->p_console->cursor != finder) {
   		out_char(p_tty->p_console,'\b');
 	}
	int thecursor = p_tty->p_console->original_addr;
	while(thecursor<=finder){
		u8* p_vmem = (u8*)(V_MEM_BASE + thecursor * 2);
		*(p_vmem+1) = DEFAULT_CHAR_COLOR;
		thecursor++;
	}
}

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

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;

                //清屏
		while (p_tty->p_console->cursor != 0) {
   			out_char(p_tty->p_console,'\b');
 		}
		p_tty->p_console->cursor = 0;
		//
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
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

	switch(ch) {//操作起来
	case '\n': //换行的话，要保存本行enter信息
		if (p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH > p_con->cursor) {
                        loc[index]=p_con->cursor;//保存位置信息
			
                        p_con->cursor = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) /SCREEN_WIDTH + 1); //位置前移动
                        spaces[index]=p_con->cursor - loc[index];
			index++;	
		}
		break;
	case '\b': //退格的话
		if (p_con->cursor > p_con->original_addr) {

			
			if(index>0 && p_con->cursor==loc[index-1]+spaces[index-1]){ //退格包含换行时候保存的位置，光标返回之前的位置
				p_con->cursor -= spaces[index-1];
				index--;
			}else{ //同原来
				p_con->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}

		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;

			//这部分颜色设置要根据是否处于esc功能下
			if(recordEsc=='n' || recordEsc=='l'){
				*p_vmem++ = DEFAULT_CHAR_COLOR;
			}else{
				*p_vmem++ = BLUE;
			}

			p_con->cursor++;
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

