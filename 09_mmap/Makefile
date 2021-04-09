KERN_DIR = /home/book/Documents/100ask_firefly-rk3288/linux-4.4
APP_NAME = hello_drv_test

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o $(APP_NAME) $(APP_NAME).c  -Wno-error=unused-but-set-variable

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f $(APP_NAME)


obj-m += mmap_drv.o

