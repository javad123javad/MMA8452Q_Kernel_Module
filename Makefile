PWD := $(shell pwd)
obj-m += mma8452q.o
CROSS = aarch64-linux-gnu-
KERNEL_DIR = /lib/modules/`uname -r`/build
all:
	make ARCH=arm64 CROSS_COMPILE=$(CROSS) -C $(KERNEL_DIR) M=$(PWD) 
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
