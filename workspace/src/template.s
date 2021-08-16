	.syntax		unified
	.cpu		cortex-m4
	.text


/* uint32_t Incr(uint32_t x); */
	.global		Incr
	.thumb_func
	.align
Incr:
	ADD			R0,R0,1     // x+1
	BX			LR

/* uint32_t Decr(uint32_t x); */
	.global		Decr
	.thumb_func
	.align
Decr:
	SUB			R0,R0,1     // x-1
	BX			LR

	/* End of file */
	.end
