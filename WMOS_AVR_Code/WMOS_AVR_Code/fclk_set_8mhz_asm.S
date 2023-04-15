/*
 ASM routine for fclk setting
 (24 MhZ main clock?)
 */

#include <avr/io.h>

.global fcpu_init


fcpu_init:
	
	// CPU_CCP = CCP_IOREG_gc;	
	ldi	r24, 0xD8	; 216
	out	0x34, r24	; 52
	
	// CLKCTRL.OSCHFCTRLA = 0x94;  // 24 MHz main clock
	ldi	r24, 0x94	; 148
	sts	0x0068, r24	; //0x800068 <__TEXT_REGION_LENGTH__+0x7e0068>
	ret   ;

