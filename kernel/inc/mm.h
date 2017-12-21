void init_pgt(void) __attribute__((section("PGT_INIT")));
void init_mem(void); 
void *kmalloc(uint32_t size);
void kfree(uint32_t addr);
