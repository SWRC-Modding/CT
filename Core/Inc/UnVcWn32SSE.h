/*=============================================================================
	UnVcWin32SSE.h: Intel SSE support code.
	Originally created by Intel.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define NOSSE

#define XMM0	0xc0
#define XMM1	0xc1
#define XMM2	0xc2
#define XMM3	0xc3
#define XMM4	0xc4
#define XMM5	0xc5
#define XMM6	0xc6
#define XMM7	0xc7

#define xmm0	0xc0
#define xmm1	0xc1
#define xmm2	0xc2
#define xmm3	0xc3
#define xmm4	0xc4
#define xmm5	0xc5
#define xmm6	0xc6
#define xmm7	0xc7

#define _XMM0	0x00
#define _XMM1	0x01
#define _XMM2	0x02
#define _XMM3	0x03
#define _XMM4	0x04
#define _XMM5	0x05
#define _XMM6	0x06
#define _XMM7	0x07

#define _EAX 0x00
#define _ECX 0x01
#define _EDX 0x02
#define _EBX 0x03
#define _ESI 0x06
#define _EDI 0x07
#define _eax 0x00
#define _ecx 0x01
#define _edx 0x02
#define _ebx 0x03
#define _esi 0x06
#define _edi 0x07

#define REG	0xc0

#define EQ	0x0
#define LT	0x1
#define LE	0x2
#define UNORD 0x3
#define NEQ	0x4
#define NLT 0x5

#define addps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x58	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define addss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x58	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define andnps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x55	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define andps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x54	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define cmpps(dst, src, i) {	_asm _emit 0x0f	\
	_asm _emit 0xc2	\
	_asm _emit ((dst & 0x3f) << 3) | src \
	_asm _emit i}

#define cmpss(dst, src, i) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0xc2	\
	_asm _emit ((dst & 0x3f) << 3) | src \
	_asm _emit i}
//
#define comiss(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x2f	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define cvtpi2ps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x2a	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define cvtps2pi(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x2d	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define cvtsi2ss_r(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x2a	\
	_asm _emit ((dst & 0x3f) << 3) | src | REG}

#define cvtsi2ss(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x2a	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define cvtss2si(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x2d	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define cvttps2pi(dst, src) { _asm _emit 0x0f	\
	_asm _emit 0x2c	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define cvttss2si(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x2c	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define divps(dst, src) { _asm _emit 0x0f	\
	_asm _emit 0x5e	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define divss(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x5e	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define fxrstor() { _asm _emit 0x0f	\
	_asm _emit 0xae	\
	_asm _emit 0x01 }

#define fxsave() { _asm _emit 0x0f	\
	_asm _emit 0xae	\
	_asm _emit 0x00 }

#define ldmxcsr() { _asm _emit 0x0f	\
	_asm _emit 0xae	\
	_asm _emit 0x02 }
//
#define maxps(dst, src) { _asm _emit 0x0f	\
	_asm _emit 0x5f	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define maxss(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x5f	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define minps(dst, src) { _asm _emit 0x0f	\
	_asm _emit 0x5d	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define minss(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x5d	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movaps_in(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x28	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define movaps_out(src, dst) {	_asm _emit 0x0f	\
	_asm _emit 0x29	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movhps_in(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x16	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define movhps_out(src, dst) {	_asm _emit 0x0f	\
	_asm _emit 0x17	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movlps_in(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x12	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define movlps_out(src, dst) {	_asm _emit 0x0f	\
	_asm _emit 0x13	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movmskps(dst, src) { 	_asm _emit 0x0f	\
	_asm _emit 0x50	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movss_in(dst, src) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x10	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define movss_out(src, dst) { _asm _emit 0xf3	_asm _emit 0x0f	\
	_asm _emit 0x11	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//
#define movups_in(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x10	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define movups_out(src, dst) {	_asm _emit 0x0f	\
	_asm _emit 0x11	\
	_asm _emit ((dst & 0x3f) << 3) | src }
//

#define mulps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x59	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define mulss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x58	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define orps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x56	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define rcpps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x53	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define rcpss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x53	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define rsqrtps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x52	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define rsqrtss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x52	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define shufps(dst, src, i) {	_asm _emit 0x0f	\
	_asm _emit 0xc6	\
	_asm _emit ((dst & 0x3f) << 3) | src \
	_asm _emit i }

#define sqrtps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x51	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define sqrtss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x51	\
	_asm _emit ((dst & 0x3f) << 3) | src }


#define stmxcsr(dst) {	_asm _emit 0x0f	\
	_asm _emit 0xae	\
	_asm _emit 0x03 }

#define subps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x5C	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define subss(dst, src) {	_asm _emit 0xf3 _asm _emit 0x0f	\
	_asm _emit 0x5C	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define ucomiss(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x2e	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define unpckhps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x15	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define unpcklps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x14	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define xorps(dst, src) {	_asm _emit 0x0f	\
	_asm _emit 0x57	\
	_asm _emit ((dst & 0x3f) << 3) | src }

#define prefetchT0(src) {	_asm _emit 0x0f \
	_asm _emit 0x18 \
	_asm _emit src	\
	_asm _emit 0x01 }

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
