# *
# * Version: RCSL 1.0/RPSL 1.0 
# *  
# * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved. 
# *      
# * The contents of this file, and the files included with this file, are 
# * subject to the current version of the RealNetworks Public Source License 
# * Version 1.0 (the "RPSL") available at 
# * http://www.helixcommunity.org/content/rpsl unless you have licensed 
# * the file under the RealNetworks Community Source License Version 1.0 
# * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl, 
# * in which case the RCSL will apply. You may also obtain the license terms 
# * directly from RealNetworks.  You may not use this file except in 
# * compliance with the RPSL or, if you have a valid RCSL with RealNetworks 
# * applicable to this file, the RCSL.  Please see the applicable RPSL or 
# * RCSL for the rights, obligations and limitations governing use of the 
# * contents of the file.  
# *  
# * This file is part of the Helix DNA Technology. RealNetworks is the 
# * developer of the Original Code and owns the copyrights in the portions 
# * it created. 
# *  
# * This file, and the files included with this file, is distributed and made 
# * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
# * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
# * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
# * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
# * 
# * Technology Compatibility Kit Test Suite(s) Location: 
# *    http://www.helixcommunity.org/content/tck 
# * 
# * Contributor(s): 
# *  
# * ***** END LICENSE BLOCK ***** */ 

.GLOBAL xmp3_PolyphaseStereo
# * kj	AREA	|.text|, CODE, READONLY

PCM		.req	r0
VB1		.req	r1
COEF	.req 	r2

VLO		.req	r0		@ must push PCM ptr to stack during inner looop
VHI		.req	r3		@ temp variable

SUM1LL	.req 	r4
SUM1LH	.req	r5
SUM2LL	.req 	r6
SUM2LH	.req	r7
SUM1RL	.req 	r8
SUM1RH	.req	r9
SUM2RL	.req 	r10
SUM2RH	.req	r11

CF1		.req	r12
CF2		.req 	r14		

SIGN	.req  r12		@ used for clipping - after discarding CF1
MAXPOS  .req  r14		@ used for clipping - after discarding CF2

I		.req	r12		@ overlay loop counter with CF1, SIGN

		.GLOBAL	RNDVAL
