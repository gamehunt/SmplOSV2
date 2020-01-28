.PHONY: kernel modules clean

all: kernel modules

kernel:
	$(MAKE) -C kernel all 
	$(MAKE) -C modules all 
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C modules clean 
install:
	$(MAKE) -C kernel install
	$(MAKE) -C modules install
install-headers:
	$(MAKE) -C kernel install-headers
