#ifndef __LOADER_H__
#define __LOADER_H__
#include <task.h>
/* ramdisk.img is already loaded to scratch region 0x2000000*/
void load_bin(int idx);
#ifdef LOAD_FROM_MMC
void load_bin_to_scratch_mem(char *buf);
#endif
task_entry load_elf (char *elf_start, int idx);
#endif
