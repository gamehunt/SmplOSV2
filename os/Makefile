.PHONY: kernel modules test_exec clean

all: kernel modules test_exec

kernel:
	$(MAKE) -C kernel all
 
modules:
	$(MAKE) -C modules all 

test_exec:
	$(MAKE) -C test_exec all 

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C modules clean 
install:
	$(MAKE) -C kernel install
	$(MAKE) -C modules install
install-headers:
	$(MAKE) -C kernel install-headers
