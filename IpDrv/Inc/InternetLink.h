/*=============================================================================
	InternetLink.h: TCP/UDP abstraction class
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Jack Porter
=============================================================================*/

/*-----------------------------------------------------------------------------
	FArchiveTcpSocket
-----------------------------------------------------------------------------*/

class FTcpLink;

struct FArchiveTcpSocket : public FArchive
{
	// tor.
	FArchiveTcpSocket( FTcpLink* InLink )
	:	FArchive()
	,	Link(InLink)
	{}

    void SetArVer( INT InVersion )
	{
		ArVer = InVersion;
		ArNetVer = InVersion | 0x80000000;
	}
protected:
	FTcpLink* Link;
};

/*-----------------------------------------------------------------------------
	FArchiveTcpReader - read from a TCP socket as an FArchive.
-----------------------------------------------------------------------------*/

class FTcpLink;

struct FArchiveTcpReaderPacket
{
	INT Length;
	TArray<BYTE> PacketData;
};

struct FArchiveTcpReader : public FArchiveTcpSocket
{
	// tor.
	FArchiveTcpReader( FTcpLink* InLink  );

	// FArchive interface.
	virtual void Serialize( void* V, INT Length );
	virtual UBOOL AtEnd();

private:
	friend class FTcpLink;

	// FArchiveTcpReader interface.
	virtual void ReceiveDataFromLink();
	virtual INT CompletePacketsAvailable();
	TArray<FArchiveTcpReaderPacket> Packets;

	UBOOL AtPacketEnd;
};

/*-----------------------------------------------------------------------------
	FArchiveTcpWriter - write to a TCP socket as an FArchive.
-----------------------------------------------------------------------------*/

struct FArchiveTcpWriter : public FArchiveTcpSocket
{
	// tors.
	FArchiveTcpWriter( FTcpLink* InLink  );
	virtual ~FArchiveTcpWriter();

	// FArchive interface.
	virtual void Serialize( void* V, INT Length );
	virtual void Flush();

private:
    TArray<BYTE> SendData;
};

/*----------------------------------------------------------------------------
	FArchiveUdpReader
----------------------------------------------------------------------------*/

struct FArchiveUdpReader : public FArchive
{
	// tor
	FArchiveUdpReader( BYTE* InData, INT InLength );

	// FArchive interface.
	virtual void Serialize( void* V, INT l );
	virtual UBOOL AtEnd();
private:
	BYTE* Data;
	INT Length;
};

/*----------------------------------------------------------------------------
	FArchiveUdpWriter
----------------------------------------------------------------------------*/

class FUdpLink;

struct FArchiveUdpWriter : public FArchive
{
	// tor
	FArchiveUdpWriter( FUdpLink* InLink, FIpAddr InDest );

	// FArchive interface.
	virtual void Serialize( void* V, INT Length );
	virtual void Flush();
	virtual INT Tell();

protected:
	FUdpLink* Link;
	FIpAddr Dest;
	TArray<BYTE> SendData;
};

/*-----------------------------------------------------------------------------
	FInternetLink
-----------------------------------------------------------------------------*/

class FInternetLink
{
protected:
	FSocketData SocketData;
	static UBOOL ThrottleSend;
	static UBOOL ThrottleReceive;
	static INT BandwidthSendBudget;
	static INT BandwidthReceiveBudget;

	virtual UBOOL HasBudgetToRecv() { return !ThrottleReceive || BandwidthReceiveBudget > 0; }
public:
	// tors.
	FInternetLink()
	{}
	FInternetLink(FSocketData InSocketData)
	:	SocketData(InSocketData)
	{}

	// Bandwidth throttling
	static void ThrottleBandwidth( INT SendBudget, INT ReceiveBudget );
};

/*-----------------------------------------------------------------------------
	FUdpLink
-----------------------------------------------------------------------------*/

class FUdpLink : protected FInternetLink
{
	UBOOL ExternalSocket;
protected:
	INT StatBytesSent;
	INT StatBytesReceived;
public:
	// tors.
	FUdpLink();
	FUdpLink(FSocketData InSocketData);
	virtual ~FUdpLink();

	// FUdpLink interface
	virtual UBOOL BindPort( INT Port=0 );
	virtual void Poll();
	virtual INT SendTo( FIpAddr DstAddr, BYTE* Data, INT Count );
	virtual void OnReceivedData( FIpAddr SrcAddr, BYTE* Data, INT Count )=0;
};

/*-----------------------------------------------------------------------------
	FTcpLink
-----------------------------------------------------------------------------*/

enum ETcpLinkState
{
	LINK_Closed				= 0,
	LINK_Listening			= 1,
	LINK_Connecting			= 2,
	LINK_Connected			= 3,
	LINK_ClosePending		= 4,
	LINK_ShutdownPending	= 5,
};

enum ETcpLinkMode
{
	TCPLINK_Raw			= 0,
	TCPLINK_FArchive	= 1,
};

class FTcpLink : protected FInternetLink
{
	TArray<BYTE>	ReceivedData;
	TArray<BYTE>	PendingSend;
protected:
	class FResolveInfo*	ResolveInfo;

	INT StatBytesSent;
	INT StatBytesReceived;

	DWORD LastTrafficTime;

	virtual INT InternalDataAvailable() { return ReceivedData.Num(); };
	virtual INT HasSendPending() { return LinkState == LINK_Closed ? 0 : (PendingSend.Num() && (!ThrottleSend || BandwidthSendBudget > 0)); }

	friend struct FArchiveTcpReader;
public:
	ETcpLinkState	LinkState;
	ETcpLinkMode	LinkMode;

	FArchiveTcpWriter* ArSend;
	FArchiveTcpReader* ArRecv;

	// tors.
	FTcpLink();
	FTcpLink(FSocketData InSocketData);

	virtual ~FTcpLink();

	// FTcpLink interface.
	virtual INT Send( BYTE* Data, INT Count );
	virtual INT Recv( BYTE* Data, INT Count );
	virtual void PeekData( BYTE*& Data, INT& Count );
	virtual INT DataAvailable() { return LinkMode == TCPLINK_FArchive ? ArRecv->CompletePacketsAvailable() : ReceivedData.Num(); }
	virtual void SetLinkMode( ETcpLinkMode InLinkMode );
	virtual void Listen( INT LocalPort );
	virtual void Connect( FIpAddr RemoteAddr );
	virtual UBOOL Poll( INT WaitTime );
	virtual void WaitForConnections( INT WaitTime );
	virtual void ReceivePendingData();
	virtual void SendPendingData();
	virtual void Close( UBOOL Force=0 );
	virtual void Resolve( const TCHAR* Hostname );

	// Notification
	virtual void OnResolved( FIpAddr Addr ) {}
	virtual void OnResolveFailed() {}
	virtual void OnIncomingConnection( FSocketData ConnectionData ) {}
	virtual void OnConnectionSucceeded() {}
	virtual void OnConnectionFailed() {}
	virtual void OnClosed() {}
	virtual void OnDataReceived() {}
};
