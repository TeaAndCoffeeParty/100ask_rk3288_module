KERN_DIR = /home/book/Documents/100ask_firefly-rk3288/linux-4.4

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o led_app led_app.c  -Wno-error=unused-but-set-variable

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f hello_drv_test


myled_drv-y:=led_drv.o board_A_led.o chip_demoo_gpio.o
obj-m += myled_drv.o

