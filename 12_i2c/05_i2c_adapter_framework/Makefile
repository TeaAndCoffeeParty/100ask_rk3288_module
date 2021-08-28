KERN_DIR=/home/book/Documents/100ask_firefly-rk3288/linux-4.4

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += i2c_adapter_drv.o
