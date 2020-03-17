.PHONY: kernel modules test_exec usr clean

all: kernel modules test_exec usr

kernel:
	$(MAKE) -C kernel all
 
modules:
	$(MAKE) -C modules all 

test_exec:
	$(MAKE) -C test_exec all 
	
usr:
	$(MAKE) -C usr all 

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C modules clean 
	$(MAKE) -C usr clean 
	$(MAKE) -C test_exec clean 
install:
	$(MAKE) -C kernel install
	$(MAKE) -C modules install
	$(MAKE) -C test_exec install 
	$(MAKE) -C usr install 
install-headers:
	$(MAKE) -C kernel install-headers
