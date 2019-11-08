#define CTRL_GBL_START_MASK		(0x1)
#define CTRL_INTR_DONE_MASK		(0x1 << 1)
#define CTRL_IN_TRANS_MASK		(0x1 << 2)
#define STAT_ITAB_UNDER_MASK		(0x1)
#define STAT_DATA_BUFF_UNDER_MASK	(0x1 << 1)
#define STAT_ITAB_FULL_MASK		(0x1 << 2)
#define STAT_TRANSFER_DONE_MASK		(0x1 << 3)

int32_t init_io_device(void);
int32_t start_ostream(void);
int32_t stop_ostream(void);
int32_t put_to_itab(uint32_t sAddr, uint32_t sLen);
