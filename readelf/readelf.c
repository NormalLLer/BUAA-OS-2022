/* This is a simplefied ELF reader.
 * You can contact me if you find any bugs.
 *
 * Luming Wang<wlm199558@126.com>
 */

#include "kerelf.h"
#include <stdio.h>
/* Overview:
 *   Check whether it is a ELF file.
 *
 * Pre-Condition:
 *   binary must longer than 4 byte.
 *
 * Post-Condition:
 *   Return 0 if `binary` isn't an elf. Otherwise
 * return 1.
 */
int is_elf_format(u_char *binary)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
        if (ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
                ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
                ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
                ehdr->e_ident[EI_MAG3] == ELFMAG3) {
                return 1;
        }

        return 0;
}

/* Overview:
 *   read an elf format binary file. get ELF's information
 *
 * Pre-Condition:
 *   `binary` can't be NULL and `size` is the size of binary.
 *
 * Post-Condition:
 *   Return 0 if success. Otherwise return < 0.
 *   If success, output address of every section in ELF.
 */

/*
    Exercise 1.2. Please complete func "readelf". 
*/
int readelf(u_char *binary, int size)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;

        int Nr;
		int Nr2;

        Elf32_Phdr *phdr = NULL;
		Elf32_Phdr *phdr1 = NULL;
		Elf32_Phdr *phdr2 = NULL;

        u_char *ptr_ph_table = NULL;
        Elf32_Half ph_entry_count;
        Elf32_Half ph_entry_size;

		int flag = 1;
        // check whether `binary` is a ELF file.
        if (size < 4 || !is_elf_format(binary)) {
                printf("not a standard elf format\n");
                return -1;
        }

		ptr_ph_table = binary + ehdr->e_phoff;
		ph_entry_count = ehdr->e_phnum;  
		ph_entry_size = ehdr->e_phentsize;  
	
	    for (Nr = 0; Nr < ph_entry_count - 1; Nr++) {
				phdr1 = (Elf32_Phdr *)(ptr_ph_table + Nr * ph_entry_size);
				Elf32_Addr left1 = phdr1->p_vaddr;
				Elf32_Addr right1 = left1 + phdr1->p_memsz;
				for (Nr2 = Nr + 1; Nr2 < ph_entry_count; Nr2++) {
						phdr2 = (Elf32_Phdr *)(ptr_ph_table + Nr2 * ph_entry_size);
						Elf32_Addr left2 = phdr2->p_vaddr;
	   					Elf32_Addr right2 = left2 + phdr2->p_memsz;
						unsigned int page_addr = right1 & 0xfffff000;
						unsigned int page_addr2 = right2 & 0xfffff000;
						if ((left1 < right2 && left2 - right1 >= 4096) || 
								(left2 < right1 && left1 - right2 >= 4096)) {
								continue;
						} else if (left1 < right2 && right1 <= left2) {
								flag = 0;
								printf("Overlay at page va : 0x%x\n", page_addr);
								break;
						} else if (left1 < right2 && right1 > left2) {
								flag = 0;
								printf("Conflict at page va : 0x%x\n", page_addr);
								break;
						} else if (left2 < right1 && right2 <= left1) {
								flag = 0;
								printf("Overlay at page va : 0x%x\n", page_addr2);
								break;
						} else if (left2 < right1 && right2 > left1) {
								flag = 0;
								printf("Conflict at page va : 0x%x\n", page_addr2);
						}
				}
				if (!flag) return 0;		
		}

		for (Nr = 0; Nr < ph_entry_count; Nr++) {
				phdr = (Elf32_Phdr *)(ptr_ph_table + Nr * ph_entry_size);
				printf("%d:0x%x,0x%x\n", Nr, phdr->p_filesz, phdr->p_memsz);
		}

        return 0;
}

