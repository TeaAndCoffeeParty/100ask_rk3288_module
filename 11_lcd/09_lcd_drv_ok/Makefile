KERN_DIR = /home/book/imx6ull/linux-4.9.88/

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += lcd_drv.o

