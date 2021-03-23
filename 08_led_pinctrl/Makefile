KERN_DIR = /home/chen/rk3288/100ask-rk3288/linux-4.4

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o led_app led_app.c  -Wno-error=unused-but-set-variable

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f hello_drv_test


obj-m += led_drv.o

