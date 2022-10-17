all: qrng-client qrng-driver

qrng-client:
	$(MAKE) -C ./userspace

qrng-driver:
	$(MAKE) -C ./kernelspace

clean:
	$(MAKE) -C ./userspace clean
	$(MAKE) -C ./kernelspace clean