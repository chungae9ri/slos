// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_fs File system
 * @{
 *
 * @file
 *
 * @brief File system interface
 *
 */

#include <error.h>
#include <fs.h>
#include <mm.h>
#include <printk.h>

static struct lfs_config cfg;
static lfs_t lfs;

static int lfs_api_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer,
			lfs_size_t size)
{
	size_t offset = block * c->block_size + off;

	return io_ops.read(offset, size, buffer);
}

static int lfs_api_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
			const void *buffer, lfs_size_t size)
{
	size_t offset = block * c->block_size + off;

	return io_ops.write(offset, size, buffer);
}

static int lfs_api_erase(const struct lfs_config *c, lfs_block_t block)
{
	return io_ops.erase_page(block);
}

static int lfs_api_sync(const struct lfs_config *c)
{
	return 0;
}

static int init_littlefs(void)
{
	cfg.read_size = 1024;
	cfg.prog_size = 1024;
	cfg.block_size = 0x10000;
	cfg.block_count = 64;
	cfg.block_cycles = -1;
	cfg.cache_size = 1024;
	cfg.lookahead_size = 1024;

	cfg.read = lfs_api_read;
	cfg.prog = lfs_api_prog;
	cfg.erase = lfs_api_erase;
	cfg.sync = lfs_api_sync;

	return NO_ERR;
}

int32_t fs_mount(FILE_SYSTEM_TYPE fs)
{
	int32_t ret;

	io_ops.erase_chip();

	if (fs == LITTLEFS_FILE_SYSTEM) {
		init_littlefs();
		ret = lfs_format(&lfs, &cfg);
		if (ret)
			return ret;

		ret = lfs_mount(&lfs, &cfg);
		if (ret)
			return ret;
	} else {
		ret = slfs_mount();

		if (ret) {
			slfs_format();
			slfs_mount();
		}
	}

	return NO_ERR;
}

int32_t fs_open(FILE_SYSTEM_TYPE fs_t, const uint8_t *name, file_t *fp, uint32_t flag)
{
	int32_t ret;

	if (fs_t == LITTLEFS_FILE_SYSTEM) {
		fp->virt_fp = kmalloc(sizeof(lfs_file_t));
		ret = lfs_file_open(&lfs, (lfs_file_t *)fp->virt_fp, (const char *)name, (int)flag);
	} else {
		fp->virt_fp = kmalloc(sizeof(slfs_file_t));
		ret = slfs_open(name, (slfs_file_t *)fp->virt_fp);
	}

	fp->fs_t = fs_t;

	return ret;
}

int32_t fs_read(file_t *fp, uint8_t *buff, size_t len)
{
	int32_t ret;

	if (fp->fs_t == LITTLEFS_FILE_SYSTEM) {
		ret = (int32_t)lfs_file_read(&lfs, (lfs_file_t *)fp->virt_fp, buff, len);
		if (ret == len)
			return 0;
		else
			return -1;
	} else
		ret = slfs_read((slfs_file_t *)fp->virt_fp, buff, len);

	return ret;
}

int32_t fs_write(file_t *fp, const uint8_t *buff, size_t len)
{
	int32_t ret;

	if (fp->fs_t == LITTLEFS_FILE_SYSTEM) {
		ret = (int32_t)lfs_file_write(&lfs, (lfs_file_t *)fp->virt_fp, buff, len);
		if (ret == len)
			return 0;
		else
			return -1;
	} else
		ret = slfs_write((slfs_file_t *)fp->virt_fp, buff, len);

	return ret;
}

int32_t fs_seek(file_t *fp, uint32_t offset, int whence)
{
	int32_t ret;

	if (fp->fs_t == LITTLEFS_FILE_SYSTEM) {
		lfs_file_seek(&lfs, (lfs_file_t *)fp->virt_fp, offset, whence);
		ret = 0;
	} else
		ret = slfs_seek((slfs_file_t *)fp->virt_fp, offset, (slfs_fseek_t)whence);

	return ret;
}

int32_t fs_close(file_t *fp)
{
	int32_t ret;

	if (fp->fs_t == LITTLEFS_FILE_SYSTEM)
		ret = (int32_t)lfs_file_close(&lfs, (lfs_file_t *)fp->virt_fp);
	else
		ret = slfs_close((slfs_file_t *)fp->virt_fp);

	if (!ret)
		kfree((uint32_t)(fp->virt_fp));

	return ret;
}

int32_t fs_umount(FILE_SYSTEM_TYPE fs)
{
	int32_t ret;

	if (fs == LITTLEFS_FILE_SYSTEM)
		ret = (int32_t)lfs_unmount(&lfs);
	else
		ret = slfs_umount();

	return ret;
}

extern uint32_t RAMDISK_PHY_START;

int32_t create_ramdisk_fs(FILE_SYSTEM_TYPE fs_t)
{
	char fname[16];
	uint32_t i;
	uint32_t offset;
	uint32_t app_cnt;
	uint32_t app_addr;
	uint32_t app_len;
	file_t file;
	int32_t ret;
	uint32_t flag;

	fs_mount(fs_t);

	offset = 0;
	app_cnt = *((uint32_t *)&RAMDISK_PHY_START);
	offset += sizeof(uint32_t);

	for (i = 0; i < app_cnt; i++) {
		app_len = *((uint32_t *)((uint32_t)(&RAMDISK_PHY_START) + offset));
		offset += sizeof(uint32_t);
		app_addr = (uint32_t)(&RAMDISK_PHY_START) + offset;

		sprintk(fname, "/App_%x", (unsigned int)i);
		if (fs_t == LITTLEFS_FILE_SYSTEM)
			flag = LFS_O_CREAT | LFS_O_RDWR;
		else
			flag = 0;

		ret = fs_open(fs_t, (const uint8_t *)fname, &file, flag);
		if (ret)
			return ret;

		ret = fs_write(&file, (const uint8_t *)app_addr, app_len);
		if (ret)
			return ret;

		ret = fs_close(&file);
		if (ret)
			return ret;

		offset += app_len;
	}

	return NO_ERR;
}

/**
 * @}
 * @}
 *
 */
