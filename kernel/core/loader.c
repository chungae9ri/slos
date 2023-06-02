/*
  kernel/core/loader.c elf loader
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/
#include <stdint.h>
#include <printk.h>
#include <slfs.h>
#if 0
#include <elf.h>
#include <loader.h>
#include <task.h>
#include <mem_layout.h>
#include <string.h>
#endif

extern uint32_t RAMDISK_PHY_START;

int32_t create_ramdisk_fs(void)
{
	char fname[16];
	uint32_t i; 
	uint32_t offset;
	uint32_t app_cnt;
	uint32_t app_addr;
	uint32_t app_len;
	slfs_file_t fp;
	int32_t ret;

	offset = 0;
	app_cnt = *((uint32_t *)&RAMDISK_PHY_START);
	offset += sizeof(uint32_t);

	for (i = 0; i < app_cnt; i++) {
		app_len = *((uint32_t *)((uint32_t)(&RAMDISK_PHY_START) + offset));
		offset += sizeof(uint32_t);
		app_addr = (uint32_t)(&RAMDISK_PHY_START) + offset;

		sprintk(fname, "App_%x", (unsigned int)i);
		ret = slfs_open((const uint8_t *)fname, &fp);
		if (ret)
			return ret;

		ret = slfs_write(&fp, (uint8_t *)app_addr, app_len);
		if (ret)
			return ret;

		ret = slfs_close(&fp);
		if (ret)
			return ret;
		offset += app_len;
	}

	return NO_ERR;
}

#if 0
struct task_struct *upt[MAX_USR_TASK];
char *exec = NULL;

void exit_elf(uint32_t idx)
{
	if (upt[idx] != NULL)
		upt[idx]->state = TASK_STOP_RUNNING;
}

#define APP_LOAD_OFFSET		0x800000

void load_ramdisk_app(uint32_t appIdx)
{
	/* 
	 * ramdisk.img is already loaded to 0x3000000 by the bootloader.
	 * load_ramdisk_img create ramdisk file system.
	 */
	char *app_load_addr;
	char temp[16];
	struct file *fp;

	if (appIdx >= MAX_USR_TASK) {
		printk("err: user task idx overflow\n");
		return;
	}


	app_load_addr = (char *)(USER_APP_BASE + 
			APP_LOAD_OFFSET + 
			USER_APP_GAP * appIdx);

	sprintk(temp,"App_%u", (unsigned int)appIdx);
	fp = find_file_by_name(temp);
	if (fp) {
		read(fp, fp->fsz, app_load_addr);
		load_elf(app_load_addr, appIdx);
	}
}

Elf32_Addr find_sym(const char* name, 
		Elf32_Shdr* shdr, 
		const char* strings, 
		const char* src, 
		char* dst)
{
	int i;

	Elf32_Sym* syms = (Elf32_Sym*)(src + shdr->sh_offset);
	for (i = 0; i < shdr->sh_size / sizeof(Elf32_Sym); i++) {
		if (strcmp(name, strings + syms[i].st_name) == 0) {
			return (Elf32_Addr)(dst + syms[i].st_value);
		}
	}

	return -1;
}

task_entry load_elf(char *elf_start, uint32_t idx)
{
	Elf32_Ehdr      *hdr     = NULL;
	Elf32_Phdr      *phdr    = NULL;
	Elf32_Shdr      *shdr    = NULL;
	/*Elf32_Sym       *syms    = NULL;*/
	char            *strings = NULL;
	char            *start   = NULL;
	char            *taddr   = NULL;
	int i = 0, j;
	char buff[32];
	task_entry 		entry   = NULL;

	hdr = (Elf32_Ehdr *) elf_start;

	exec = (char *)(USER_APP_BASE + idx * USER_APP_GAP);
	phdr = (Elf32_Phdr *)(elf_start + hdr->e_phoff);

	for (i = 0; i < hdr->e_phnum; ++i) {
		if (phdr[i].p_type != PT_LOAD) {
			continue;
		}
		if (phdr[i].p_filesz > phdr[i].p_memsz) {
			printk("load_elf:: p_filesz > p_memsz\n");
			return 0;
		}
		if (!phdr[i].p_filesz) {
			continue;
		}

		start = elf_start + phdr[i].p_offset;
		taddr = phdr[i].p_vaddr + exec;
		for (j = 0; j < phdr[i].p_filesz; j++) 
			taddr[j] = start[j];
	}
	shdr = (Elf32_Shdr *)(elf_start + hdr->e_shoff);

	for (i = 0; i < hdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_SYMTAB) {
			/*syms = (Elf32_Sym *)(elf_start + shdr[i].sh_offset);*/
			strings = elf_start + shdr[shdr[i].sh_link].sh_offset;
			entry = (task_entry)find_sym("main", shdr + i, strings, elf_start, exec);
			break;
		}
	}

	/* 
	 * to do: build page translation table here. 
	 * For now, share the kernel page table.
	 */

	sprintk(buff,"user_%u",(unsigned int)idx);
	create_usr_cfs_task(buff, (task_entry)entry, 4, idx);

	return entry;
}
#endif
