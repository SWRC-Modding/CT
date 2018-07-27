/*=============================================================================
	FOutputDeviceNull.h: Null output device.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
// ANSI stdout output device.
//
class FOutputDeviceNull : public FOutputDevice
{
public:
	void Serialize( const TCHAR* V, enum EName Event )
	{}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
