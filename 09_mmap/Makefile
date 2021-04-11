KERN_DIR = /home/book/Documents/100ask_firefly-rk3288/linux-4.4
APP_NAME = mmap_test

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o app_test $(APP_NAME).c  -Wno-error=unused-but-set-variable

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f app_test


obj-m += mmap_drv.o

