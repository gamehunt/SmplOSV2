/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/elf.h>
#include <kernel/module/symtable.h>

uint8_t   elf_check_format(elf32_hdr_t *hdr){
	if(!hdr) return 0;
	if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
		return 0;
	}
	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
		return 0;
	}
	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
		return 0;
	}
	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
		return 0;
	}
//	kinfo("Format passed\n");
	return 1;
}
uint8_t   elf_check_supported(elf32_hdr_t *hdr){
	if(!elf_check_format(hdr)){
		return 0;
	}
	if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {

		return 0;
	}
	if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {

		return 0;
	}
	if(hdr->e_machine != EM_386) {

		return 0;
	}
	if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {

		return 0;
	}
	if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {

		return 0;
	}
	return 1;
}
void *elf_get_symbol(elf32_hdr_t *target,char sname[64]){
	elf32_sect_hdr_t * sym_shdr;
	char * shstrtab = 0;
	char * symstrtab = 0;
	uint16_t i = 0;
	//INFO("0x%x %d 0x%x",target,sizeof(target),target[4305]);
	//INFO("%d\n",(unsigned int)target->e_shentsize * target->e_shnum/target->e_shentsize)
	for (unsigned int x = 0; x < (unsigned int)target->e_shentsize * target->e_shnum; x += target->e_shentsize) {
			elf32_sect_hdr_t * shdr = (elf32_sect_hdr_t *)((uintptr_t)target + (target->e_shoff + x));
			if (shdr->sh_type == SHT_SYMTAB) {
				sym_shdr = shdr;
			}
			if (i == target->e_shstrndx) {
				shstrtab = (char *)((uintptr_t)target + shdr->sh_offset);
			}
			i++;
		
	} 
	for (unsigned int x = 0; x < (unsigned int)target->e_shentsize * target->e_shnum; x += target->e_shentsize) {
			elf32_sect_hdr_t * shdr = (elf32_sect_hdr_t *)((uintptr_t)target + (target->e_shoff + x));
			if (shdr->sh_type == SHT_STRTAB && (!strcmp((char *)((uintptr_t)shstrtab + shdr->sh_name), ".strtab"))) {
				symstrtab = (char *)((uintptr_t)target + shdr->sh_offset);
			}
	}
	if (!sym_shdr || !shstrtab || !symstrtab) {
		kerr("Failed to find symbol '%s': symtable issue %d%d%d\n",sname,!sym_shdr,!shstrtab,!symstrtab);
		return 0;
	}
	elf32_sym_t * table = (elf32_sym_t *)((uintptr_t)target + sym_shdr->sh_offset);
	while ((uintptr_t)table - ((uintptr_t)target + sym_shdr->sh_offset) < sym_shdr->sh_size) {
		if (table->st_name) {
			char * name = (char *)((uintptr_t)symstrtab + table->st_name);

			if(!strcmp(name,sname)){
				return table;
			}
			
			
		}
		table++;
	}	
	return 0;
}
int   elf_get_symval(elf32_hdr_t *hdr, int table, uint32_t idx){
	if(table == SHN_UNDEF || idx == SHN_UNDEF) return 0;
	elf32_sect_hdr_t *symtab = elf_section(hdr, table);
	uint32_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
	if(idx >= symtab_entries) {
		kerr("Symbol Index out of Range (%d:%d).\n", table, idx);
		return ELF_RELOC_ERR;
	}
	int symaddr = (int)hdr + symtab->sh_offset;
	elf32_sym_t *symbol = &((elf32_sym_t *)symaddr)[idx];
	if(symbol->st_shndx == SHN_UNDEF) {
		// External symbol, lookup value
		elf32_sect_hdr_t *strtab = elf_section(hdr, symtab->sh_link);
		const char *name = (const char *)hdr + strtab->sh_offset + symbol->st_name;

		sym_entry_t *sym = symbol_seek(name);
		uint32_t target = 0;
		if(sym){
			target = (uint32_t)sym->addr;
		}
		if(target == 0) {
			// Extern symbol not found
			if(ELF32_ST_BIND(symbol->st_info) & STB_WEAK) {
				// Weak symbol initialized as 0
				return 0;
			} else {
				kerr("Undefined External Symbol : %s.\n", name);
				return ELF_RELOC_ERR;
			}
		} else {
			return (int)target;
		}
	} else if(symbol->st_shndx == SHN_ABS) {
		return symbol->st_value;
	} else {
		// Internally defined symbol
		elf32_sect_hdr_t *target = elf_section(hdr, symbol->st_shndx);
		return (int)hdr + symbol->st_value + target->sh_offset;
	}
}


