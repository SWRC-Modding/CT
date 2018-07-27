/*=============================================================================
	UnNet.h: Unreal networking.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Forward declarations.
-----------------------------------------------------------------------------*/

#ifndef _UNNET_H_
#define _UNNET_H_

class	UChannel;
class		UControlChannel;
class		UActorChannel;
class		UFileChannel;
class	FInBunch;
class	FOutBunch;
class	UChannelIterator;

class	UNetDriver;
class	UNetConnection;
class	UPendingLevel;
class		UNetPendingLevel;
class		UDemoPlayPendingLevel;

/*-----------------------------------------------------------------------------
	Types.
-----------------------------------------------------------------------------*/

// Up to this many reliable channel bunches may be buffered.
enum {RELIABLE_BUFFER         = 128   }; // Power of 2 >= 1.
#if ENGINE_VERSION<230
	enum {MAX_PACKETID        = 65536 }; // Power of 2 >= 1, covering guaranteed loss/misorder time.
	enum {MAX_CHSEQUENCE      = 65536 }; // Power of 2 >RELIABLE_BUFFER, covering loss/misorder time.
#else
	enum {MAX_PACKETID        = 16384 }; // Power of 2 >= 1, covering guaranteed loss/misorder time.
	enum {MAX_CHSEQUENCE      = 1024  }; // Power of 2 >RELIABLE_BUFFER, covering loss/misorder time.
#endif
enum {MAX_BUNCH_HEADER_BITS   = 64    };
enum {MAX_PACKET_HEADER_BITS  = 16    };
enum {MAX_PACKET_TRAILER_BITS = 1     };

// Return the value of Max/2 <= Value-Reference+some_integer*Max < Max/2.
inline INT BestSignedDifference( INT Value, INT Reference, INT Max )
{
	return ((Value-Reference+Max/2) & (Max-1)) - Max/2;
}
inline INT MakeRelative( INT Value, INT Reference, INT Max )
{
	return Reference + BestSignedDifference(Value,Reference,Max);
}

// Types of channels.
enum EChannelType
{
	CHTYPE_None			= 0,  // Invalid type.
	CHTYPE_Control		= 1,  // Connection control.
	CHTYPE_Actor  		= 2,  // Actor-update channel.
	CHTYPE_File         = 3,  // Binary file transfer.
	CHTYPE_MAX          = 8,  // Maximum.
};

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/

#include "UnNetDrv.h"		// Network driver class.
#include "UnBunch.h"		// Bunch class.
#include "UnDownload.h"		// Autodownloading classes.
#include "UnConn.h"			// Connection class.
#include "UnChan.h"			// Channel class.
#include "UnPenLev.h"		// Pending levels.
#include "UnDemoPenLev.h"	// Demo playback pending level
#include "UnDemoRec.h"		// Demo recording classes.

#endif
/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
