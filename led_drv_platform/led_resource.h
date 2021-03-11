#ifndef _LED_RESOUCE_H
#define _LED_RESOUCE_H

/* bit[31:16] = group */
/* bit[15:0] = which pin */
#define GROUP(x)    (x>>16)
#define PIN(x)      (x&0xffff)
#define GROUP_PIN(g, p) ((g<<16| (p)))

struct resource *get_led_resource(void);

#endif

