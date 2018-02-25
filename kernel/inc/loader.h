#ifndef __LOADER_H__
#define __LOADER_H__
#include <task.h>

#define MAX_USR_TASK	5

int32_t load_ramdisk_img(void);
#if 0
void load_bin(int idx);
task_entry load_elf (char *elf_start, int idx);
void load_ramdisk();
void exit_elf(int idx);
#endif
#endif
