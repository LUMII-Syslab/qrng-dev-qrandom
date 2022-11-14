obj-m += qrng-driver.o 
 
PWD:=$(CURDIR) 
KDIR:=/lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install: all
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_DIR=extra modules_install
	modprobe qrng-driver
	echo "qrng-driver" > /etc/modules-load.d/qrng-driver.conf
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean