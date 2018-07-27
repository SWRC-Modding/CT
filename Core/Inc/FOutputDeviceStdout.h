/*=============================================================================
	FOutputDeviceStdout.h: ANSI stdout output device.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
// ANSI stdout output device.
//
class FOutputDeviceStdout : public FOutputDevice
{
public:
	void Serialize( const TCHAR* V, EName Event )
	{
		printf( "%s\n", V );
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
