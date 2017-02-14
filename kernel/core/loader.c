#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <elf.h>
#include <loader.h>
#include <task.h>
#include <mem_layout.h>
#include <stdbool.h>
#include <xlibs.h>

extern struct cfs_rq *runq;
extern struct task_struct *upt[MAX_USR_TASK];
char *exec = NULL;

void exit_elf(int idx)
{
	if (upt[idx] != NULL)
		upt[idx]->state = TASK_STOP_RUNNING;
}

void load_ramdisk()
{
	int tasknum = 0, i, size, buf0, buf1, buf2, buf3;
	char *pramdisk = (char *)(SCRATCH_BASE);
	task_entry ptr;
	char temp[64];

	tasknum = pramdisk[0] | (pramdisk[1] << 8) | (pramdisk[2] << 16) | (pramdisk[3] << 24);
	sprintf(temp, "\r\nuser task number : %d", tasknum);
	print_msg(temp);
	pramdisk += 4;

	for (i = 0; i < tasknum; i++) {
		size = pramdisk[0] | (pramdisk[1] << 8) | (pramdisk[2] << 16) | (pramdisk[3] << 24);
		sprintf(temp, "\r\nload_bin cnt : %d, size : %d", i, size);
		print_msg(temp);
		pramdisk += 4;
		ptr = (task_entry)load_elf(pramdisk, i);
		pramdisk += size;
	}
}

Elf32_Addr find_sym(const char* name, Elf32_Shdr* shdr, const char* strings, const char* src, char* dst)
{
    Elf32_Sym* syms = (Elf32_Sym*)(src + shdr->sh_offset);
    int i;
    for (i = 0; i < shdr->sh_size / sizeof(Elf32_Sym); i++) {
        if (xstrcmp(name, strings + syms[i].st_name) == 0) {
            return (Elf32_Addr)(dst + syms[i].st_value);
        }
    }
    return -1;
}
task_entry load_elf (char *elf_start, int idx)
{
    Elf32_Ehdr      *hdr     = NULL;
    Elf32_Phdr      *phdr    = NULL;
    Elf32_Shdr      *shdr    = NULL;
    Elf32_Sym       *syms    = NULL;
    char            *strings = NULL;
    char            *start   = NULL;
    char            *taddr   = NULL;
    task_entry 		entry   = NULL;
    int i = 0, j;
    char buff[32];

    hdr = (Elf32_Ehdr *) elf_start;

    exec = (char *)(USER_CODE_BASE + idx*USER_CODE_GAP);
    phdr = (Elf32_Phdr *)(elf_start + hdr->e_phoff);

    for (i = 0; i < hdr->e_phnum; ++i) {
            if (phdr[i].p_type != PT_LOAD) {
                    continue;
            }
            if (phdr[i].p_filesz > phdr[i].p_memsz) {
                    print_msg("load_elf:: p_filesz > p_memsz\n");
                    return 0;
            }
            if (!phdr[i].p_filesz) {
                    continue;
            }

            start = elf_start + phdr[i].p_offset;
            taddr = phdr[i].p_vaddr + exec;
	    /* copy program to 0x1600000 */
	    /*memmove(taddr,start,phdr[i].p_filesz);*/
	    for (j = 0; j < phdr[i].p_filesz; j++) 
		    taddr[j] = start[j];
    }
    shdr = (Elf32_Shdr *)(elf_start + hdr->e_shoff);

    for (i = 0; i < hdr->e_shnum; i++) {
	    if (shdr[i].sh_type == SHT_SYMTAB) {
		syms = (Elf32_Sym *)(elf_start + shdr[i].sh_offset);
		strings = elf_start + shdr[shdr[i].sh_link].sh_offset;
		entry = (task_entry)find_sym("main", shdr + i, strings, elf_start, exec);
		break;
	    }
    }

    /* to do page directory construction here */
    /* ismi*/
    unsigned int *ppd = 0x0;

/* user task is not inserted to rq. it should be called explicitly*/
    sprintf(buff,"user%d",idx);
#ifdef USE_MMU
    upt[idx]= do_forkyi(buff, (task_entry)entry, idx, ppd); 
#else
    upt[idx]= do_forkyi(buff, (task_entry)entry, idx,CFS_TASK);
#endif

    set_priority(upt[idx], 4);
    rb_init_node(&(upt[idx]->se).run_node);
    enqueue_se_to_runq(runq, &(upt[idx]->se), true);

    return entry;
}
