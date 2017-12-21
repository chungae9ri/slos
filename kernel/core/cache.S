.global flush_ent_cache

flush_ent_cache :
	mov	r0, #0
	mcr 	p15, 0, r0, c7, c5, 1
	mcr 	p15, 0, r0, c7, c6, 1
	mov	pc, lr
