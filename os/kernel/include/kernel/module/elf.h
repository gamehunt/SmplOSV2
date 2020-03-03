/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <kernel/fs/vfs.h>

#define ELF_NIDENT	16

#define ELFMAG0	0x7F // e_ident[EI_MAG0]
#define ELFMAG1	'E'  // e_ident[EI_MAG1]
#define ELFMAG2	'L'  // e_ident[EI_MAG2]
#define ELFMAG3	'F'  // e_ident[EI_MAG3]
 
#define ELFDATA2LSB	(1)  // Little Endian
#define ELFCLASS32	(1)  // 32-bit Architecture

#define EM_386		(3)  // x86 Machine Type
#define EV_CURRENT	(1)  // ELF Current Version


#define SHN_UNDEF	(0x00)
#define SHN_ABS 		(0xfff1)

#define ELF32_ST_BIND(INFO)	((INFO) >> 4)
#define ELF32_ST_TYPE(INFO)	((INFO) & 0x0F)

#define ELF32_R_SYM(INFO)	((INFO) >> 8)
#define ELF32_R_TYPE(INFO)	((uint8_t)(INFO))

#define ELF_RELOC_ERR -1
#define DO_386_32(S, A)	((S) + (A))
#define DO_386_PC32(S, A, P)	((S) + (A) - (P))

typedef uint16_t elf32_half;	// Unsigned half int
typedef uint32_t elf32_off;	// Unsigned offset
typedef uint32_t elf32_addr;	// Unsigned address
typedef uint32_t elf32_word;	// Unsigned int
typedef int32_t  elf32_sword;	// Signed int

enum elf_ident {
	EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};

enum elf_type {
	ET_NONE		= 0, // Unkown Type
	ET_REL		= 1, // Relocatable File
	ET_EXEC		= 2  // Executable File
};

enum sht_types {
	SHT_NULL	= 0,   // Null section
	SHT_PROGBITS	= 1,   // Program information
	SHT_SYMTAB	= 2,   // Symbol table
	SHT_STRTAB	= 3,   // String table
	SHT_RELA	= 4,   // Relocation (w/ addend)
	SHT_NOBITS	= 8,   // Not present in file
	SHT_REL		= 9,   // Relocation (no addend)
};
 
enum sht_attrib {
	SHF_WRITE	= 0x01, // Writable section
	SHF_ALLOC	= 0x02  // Exists in memory
};

typedef struct {
	uint8_t		e_ident[ELF_NIDENT];
	elf32_half	e_type;
	elf32_half	e_machine;
	elf32_word	e_version;
	elf32_addr	e_entry;
	elf32_off	e_phoff;
	elf32_off	e_shoff;
	elf32_word	e_flags;
	elf32_half	e_ehsize;
	elf32_half	e_phentsize;
	elf32_half	e_phnum;
	elf32_half	e_shentsize;
	elf32_half	e_shnum;
	elf32_half	e_shstrndx;
} elf32_hdr_t;

typedef struct {
	elf32_word	sh_name;
	elf32_word	sh_type;
	elf32_word	sh_flags;
	elf32_addr	sh_addr;
	elf32_off	sh_offset;
	elf32_word	sh_size;
	elf32_word	sh_link;
	elf32_word	sh_info;
	elf32_word	sh_addralign;
	elf32_word	sh_entsize;
} elf32_sect_hdr_t;

typedef struct {
	elf32_word		st_name;
	elf32_addr		st_value;
	elf32_word		st_size;
	uint8_t			st_info;
	uint8_t			st_other;
	elf32_half		st_shndx;
} elf32_sym_t;

enum stt_bindings {
	STB_LOCAL		= 0, // Local scope
	STB_GLOBAL		= 1, // Global scope
	STB_WEAK		= 2  // Weak, (ie. __attribute__((weak)))
};
 
enum stt_types {
	STT_NOTYPE		= 0, // No type
	STT_OBJECT		= 1, // Variables, arrays, etc.
	STT_FUNC		= 2  // Methods or functions
};

typedef struct {
	elf32_addr		r_offset;
	elf32_word		r_info;
} elf32_rel_t;
 
typedef struct {
	elf32_addr		r_offset;
	elf32_word		r_info;
	elf32_sword		r_addend;
} elf32_rela_t;

enum rtt_Types {
	R_386_NONE		= 0, // No relocation
	R_386_32		= 1, // Symbol + Offset
	R_386_PC32		= 2  // Symbol + Offset - Section Offset
};
typedef struct {
	elf32_word		p_type;
	elf32_off		p_offset;
	elf32_addr		p_vaddr;
	elf32_addr		p_paddr;
	elf32_word		p_filesz;
	elf32_word		p_memsz;
	elf32_word		p_flags;
	elf32_word		p_align;
} elf32_prog_hdr_t;

static inline elf32_sect_hdr_t *elf_sheader(elf32_hdr_t *hdr) {
	return (elf32_sect_hdr_t *)((int)hdr + hdr->e_shoff);
}
static inline elf32_sect_hdr_t *elf_section(elf32_hdr_t *hdr, int idx) {
	return &elf_sheader(hdr)[idx];
}
static inline char *elf_str_table(elf32_hdr_t *hdr) {
	if(hdr->e_shstrndx == SHN_UNDEF) return 0;
	return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}
static inline char *elf_lookup_string(elf32_hdr_t *hdr, int offset) {
	char *strtab = elf_str_table(hdr);
	if(strtab == 0) return 0;
	return strtab + offset;
}

uint8_t  elf_check_format(elf32_hdr_t *hdr);
uint8_t  elf_check_supported(elf32_hdr_t *hdr);

int   elf_get_symval(elf32_hdr_t *hdr, int table, uint32_t idx);
void* elf_get_symbol(elf32_hdr_t *hdr,char name[64]);

int   elf_make_bss(elf32_hdr_t* hdr);
int   elf_make_relocations(elf32_hdr_t* hdr);
int   elf_do_reloc(elf32_hdr_t *hdr, elf32_rel_t *rel, elf32_sect_hdr_t *reltab);
void *elf_load_reloc(elf32_hdr_t *file);
void *elf_load_file(uint8_t  *file);

