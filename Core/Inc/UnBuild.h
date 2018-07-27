/*=============================================================================
	UnBuild.h: Unreal build settings.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if defined(_DEBUG) && !defined(_REALLY_WANT_DEBUG)
#error "Your active configuration is set to DEBUG.  Click here for important information!"
//
// ** IMPORTANT INFORMATION **
//
// When running in DEBUG mode, all errors intentionally trigger a breakpoint. 
// Therefore, DEBUG  builds are only suitable for running from within the debugger.  
// If you run DEBUG builds outside the debugger, you will get a GPF when an error 
// occurs instead of a useful error message.  If this is what you want, 
// define _REALLY_WANT_DEBUG and recompile.
//
// This message is here because Visual C++ sets itself to DEBUG mode when you 
// first load Unreal.dsw, which is probably not what you want.
//
// Always do a full recompile of 100% of the C++ code (all DLL's and EXE's) 
// when switching between DEBUG and RELEASE builds.  The two builds generate 
// the same named EXE's and DLL's, but they are not compatible.
// 
#endif

/*-----------------------------------------------------------------------------
	Major build options.
-----------------------------------------------------------------------------*/

// Whether to turn off all checks.
#ifndef DO_CHECK
#define DO_CHECK 1
#endif

// Whether to track call-stack errors.
#ifndef DO_GUARD
#define DO_GUARD 1
#endif

// Whether to track call-stack errors in performance critical routines.
#ifndef DO_GUARD_SLOW
#define DO_GUARD_SLOW 0
#endif

// Whether to perform CPU-intensive timing of critical loops.
#ifndef DO_CLOCK_SLOW
#define DO_CLOCK_SLOW 0
#endif

// Whether to gather performance statistics.
#ifndef STATS
#define STATS 1
#endif

// Whether to use Intel assembler code.
#ifndef ASM
#define ASM 1
#endif

// Whether to use 3DNow! assembler code.
#ifndef ASM3DNOW
#define ASM3DNOW 1
#endif

// Whether to use Katmai assembler code.
#ifndef ASMKNI
#define ASMKNI 1
#endif

// Demo version.
#ifndef DEMOVERSION
#define DEMOVERSION 0
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/