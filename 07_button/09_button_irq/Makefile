
KERN_DIR = /home/book/Documents/100ask_firefly-rk3288/linux-4.4

all:
	make -C $(KERN_DIR) M=`pwd` modules
#	$(CROSS_COMPILE)gcc -o button_test button_test.c

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
#	rm -f button_test

obj-m += button_irq.o
