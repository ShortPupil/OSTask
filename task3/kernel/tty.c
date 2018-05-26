
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
                                                    Zhong， 2018
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

char recordEsc;		//n为正常状态，s为查找状态，l为屏蔽状态
int oneline;		//未满的一行的字符数
int spacenum;		//tab应该打印的空格数

int loc[1024];
int spaces[1024];	
int index;

int finder;		//光标，搜索开始
extern void EscFindString(TTY* p_tty, int finder, int length);	//搜索字符串
extern void FinishFindString(TTY* p_tty, int finder);		//结束搜索

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

        //初始化那些值
        index=0;
	recordEsc='n';
	int refresh=0; //清屏的

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);

	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);

                        //进行清屏
			if(is_current_console(p_tty->p_console)){
				if(recordEsc=='n'){
					if(refresh<4000){  //调用milli_delay
						while (p_tty->p_console->cursor != 0) {
   							out_char(p_tty->p_console,'\b');
 						}
                                                p_tty->p_console->cursor = 0;
						refresh=0;
					}else{
						milli_delay(1);
						refresh++;
					}
				}
			}
			//
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};
	int space;

        if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
        }
        else {
                int raw_code = key & MASK_RAW;
                switch(raw_code) {
                case ENTER:
                        //需要加入对esc模式的判断
			if(recordEsc=='s'){			//搜索模式变为锁定模式
				recordEsc='l';
				//搜索
				EscFindString(p_tty, finder, p_tty->p_console->cursor-finder);
			}else{
				put_key(p_tty, '\n');
			}
			break;
		//加入ESC
		case ESC:
			if(recordEsc=='n'){			//等于第一次按esc
				recordEsc='s';
				//记录光标
				finder=p_tty->p_console->cursor;
			}else if(recordEsc=='l'){		//第二次按esc
				recordEsc='n';
				//结束搜索
				FinishFindString(p_tty, finder);
			}
			break;

		//加入TAB
		case TAB:
			oneline=p_tty->p_console->cursor%SCREEN_WIDTH; //保证纵向对齐
			if(SCREEN_WIDTH-oneline<4){
				spacenum=SCREEN_WIDTH-oneline;
			}else{
				spacenum=4-oneline%4;
			}
			for(int i=0;i<spacenum;i++){
				put_key(p_tty , ' ');
			}
			
			//保存此tab信息
			loc[index]=p_tty->p_console->cursor;
			spaces[index]=spacenum;
			index++;			
			
			break;
                
                //以下同原来
                case BACKSPACE:
			put_key(p_tty, '\b');
			break;
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
                        }
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;



		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				select_console(raw_code - F1);
			}
			break;
                default:
                        break;
                }
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}


