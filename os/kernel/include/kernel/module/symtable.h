#pragma once

#include <stdint.h>
#include <kernel/misc/function.h>

#define SYMTABLE 0x1007000

#define GET_KERNEL_SYMBOL(id) ((sym_entry_t**)SYMTABLE)[id]
#define SEEK_KERNEL_SYMBOL(name) ((sym_entry_t*(*)(char[64]))(GET_KERNEL_SYMBOL(1)->addr))(name)

#define KERNEL_CALL_NOA(name,ftype) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)() //usage : KERNEL_CALL_A[n]("kernel_function",F_PTR_A[N](ret-type,param-type,...),arg,...)
#define KERNEL_CALL_A1(name,ftype,a) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)(a)
#define KERNEL_CALL_A2(name,ftype,a1,a2) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)(a1,a2)
#define KERNEL_CALL_A3(name,ftype,a1,a2,a3) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)(a1,a2,a3)
#define KERNEL_CALL_A4(name,ftype,a1,a2,a3,a4) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)(a1,a2,a3,a4)
#define KERNEL_CALL_A5(name,ftype,a1,a2,a3,a4,a5) ((ftype)SEEK_KERNEL_SYMBOL(name)->addr)(a1,a2,a3,a4,a5)

struct symtable_entry{
	uint32_t* addr;
	char name[64];
};

typedef struct symtable_entry sym_entry_t;

/*#define KERNEL_CALL_CHECK(func,name)\
 if(SEEK_KERNEL_SYMBOL(name) == 0){\
	KERNEL_CALL_A2("create_ksym",F_PTR_A2(sym_entry_t*,char[64],uint32_t*),name,func);\ //TODO
}*/

#ifdef KERNEL_MODULE
#define KERNEL_EXPORT_NOA(name,ftype) {\
	//KERNEL_CALL_CHECK(addr,name)
	KERNEL_CALL_NOA(name,ftype);\
}
 
#define KERNEL_EXPORT_A1(name,ftype,a) {\
	//KERNEL_CALL_CHECK(addr,name)
	KERNEL_CALL_A1(name,ftype,a);\
}

#define KERNEL_EXPORT_A2(name,ftype,a1,a2) {\
	//KERNEL_CALL_CHECK(addr,name)
	KERNEL_CALL_A2(name,ftype,a1,a2);\
}

#define KERNEL_EXPORT_A3(name,ftype,a1,a2,a3) {\
	//KERNEL_CALL_CHECK(addr,name)
	KERNEL_CALL_A3(name,ftype,a1,a2,a3);\
}

#define KERNEL_EXPORT_A4(name,ftype,a1,a2,a3,a4) {\
	//KERNEL_CALL_CHECK(addr,name)
	KERNEL_CALL_A4(name,ftype,a1,a2,a3,a4);\
} 

#define KERNEL_EXPORT_A5(name,ftype,a1,a2,a3,a4,a5) {\
	//KERNEL_CALL_CHECK(addr,name,ftype)
	KERNEL_CALL_A5(name,ftype,a1,a2,a3,a4,a5);\
}
#else
	#define KERNEL_EXPORT_NOA(name,ftype) ;
	#define KERNEL_EXPORT_A1(name,ftype,a) ;
	#define KERNEL_EXPORT_A2(name,ftype,a1,a2) ;
	#define KERNEL_EXPORT_A3(name,ftype,a1,a2,a3) ;
	#define KERNEL_EXPORT_A4(name,ftype,a1,a2,a3,a4) ;
	#define KERNEL_EXPORT_A5(name,ftype,a1,a2,a3,a4,a5) ;
#endif


sym_entry_t* create_ksym(char name[64],uint32_t* addr);
sym_entry_t* get_ksym(uint32_t id);
sym_entry_t* seek_ksym(char name[64]);

void init_symtable();


