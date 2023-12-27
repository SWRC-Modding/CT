/*=============================================================================
	IpDrvPrivate.h: Unreal TCP/IP driver.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#ifndef UNIPDRV_H
#define UNIPDRV_H

#if _MSC_VER
	#pragma warning( disable : 4201 )
#endif

// Socket API.
#if _MSC_VER
	#define __WINSOCK__ 1
	#define SOCKET_API "WinSock"
#else
	#define __BSD_SOCKETS__ 1
	#define SOCKET_API "Sockets"
#endif

// WinSock includes.
#if __WINSOCK__
#ifndef XBOX
	#include <winsock.h>
	#include <conio.h>
#endif
#endif

// BSD socket includes.
#if __BSD_SOCKETS__
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/uio.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>
	#include <errno.h>
	#include <pthread.h>
	#include <fcntl.h>

	// Handle glibc < 2.1.3
	#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0x4000
	#endif
#endif

#if SUPPORTS_PRAGMA_PACK
#pragma pack(push,4)
#endif

/*-----------------------------------------------------------------------------
	Includes..
-----------------------------------------------------------------------------*/

#include "Engine.h"
#include "UnNet.h"
#include "UnSocket.h"

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

// Globals.
extern UBOOL GIpDrvInitialized;

/*-----------------------------------------------------------------------------
	More Includes.
-----------------------------------------------------------------------------*/

#include "InternetLink.h"

/*-----------------------------------------------------------------------------
	Host resolution thread.
-----------------------------------------------------------------------------*/

#if __UNIX__
void* ResolveThreadEntry( void* Arg );
#else
DWORD STDCALL ResolveThreadEntry( void* Arg );
#endif

//
// Class for creating a background thread to resolve a host.
//
class FResolveInfo
{
public:
	// Variables.
	in_addr		Addr;
	DWORD		ThreadId;
	ANSICHAR	HostName[256];
	TCHAR		Error[256];

	#if __UNIX__
	pthread_t	ResolveThread;
	#endif

	// Functions.
	FResolveInfo( const TCHAR* InHostName )
	{
		debugf( "Resolving %s...", InHostName );

		appMemcpy( HostName, TCHAR_TO_ANSI(InHostName), appStrlen(InHostName) + 1 );
		*Error = 0;
		ThreadId = 1;

#if _MSC_VER
		HANDLE hThread = CreateThread( NULL, 0, ResolveThreadEntry, this, 0, &ThreadId );
		check(hThread);
		CloseHandle( hThread );
#else
		pthread_attr_t ThreadAttributes;
		pthread_attr_init( &ThreadAttributes );
		pthread_attr_setdetachstate( &ThreadAttributes, PTHREAD_CREATE_DETACHED );
		pthread_create( &ResolveThread, &ThreadAttributes, &ResolveThreadEntry, this );
#endif

	}
	UBOOL Resolved()
	{
#if __UNIX__
		if( ThreadId==0 )
			pthread_join(ResolveThread, NULL);
#endif
		return ThreadId==0;
	}
	const TCHAR* GetError()
	{
		return *Error ? Error : NULL;
	}
	const in_addr GetAddr()
	{
		return Addr;
	}
	const FString GetHostName()
	{
		return FString(HostName);
	}
};

/*-----------------------------------------------------------------------------
	Bind to next available port.
-----------------------------------------------------------------------------*/

//
// Bind to next available port.
//
inline int bindnextport( SOCKET s, struct sockaddr_in* addr, int portcount, int portinc )
{
	for( int i=0; i<portcount; i++ )
	{
		if( !bind( s, (sockaddr*)addr, sizeof(sockaddr_in) ) )
		{
			if (ntohs(addr->sin_port) != 0)
				return ntohs(addr->sin_port);
			else
			{
				// 0 means allocate a port for us, so find out what that port was
				struct sockaddr_in boundaddr;
				SOCKLEN size = sizeof(boundaddr);
				getsockname ( s, (sockaddr*)(&boundaddr), &size);
				return ntohs(boundaddr.sin_port);
			}
		}
		if( addr->sin_port==0 )
			break;
		addr->sin_port = htons( ntohs(addr->sin_port) + portinc );
	}
	return 0;
}

inline int getlocalhostaddr( FOutputDevice& Out, in_addr &HostAddr )
{
	int CanBindAll = 0;
	IpSetInt( HostAddr, INADDR_ANY );
#ifndef XBOX
	TCHAR Home[256]="";
	TCHAR HostName[256]="";
	ANSICHAR AnsiHostName[256]="";
	if( gethostname( AnsiHostName, 256 ) )
		Out.Logf( "%s: gethostname failed (%s)", SOCKET_API, SocketError() );
	appStrcpy( HostName, ANSI_TO_TCHAR(AnsiHostName) );
	if( Parse(appCmdLine(),"MULTIHOME=",Home,ARRAY_COUNT(Home)) )
	{
		TCHAR *A, *B, *C, *D;
		A=Home;
		if
		(	(A=Home)!=NULL
		&&	(B=appStrchr(A,'.'))!=NULL
		&&	(C=appStrchr(B+1,'.'))!=NULL
		&&	(D=appStrchr(C+1,'.'))!=NULL )
		{
			IpSetBytes( HostAddr, appAtoi(A), appAtoi(B+1), appAtoi(C+1), \
				appAtoi(D+1) );
		}
		else Out.Logf( "Invalid multihome IP address %s", Home );
	}
	else
	{
		//GHostByNameCriticalSection->Lock();
		HOSTENT* HostEnt = gethostbyname( TCHAR_TO_ANSI(HostName) );
		if( HostEnt==NULL )
		{
			Out.Logf( "gethostbyname failed (%s)", SocketError() );
		}
		else if( HostEnt->h_addrtype!=PF_INET )
		{
			Out.Logf( "gethostbyname: non-Internet address (%s)", \
				SocketError() );
		}
		else
		{
			HostAddr = *(in_addr*)( *HostEnt->h_addr_list );
			if( !ParseParam(appCmdLine(),"PRIMARYNET") )
				CanBindAll = 1;
			static UBOOL First=0;
			if( !First )
			{
				First = 1;
				debugf( NAME_Init, "%s: I am %s (%s)", SOCKET_API, HostName, *IpString( HostAddr ) );
			}
		}
		//GHostByNameCriticalSection->Unlock();
	}
#endif
	return CanBindAll;
}

//
// Get local IP to bind to
//
inline in_addr getlocalbindaddr( FOutputDevice& Out )
{
	in_addr BindAddr;

	// If we can bind to all addresses, return 0.0.0.0
	if( getlocalhostaddr( Out, BindAddr ) )
		IpSetInt( BindAddr, INADDR_ANY );
	return BindAddr;

}
/*-----------------------------------------------------------------------------
	Public includes.
-----------------------------------------------------------------------------*/

#include "IpDrvClasses.h"
#include "HTTPDownload.h"
#include "UnTcpNetDriver.h"

#pragma LINK_LIB(IpDrv)

#if SUPPORTS_PRAGMA_PACK
#pragma pack(pop)
#endif

#endif // UNIPDRV_H

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
