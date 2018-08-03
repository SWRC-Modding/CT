/*=============================================================================
	UnObjVer.h: Unreal object version.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Version coding.
-----------------------------------------------------------------------------*/

// Earliest engine build that is network compatible with this one.
CORE_API INT ENGINE_MIN_NET_VERSION;

// Engine build number, for displaying to end users.
CORE_API INT ENGINE_VERSION;

// Base protocol version to negotiate in network play.
CORE_API INT ENGINE_NEGOTIATION_VERSION;

// Prevents incorrect files from being loaded.
CORE_API INT PACKAGE_FILE_TAG;

// The current Unrealfile version.
CORE_API INT PACKAGE_FILE_VERSION;

//
// Licensee Version Number
// by Paul Du Bois, Infinite Machine
//
// Uses the upper two words of the VersionNum DWORD to encode a licensee-specific
// version WORD.
//
CORE_API INT PACKAGE_FILE_VERSION_LICENSEE;

CORE_API INT VERSION_ENGINE_ONLY;

// The earliest file version which we can load with complete
// backwards compatibility. Must be at least PACKAGE_FILE_VERSION.
CORE_API INT PACKAGE_MIN_VERSION;

CORE_API INT BUILD_VERSION;
CORE_API TCHAR* BUILD_DATE;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/