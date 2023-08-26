#ifndef _SLFS_H_
#define _SLFS_H_

#include <stdint.h>
#include <ramdisk_io.h>

#define SLFS_FNAME_LEN		(16)

typedef struct {
	uint32_t fd;
	uint32_t pos;
	uint32_t file_size;		/**< used for file update starts (OPEN evt) flag */
	uint32_t open_cnt;		
	uint32_t datablk_addr;		/**< used for file update ends (CLOSE evt) flag  */
	uint32_t inode_idx;
	uint32_t allocedblk_num;
	uint32_t update_cnt;		/**< only the biggest one is valid, others are outdated inode*/
	uint8_t name[SLFS_FNAME_LEN];
} slfs_file_t;

typedef enum {
	SLFS_SEEK_SET,
	SLFS_SEEK_CUR,
	SLFS_SEEK_END,
} slfs_fseek_t;

int slfs_mount(void);
int slfs_umount(void);
int slfs_format(void);
int slfs_open(const uint8_t *pname, slfs_file_t *pf);
int slfs_seek(slfs_file_t *pf, uint32_t offset, slfs_fseek_t whence);
int slfs_write(slfs_file_t *pf, const uint8_t *pbuf, uint32_t len);
int slfs_read(slfs_file_t *pf, uint8_t *pbuf, uint32_t len); 
int slfs_close(slfs_file_t *pf);
int slfs_delete(slfs_file_t *pf);
int slfs_get_next_file(slfs_file_t *pf, uint32_t *pinode_loc);
#endif
