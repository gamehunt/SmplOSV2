/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#pragma once

#if !defined(ARCH) || !defined(__smplos_kernel) || !defined(__ARCH)
#error Invalid macro environment
#endif

#if __ARCH != i386
#error Invalid compilation architecture	
#endif


//HEH
#ifdef K_COMPILATION_FAILURE
#error ???? ??????? ??????? ??????????????? (kernel.o + 0x214)
#endif
