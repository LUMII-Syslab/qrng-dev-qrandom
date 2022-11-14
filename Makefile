all: 
	$(MAKE) -C ./kernelspace
	$(MAKE) -C ./userspace

install:
	$(MAKE) -C ./kernelspace install
	$(MAKE) -C ./userspace install

clean:
	$(MAKE) -C ./kernelspace clean
	$(MAKE) -C ./userspace clean