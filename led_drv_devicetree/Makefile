KERN_DIR = /home/chen/PrintOS/pro_kernel/Ray_Kernel-master

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o led_app led_app.c  -Wno-error=unused-but-set-variable

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f hello_drv_test


obj-m += chip_demoo_gpio.o led_drv.o

