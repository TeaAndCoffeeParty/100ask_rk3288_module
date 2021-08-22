
KERN_DIR = /home/book/imx6ull/linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules
	arm-linux-gnueabihf-gcc -o ap3216_test ap3216_test.c

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f ap3216_test

obj-m += ap3216_driver.o ap3216_client.o
