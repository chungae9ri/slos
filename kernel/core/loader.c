// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>
#include <string.h>

#include <printk.h>
#include <elf.h>
#include <loader.h>
#include <task.h>
#include <mem_layout.h>
#include <error.h>


struct task_struct *upt[MAX_USR_TASK];
char *exec = NULL;

void exit_elf(uint32_t idx)
{
	if (upt[idx] != NULL)
		upt[idx]->state = TASK_STOP_RUNNING;
}

int32_t load_ramdisk_app(FILE_SYSTEM_TYPE fs_t, uint32_t app_idx)
{
	/* user elfs are loaded to the file system.
	 * read it to the buffer and load it to
	 */
	uint32_t base_addr;
	uint8_t fname[SLFS_FNAME_LEN];
	file_t fp;
	int32_t ret;
	int32_t i;
	int32_t j;
	uint32_t offset;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	Elf32_Shdr shdr;
	Elf32_Shdr str_shdr;
	uint8_t *string_tab;
	uint8_t *sym_tab;
	uint32_t dst_addr;
	Elf32_Addr entry;
	Elf32_Sym *syms;
	uint32_t flag = 0;

	if (app_idx >= MAX_USR_TASK) {
		printk("err: user task idx overflow\n");
		return USER_APP_MAX_ERR;
	}

	fp.fs_t = fs_t;
	sprintk((char *)fname, "/App_%x", (unsigned int)app_idx);
	if (fs_t == LITTLEFS_FILE_SYSTEM)
		flag = LFS_O_RDONLY;

	ret = fs_open(fs_t, (const uint8_t *)fname, &fp, flag);
	if (ret)
		return ret;

	ret = fs_read(&fp, (uint8_t *)&ehdr, sizeof(ehdr));
	if (ret)
		return ret;

	base_addr = USER_APP_BASE + (USER_APP_GAP * app_idx);

	/* load all loadable segments to memory */
	for (i = 0; i < ehdr.e_phnum; i++) {
		offset = ehdr.e_phoff + i * ehdr.e_phentsize;
		ret = fs_seek(&fp, offset, SLFS_SEEK_SET);
		if (ret)
			return ret;

		ret = fs_read(&fp, (uint8_t *)&phdr, ehdr.e_phentsize);
		if (ret)
			return ret;

		if (phdr.p_type != PT_LOAD)
			continue;

		if (!phdr.p_filesz)
			continue;

		ret = fs_seek(&fp, phdr.p_offset, SLFS_SEEK_SET);
		if (ret)
			return ret;

		dst_addr = base_addr + phdr.p_vaddr;
		ret = fs_read(&fp, (uint8_t *)dst_addr, phdr.p_filesz);
		if (ret)
			return ret;
	}

	/* seek main entry from symbol table section */
	for (i = 0; i < ehdr.e_shnum; i++) {
		offset = ehdr.e_shoff + i * ehdr.e_shentsize;
		ret = fs_seek(&fp, offset, SLFS_SEEK_SET);
		if (ret)
			return ret;

		ret = fs_read(&fp, (uint8_t *)&shdr, ehdr.e_shentsize);
		if (ret)
			return ret;

		if (shdr.sh_type == SHT_SYMTAB) {
			/* sh_link is equal to the section hdr index of associated string table */
			offset = ehdr.e_shoff + shdr.sh_link * ehdr.e_shentsize;
			ret = fs_seek(&fp, offset, SLFS_SEEK_SET);
			if (ret)
				return ret;

			ret = fs_read(&fp, (uint8_t *)&str_shdr, ehdr.e_shentsize);
			if (ret)
				return ret;

			string_tab = (uint8_t *)kmalloc(str_shdr.sh_size);
			ret = fs_seek(&fp, str_shdr.sh_offset, SLFS_SEEK_SET);
			if (ret)
				return ret;

			ret = fs_read(&fp, string_tab, str_shdr.sh_size);
			if (ret)
				return ret;

			sym_tab = (uint8_t *)kmalloc(shdr.sh_size);
			ret = fs_seek(&fp, shdr.sh_offset, SLFS_SEEK_SET);
			if (ret)
				return ret;

			ret = fs_read(&fp, sym_tab, shdr.sh_size);
			if (ret)
				return ret;

			syms = (Elf32_Sym *)sym_tab;
			for (j = 0; j < shdr.sh_size / sizeof(Elf32_Sym); j++) {
				if (strcmp("main", (const char *)(string_tab + syms[j].st_name)) == 0) {
					entry = (Elf32_Addr)(base_addr + syms[j].st_value);
					break;
				}
			}
			break;
		}
	}

	kfree((uint32_t)string_tab);
	kfree((uint32_t)sym_tab);

	ret = fs_close(&fp);
	if (ret)
		return ret;

	/* TODO: build application's page translation table here.
	 *       for now, share the kernel page table.
	 */

	create_usr_cfs_task((char *)fname, (task_entry)entry, 4, app_idx);

	return NO_ERR;
}
