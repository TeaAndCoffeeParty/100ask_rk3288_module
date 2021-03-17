#ifndef __LED_DRV_H_          
#define __LED_DRV_H_ 

void led_class_create_device(int index);
void led_class_destroy_device(int index);
void register_led_operations(struct led_operations *led_opr);


#endif