int   elf_make_bss(elf32_hdr_t* hdr){
	elf32_sect_hdr_t *shdr = elf_sheader(hdr);
 
	unsigned int i;
	// Iterate over section headers
	for(i = 0; i < hdr->e_shnum; i++) {
		elf32_sect_hdr_t *section = &shdr[i];
 
		// If the section isn't present in the file
		if(section->sh_type == SHT_NOBITS) {
			// Skip if it the section is empty
			if(!section->sh_size) continue;
			// If the section should appear in memory
			if(section->sh_flags & SHF_ALLOC) {
				// Allocate and zero some memory
				void *mem = kmalloc(section->sh_size);
				memset(mem, 0, section->sh_size);
 
				// Assign the memory offset to the section offset
				section->sh_offset = (int)mem - (int)hdr;
				kinfo("Allocated memory for a section (%d).\n", section->sh_size);
			}
		}
	}
	return 0;
}
int   elf_make_relocations(elf32_hdr_t* hdr){
	elf32_sect_hdr_t *shdr = elf_sheader(hdr);
 
	unsigned int i, idx;
	// Iterate over section headers
	for(i = 0; i < hdr->e_shnum; i++) {
		elf32_sect_hdr_t *section = &shdr[i];
 
		// If this is a relocation section
		if(section->sh_type == SHT_REL) {
			// Process each entry in the table
			for(idx = 0; idx < section->sh_size / section->sh_entsize; idx++) {
				elf32_rel_t *reltab = &((elf32_rel_t *)((int)hdr + section->sh_offset))[idx];
				int result = elf_do_reloc(hdr, reltab, section);
				// On error, display a message and return
				if(result == ELF_RELOC_ERR) {
					kerr("Failed to relocate symbol.\n");
					return ELF_RELOC_ERR;
				}
			}
		}
	}
	return 0;
}
int   elf_do_reloc(elf32_hdr_t *hdr, elf32_rel_t *rel, elf32_sect_hdr_t *reltab){
	elf32_sect_hdr_t *target = elf_section(hdr, reltab->sh_info);
 
	int addr = (int)hdr + target->sh_offset;
	int *ref = (int *)(addr + rel->r_offset);
	int symval = 0;
	if(ELF32_R_SYM(rel->r_info) != SHN_UNDEF) {
		symval = elf_get_symval(hdr, reltab->sh_link, ELF32_R_SYM(rel->r_info));
		if(symval == ELF_RELOC_ERR) return ELF_RELOC_ERR;
	}
	switch(ELF32_R_TYPE(rel->r_info)) {
		case R_386_NONE:
			// No relocation
			break;
		case R_386_32:
			// Symbol + Offset
			*ref = DO_386_32(symval, *ref);
			break;
		case R_386_PC32:
			// Symbol + Offset - Section Offset
			*ref = DO_386_PC32(symval, *ref, (int)ref);
			break;
		default:
			// Relocation type not supported, display error and return
			kerr("Unsupported Relocation Type (%d).\n", ELF32_R_TYPE(rel->r_info));
			return ELF_RELOC_ERR;
	}
	return symval;
}
void *elf_load_reloc(elf32_hdr_t *hdr){
	int result;
	result = elf_make_bss(hdr);
	if(result == ELF_RELOC_ERR) {
		kerr("Unable to load ELF file.\n");
		return 0;
	}
	result = elf_make_relocations(hdr);
	if(result == ELF_RELOC_ERR) {
		kerr("Unable to load ELF file.\n");
		return 0;
	}
	return 1;
}

void *elf_load_exec(elf32_hdr_t* hdr){
	//kinfo("Loading exec\n");
	elf32_prog_hdr_t* prog_hdrs = (elf32_prog_hdr_t*)((uint32_t)hdr + hdr->e_phoff);
	//kinfo("Array at %a\n",prog_hdrs);
	for(uint16_t i = 0;i<hdr->e_phnum;i++){
	//	kinfo("HERE\n");
		elf32_prog_hdr_t ph = prog_hdrs[i];
		//kinfo("HERE %d\n",ph.p_type);
		if(ph.p_type == PH_LOAD){
			kinfo("Allocating memory for segment %a - %a\n",ph.p_vaddr,ph.p_vaddr + ph.p_memsz);
			for(uint32_t i = ph.p_vaddr;i<ph.p_vaddr + ph.p_memsz;i+=4096){	
				knpalloc(i); // Don't fucking work
				kinfo("%a - %a - %d(%d)\n",i,virtual2physical(i),ph.p_memsz,ph.p_filesz);
				//kinfo("%a\n",val);
			}
			memset((uint8_t*)ph.p_vaddr,0,ph.p_memsz);
			memcpy((uint8_t*)ph.p_vaddr,(uint8_t*)((uint32_t)hdr + ph.p_offset),ph.p_filesz);
		}
	}
	return hdr->e_entry;
}

void *elf_load_file(uint8_t *file){
	elf32_hdr_t *hdr = (elf32_hdr_t *)file;
	if(!elf_check_supported(hdr)) {
		kerr("ELF File cannot be loaded.\n");
		return 0;
	}
	switch(hdr->e_type) {
		case ET_EXEC:
			return elf_load_exec(hdr);
		case ET_REL:
			return elf_load_reloc(hdr);
	}
	return 0;
}
