.PHONY: kernel modules clean

all: kernel modules

kernel:
	$(MAKE) -C kernel all 
	
clean:
	$(MAKE) -C kernel clean

install:
	$(MAKE) -C kernel install

install-headers:
	$(MAKE) -C kernel install-headers
