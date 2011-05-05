; /* Contributor(s): 
; *  2011.4.1.Rex   change to GNU
; * ***** END LICENSE BLOCK ***** */ 


	.text
	@.THUMB
	@.eabi_attribute Tag_ABI_align_needed, 1	@REQUIRE8
	@.eabi_attribute Tag_ABI_align_preserved, 1	@PRESERVE8
@ int xmp3_MULSHIFT32(int x, int y)
	.global xmp3_MULSHIFT32
	.global xmp3_FASTABS

@.cpu cortex-m3
@.fpu softvfp   
@.syntax unified 
@.thumb
@.text

    @ .global xmp3_MULSHIFT32
    @ .thumb_func
xmp3_MULSHIFT32:
	smull	r2, r0, r1, r0
@	mov		pc, lr	  @ KJ changed to BX for Cortex
	BX lr
	MOV pc, lr

	@.global	xmp3_FASTABS
    @.thumb_func
xmp3_FASTABS:
	mov r1, r0
	mov r0, #0x0
	eor r0, r1, r1,ASR#31
	sub r0, r0, r1,ASR#31

	bx lr
	MOV pc, lr
	
	.END
