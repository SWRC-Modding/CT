/*=============================================================================
	UnBuild.h: Unreal build settings.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Major build options.
-----------------------------------------------------------------------------*/

// Whether to turn off all checks.
#ifndef DO_CHECK
#define DO_CHECK 1
#endif

// Whether to turn off all checks in performance critical routines.
#ifndef DO_CHECK_SLOW
#define DO_CHECK_SLOW 0
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

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
