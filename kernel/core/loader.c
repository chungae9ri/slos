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
#include <elf.h>
#include <loader.h>
#include <task.h>
#include <mem_layout.h>
#include <string.h>
#include <error.h>

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

struct task_struct *upt[MAX_USR_TASK];
char *exec = NULL;

void exit_elf(uint32_t idx)
{
	if (upt[idx] != NULL)
		upt[idx]->state = TASK_STOP_RUNNING;
}

int32_t load_ramdisk_app(uint32_t app_idx)
{
	/* user elfs are loaded to the file system.
	 * read it to the buffer and load it to
	 */
	uint32_t base_addr;
	uint8_t fname[SLFS_FNAME_LEN];
	slfs_file_t fp;
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

	if (app_idx >= MAX_USR_TASK) {
		printk("err: user task idx overflow\n");
		return USER_APP_MAX_ERR;
	}
	sprintk((char *)fname, "App_%x", (unsigned int)app_idx);
	ret = slfs_open((const uint8_t *)fname, &fp);
	if (ret)
		return ret;

	ret = slfs_read(&fp, (uint8_t *)&ehdr, sizeof(ehdr));
	if (ret)
		return ret;

	base_addr = USER_APP_BASE + (USER_APP_GAP * app_idx);

	/* load all loadable segments to memory */
	for (i = 0; i < ehdr.e_phnum; i++) {
		offset = ehdr.e_phoff + i * ehdr.e_phentsize;
		ret = slfs_seek(&fp, offset, SLFS_SEEK_SET);
		if (ret)
			return ret;

		ret = slfs_read(&fp, (uint8_t *)&phdr, ehdr.e_phentsize);
		if (ret)
			return ret;

		if (phdr.p_type != PT_LOAD)
			continue;

		if (!phdr.p_filesz)
			continue;

		ret = slfs_seek(&fp, phdr.p_offset, SLFS_SEEK_SET);
		if (ret)
			return ret;

		dst_addr = base_addr + phdr.p_vaddr;
		ret = slfs_read(&fp, (uint8_t *)dst_addr, phdr.p_filesz);
		if (ret)
			return ret;
	}

	/* seek main entry from symbol table section */
	for (i = 0; i < ehdr.e_shnum; i++) {
		offset = ehdr.e_shoff + i * ehdr.e_shentsize;
		ret = slfs_seek(&fp, offset, SLFS_SEEK_SET);
		if (ret)
			return ret;
		
		ret = slfs_read(&fp, (uint8_t *)&shdr, ehdr.e_shentsize);
		if (ret)
			return ret;

		if (shdr.sh_type == SHT_SYMTAB) {
			/* sh_link is equal to the section hdr index of associated string table */
			offset = ehdr.e_shoff + shdr.sh_link * ehdr.e_shentsize;
			ret = slfs_seek(&fp, offset, SLFS_SEEK_SET);
			if (ret)
				return ret;

			ret = slfs_read(&fp, (uint8_t *)&str_shdr, ehdr.e_shentsize);
			if (ret)
				return ret;
			
			string_tab = (uint8_t *)kmalloc(str_shdr.sh_size);
			ret = slfs_seek(&fp, str_shdr.sh_offset, SLFS_SEEK_SET);
			if (ret)
				return ret;
			
			ret = slfs_read(&fp, string_tab, str_shdr.sh_size);
			if (ret)
				return ret;

			sym_tab = (uint8_t *)kmalloc(shdr.sh_size);
			ret = slfs_seek(&fp, shdr.sh_offset, SLFS_SEEK_SET);
			if (ret)
				return ret;

			ret = slfs_read(&fp, sym_tab, shdr.sh_size);
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

	ret = slfs_close(&fp);
	if (ret)
		return ret;

	/* TODO: build application's page translation table here.
	 *       for now, share the kernel page table.
	 */

	create_usr_cfs_task((char *)fname, (task_entry)entry, 4, app_idx);

	return NO_ERR;
}
