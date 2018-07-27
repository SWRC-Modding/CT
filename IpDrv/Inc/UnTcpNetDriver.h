/*=============================================================================
	UnTcpNetDriver.h: Unreal TCP/IP driver.
	Copyright 1997-2000 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Brandon Reinhart.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UTcpipConnection.
-----------------------------------------------------------------------------*/

//
// Windows socket class.
//
class UTcpipConnection : public UNetConnection
{
	DECLARE_CLASS(UTcpipConnection,UNetConnection,CLASS_Config|CLASS_Transient,IpDrv)
	NO_DEFAULT_CONSTRUCTOR(UTcpipConnection)

	// Variables.
	sockaddr_in		RemoteAddr;
	SOCKET			Socket;
	UBOOL			OpenedLocally;
	FResolveInfo*	ResolveInfo;

	// Constructors and destructors.
	UTcpipConnection( SOCKET InSocket, UNetDriver* InDriver, sockaddr_in InRemoteAddr, EConnectionState InState, UBOOL InOpenedLocally, const FURL& InURL );

	void LowLevelSend( void* Data, INT Count );
	FString LowLevelGetRemoteAddress();
	FString LowLevelDescribe();
};

/*-----------------------------------------------------------------------------
	UTcpNetDriver.
-----------------------------------------------------------------------------*/

//
// BSD sockets network driver.
//
class UTcpNetDriver : public UNetDriver
{
	DECLARE_CLASS(UTcpNetDriver,UNetDriver,CLASS_Transient|CLASS_Config,IpDrv)

	UBOOL AllowPlayerPortUnreach;
	UBOOL LogPortUnreach;

	// Variables.
	sockaddr_in	LocalAddr;
	SOCKET		Socket;

	// Constructor.
	void StaticConstructor();
	UTcpNetDriver()
	{}

	// UNetDriver interface.
	UBOOL InitConnect( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error );
	UBOOL InitListen( FNetworkNotify* InNotify, FURL& LocalURL, FString& Error );
	void TickDispatch( FLOAT DeltaTime );
	FString LowLevelGetNetworkNumber();
	void LowLevelDestroy();

	// UTcpNetDriver interface.
	UBOOL InitBase( UBOOL Connect, FNetworkNotify* InNotify, FURL& URL, FString& Error );
	UTcpipConnection* GetServerConnection();
	FSocketData GetSocketData();
};