#RNDVAL	SETA	(1 << ((32 - 12) + (6 - 1)))
    .set RNDVAL,(1 << ((32 - 12) + (6 - 1)))

	# C64TOS - clip 64-bit accumulator to short (no rounding)
	#   xl, xh = value (lo 32, hi 32)
	#   input assumed to have 6 fraction bits
	#   sign = temp variable to use for sign
	#   maxPos = 0x00007fff (takes 2 instr. to generate - calculating
	#              once and using repeatedly saves if you do several CTOS in a row)
	.MACRO	C64TOS	xl, xh, sign, maxPos

	mov		\xl, \xl, lsr #(20+6)
	orr		\xl, \xl, \xh, lsl #(12-6)
	mov		\sign, \xl, ASR #31
	cmp		\sign, \xl, ASR #15
	eorne	\xl, \sign, \maxPos
	
	.endm	@ C64TOS

	# MC0S - process 2 taps, 1 sample per channel (sample 0)
	#   x = vb1 offset
	.MACRO	MC0S	x

	ldr		r12, [r2], #4
	ldr		r14, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	ldr		r3, [r1, #(4*(23 - \x))]

	smlal	r4, r5, r0, r12
	ldr		r0, [r1, #(4*(32 + \x))]
	rsb		r14, r14, #0
	smlal	r4, r5, r3, r14
	ldr		r3, [r1, #(4*(32 + 23 - \x))]
	
	smlal	r8, r9, r0, r12
	smlal	r8, r9, r3, r14

	.ENDM	@ MC0S

	# MC1S - process 2 taps, 1 sample per channel (sample 16)
	#   x = vb1 offset
	.MACRO	MC1S	x

	ldr		r12, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	ldr		r3, [r1, #(4*(32 + \x))]
	smlal	r4, r5, r0, r12
	smlal	r8, r9, r3, r12
	
	.ENDM	@ MC1S

	# MC2S - process 2 taps, 2 samples per channel
	#   x = vb1 offset
	.MACRO	MC2S	x

	# load data as far as possible in advance of using it
	ldr		r12, [r2], #4
	ldr		r14, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	ldr		r3, [r1, #(4*(23 - \x))]

	smlal	r4, r5, r0, r12
	smlal	r6, r7, r0, r14
	rsb		r14, r14, #0
	smlal	r6, r7, r3, r12
	smlal	r4, r5, r3, r14

	ldr		r3, [r1, #(4*(32 + 23 - \x))]
	ldr		r0, [r1, #(4*(32 + \x))]

	smlal	r8, r9, r3, r14
	smlal	r10, r11, r3, r12
	rsb		r14, r14, #0
	smlal	r8, r9, r0, r12
	smlal	r10, r11, r0, r14

	.ENDM	@ MC2S

# void PolyphaseStereo(short *pcm, int *vbuf, const int *coefBase)

xmp3_PolyphaseStereo:
	.global	xmp3_PolyphaseStereo

	stmfd	sp!, {r4-r11, r14} @ Push to stack
	
	# clear out stack space for 2 local variables (4 bytes each)
	sub		sp, sp, #8
	str		r0, [sp, #4]				@ sp[1] = pcm pointer

	# special case, output sample 0
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r8, #RNDVAL				@ load rndVal (low 32)
	mov		r5, #0
	mov		r9, #0

	MC0S	0
	MC0S	1
	MC0S	2
	MC0S	3
	MC0S	4
	MC0S	5
	MC0S	6
	MC0S	7

	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff
	
	C64TOS	r4, r5, r12, r14
	C64TOS	r8, r9, r12, r14

	strh	r4, [r0, #(2*0)]
	strh	r8, [r0, #(2*1)]

	# special case, output sample 16
	add		r2, r2, #(4*(256-16))	@ coef = coefBase + 256 (was coefBase + 16 after MC0S block)
	add		r1, r1, #(4*1024)			@ vb1 = vbuf + 64*16
	
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r8, #RNDVAL				@ load rndVal (low 32)
	mov		r5, #0
	mov		r9, #0

	MC1S	0
	MC1S	1
	MC1S	2
	MC1S	3
	MC1S	4
	MC1S	5
	MC1S	6
	MC1S	7

	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff
	
	C64TOS	r4, r5, r12, r14
	C64TOS	r8, r9, r12, r14

	strh	r4, [r0, #(2*(2*16+0))]
	strh	r8, [r0, #(2*(2*16+1))]

	# main convolution loop: sum1L = samples 1, 2, 3, ... 15   sum2L = samples 31, 30, ... 17
	sub		r2, r2, #(4*(264-16))	@ coef = coefBase + 16 (was coefBase + 264 after MC1S block)
	sub		r1, r1, #(4*(1024-64))	@ vb1 = vbuf + 64 (was vbuf + 64*16 after MC1S block)
	mov		r12, #15						@ loop counter, count down
	add		r0, r0, #(2*2)			@ pcm+=2
	
LoopPS:
	str		r12, [sp, #0]					@ sp[0] = i (loop counter)
	str		r0, [sp, #4]				@ sp[1] = pcm (pointer to pcm buffer)
	
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r8, #RNDVAL				@ load rndVal (low 32)
	mov		r6, #RNDVAL				@ load rndVal (low 32)
	mov		r10, #RNDVAL				@ load rndVal (low 32)
	
	mov		r5, #0
	mov		r9, #0
	mov		r7, #0
	mov		r11, #0

	MC2S	0
	MC2S	1
	MC2S	2
	MC2S	3
	MC2S	4
	MC2S	5
	MC2S	6
	MC2S	7
	
	add		r1, r1, #(4*64)	@ vb1 += 64
	
	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff



	#C64TOS	$xl, $xh, $sign, $maxPos

#	mov		r4, r4, lsr #(20+6)
#	orr		r4, r4, r5, lsl #(12-6)
#	mov		r12, r4, ASR #31
#	cmp		r12, r4, ASR #15
#	eorne	r4,r12, r14

	
	C64TOS	r4, r5, r12, r14
	C64TOS	r8, r9, r12, r14
	C64TOS	r6, r7, r12, r14
	C64TOS	r10, r11, r12, r14
	
	ldr		r12, [sp, #0]			@ load loop counter
	add		r14, r0, r12, lsl #3	@ CF2 = PCM + 4*i (short offset)
	strh	r6, [r14], #2	@ *(pcm + 2*2*i + 0)
	strh	r10, [r14], #2	@ *(pcm + 2*2*i + 1)

	strh	r4, [r0], #2	@ *(pcm + 0)
	strh	r8, [r0], #2	@ *(pcm + 1)
	
	subs	r12, r12, #1
	bne		LoopPS

	# restore stack pointer
	add		sp, sp, #8

	ldmfd	sp!, {r4-r11, pc}
	MOV pc, lr 

## MONO PROCESSING

	# MC0M - process 2 taps, 1 sample (sample 0)
	#   x = vb1 offset
	.MACRO	MC0M	x

	ldr		r12, [r2], #4
	ldr		r14, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	ldr		r3, [r1, #(4*(23 - \x))]

	rsb		r14, r14, #0
	smlal	r4, r5, r0, r12
	smlal	r4, r5, r3, r14

	.endm	@ MC0M

	# MC1M - process 2 taps, 1 sample (sample 16)
	#   x = vb1 offset
	.MACRO	MC1M	x

	ldr		r12, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	smlal	r4, r5, r0, r12
	
	.endm	@ MC1M

	# MC2M - process 2 taps, 2 samples
	#   x = vb1 offset
	.MACRO	MC2M	x

	# load data as far as possible in advance of using it
	ldr		r12, [r2], #4
	ldr		r14, [r2], #4
	ldr		r0, [r1, #(4*(\x))]
	ldr		r3, [r1, #(4*(23 - \x))]

	smlal	r4, r5, r0, r12
	smlal	r6, r7, r0, r14
	rsb		r14, r14, #0
	smlal	r4, r5, r3, r14
	smlal	r6, r7, r3, r12

	.endm	@ MC2M

# void PolyphaseMono(short *pcm, int *vbuf, const int *coefBase)

xmp3_PolyphaseMono:
	.global	xmp3_PolyphaseMono

	stmfd	sp!, {r4-r11, r14}
	
	# clear out stack space for 4 local variables (4 bytes each)
	sub		sp, sp, #8
	str		r0, [sp, #4]			@ sp[1] = pcm pointer

	# special case, output sample 0
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r5, #0

	MC0M	0
	MC0M	1
	MC0M	2
	MC0M	3
	MC0M	4
	MC0M	5
	MC0M	6
	MC0M	7

	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff
	
	C64TOS	r4, r5, r12, r14
	strh	r4, [r0, #(2*0)]

	# special case, output sample 16
	add		r2, r2, #(4*(256-16))	@ coef = coefBase + 256 (was coefBase + 16 after MC0M block)
	add		r1, r1, #(4*1024)			@ vb1 = vbuf + 64*16
	
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r5, #0

	MC1M	0
	MC1M	1
	MC1M	2
	MC1M	3
	MC1M	4
	MC1M	5
	MC1M	6
	MC1M	7

	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff
	
	C64TOS	r4, r5, r12, r14

	strh	r4, [r0, #(2*16)]

	# main convolution loop: sum1L = samples 1, 2, 3, ... 15   sum2L = samples 31, 30, ... 17
	sub		r2, r2, #(4*(264-16))	@ coef = coefBase + 16 (was coefBase + 264 after MC1M block)
	sub		r1, r1, #(4*(1024-64))	@ vb1 = vbuf + 64 (was vbuf + 64*16 after MC1M block)
	mov		r12, #15						@ loop counter, count down
	add		r0, r0, #(2)				@ pcm++
	
LoopPM:
	str		r12, [sp, #0]					@ sp[0] = i (loop counter)
	str		r0, [sp, #4]				@ sp[1] = pcm (pointer to pcm buffer)
	
	mov		r4, #RNDVAL				@ load rndVal (low 32)
	mov		r6, #RNDVAL				@ load rndVal (low 32)
	mov		r5, #0
	mov		r7, #0

	MC2M	0
	MC2M	1
	MC2M	2
	MC2M	3
	MC2M	4
	MC2M	5
	MC2M	6
	MC2M	7
	
	add		r1, r1, #(4*64)	@ vb1 += 64
	
	ldr		r0, [sp, #4]		@ load pcm pointer
	mov		r14, #0x7f00
	orr		r14, r14, #0xff
	
	C64TOS	r4, r5, r12, r14
	C64TOS	r6, r7, r12, r14
	
	ldr		r12, [sp, #0]			@ load loop counter
	add		r14, r0, r12, lsl #2	@ CF2 = PCM + 2*i (short offset)
	strh	r6, [r14], #2	@ *(pcm + 2*i + 0)
	strh	r4, [r0], #2	@ *(pcm + 0) # pcm++
	
	subs	r12, r12, #1
	bne		LoopPM

	# restore stack pointer
	add		sp, sp, #8

	ldmfd	sp!, {r4-r11, pc}
	MOV pc, lr 

	.END
