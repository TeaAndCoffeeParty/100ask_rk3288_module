#ifndef __LED_OPS_H_
#define __LED_OPS_H_

struct led_operations {
	int (*init)(int which);			/* init led, which:led num  */
	int (*ctl)(int which, char status);		/* control led, whic:led num,status:0-On 1-Off */
};

struct led_operations *get_board_led_ops(void);

#endif

