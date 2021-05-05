
KERN_DIR = /home/book/Documents/100ask_firefly-rk3288/linux-4.4

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc pca9685_test.c -o pca9685_test 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += pca9685_driver.o
