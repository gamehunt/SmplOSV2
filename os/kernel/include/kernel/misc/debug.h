#pragma once

#define STOP \
	kerr("BREAKPOINT: in %s:%d in %s",__FILE__,__LINE__,__func__);\
	while(1);\
