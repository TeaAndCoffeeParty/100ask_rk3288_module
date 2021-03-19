
KERN_DIR = /home/chen/rk3288/100ask-rk3288/linux-4.4/
CROSS_COMPILE=arm-linux-gnueabihf-


all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += button_drv.o 
