/*=============================================================================
	UnConn.h: Unreal network connection base class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

class UNetDriver;

/*-----------------------------------------------------------------------------
	UNetConnection.
-----------------------------------------------------------------------------*/

//
// Whether to support net lag and packet loss testing.
//
#define DO_ENABLE_NET_TEST 1

//
// State of a connection.
//
enum EConnectionState
{
	USOCK_Invalid   = 0, // Connection is invalid, possibly uninitialized.
	USOCK_Closed    = 1, // Connection permanently closed.
	USOCK_Pending	= 2, // Connection is awaiting connection.
	USOCK_Open      = 3, // Connection is open.
};

#if DO_ENABLE_NET_TEST
//
// A lagged packet
//
struct DelayedPacket
{
	TArray<BYTE> Data;
	DOUBLE SendTime;
};
#endif

struct FDownloadInfo
{
	UClass* Class;
	FString ClassName;
	FString Params;
	UBOOL Compression;
};

//
// A network connection.
//
class UNetConnection : public UPlayer
{
	DECLARE_ABSTRACT_CLASS(UNetConnection,UPlayer,CLASS_Transient|CLASS_Config,Engine)

	// Constants.
	enum{ MAX_PROTOCOL_VERSION = 1     };	// Maximum protocol version supported.
	enum{ MIN_PROTOCOL_VERSION = 1     };	// Minimum protocol version supported.
	enum{ MAX_CHANNELS         = 1023  };	// Maximum channels.

	// Connection information.
	UNetDriver*			Driver;					// Owning driver.
	EConnectionState	State;					// State this connection is in.
	FURL				URL;					// URL of the other side.
	UPackageMap*		PackageMap;				// Package map between local and remote.

	// Negotiated parameters.
	INT				ProtocolVersion;		// Protocol version we're communicating with (<=PROTOCOL_VERSION).
	INT				MaxPacket;				// Maximum packet size.
	INT				PacketOverhead;			// Bytes overhead per packet sent.
	UBOOL			InternalAck;			// Internally ack all packets, for 100% reliable connections.
	INT				Challenge;				// Server-generated challenge.
	INT				NegotiatedVer;			// Negotiated version for new channels.
	INT				UserFlags;				// User-specified flags.
	FStringNoInit	RequestURL;				// URL requested by client

	// CD key authentication
    FString			CDKeyHash;				// Hash of client's CD key
	FString			CDKeyResponse;			// Client's response to CD key challenge

	// Internal.
	DOUBLE			LastReceiveTime;		// Last time a packet was received, for timeout checking.
	DOUBLE			LastSendTime;			// Last time a packet was sent, for keepalives.
	DOUBLE			LastTickTime;			// Last time of polling.
	DOUBLE			LastRepTime;			// Time of last replication.
	INT				QueuedBytes;			// Bytes assumed to be queued up.
	INT				TickCount;				// Count of ticks.

	// Merge info.
	FBitWriterMark  LastStart;				// Most recently sent bunch start.
	FBitWriterMark  LastEnd;				// Most recently sent bunch end.
	UBOOL			AllowMerge;				// Whether to allow merging.
	UBOOL			TimeSensitive;			// Whether contents are time-sensitive.
	FOutBunch*		LastOutBunch;			// Most recent outgoing bunch.
	FOutBunch		LastOut;

	// Stat display.
	DOUBLE			StatUpdateTime;			// Time of last stat update.
	FLOAT			StatPeriod;				// Interval between gathering stats.
	FLOAT			InRate,    OutRate;		// Rate for last interval.
	FLOAT			InPackets, OutPackets;	// Packet counts.
	FLOAT			InBunches, OutBunches;	// Bunch counts.
	FLOAT			InLoss,    OutLoss;		// Packet loss percent.
	FLOAT			InOrder,   OutOrder;	// Out of order incoming packets.
	FLOAT			BestLag,   AvgLag;		// Lag.

	// Stat accumulators.
	FLOAT			LagAcc, BestLagAcc;		// Previous msec lag.
	INT				InLossAcc, OutLossAcc;	// Packet loss accumulator.
	INT				InPktAcc,  OutPktAcc;	// Packet accumulator.
	INT				InBunAcc,  OutBunAcc;	// Bunch accumulator.
	INT				InByteAcc, OutByteAcc;	// Byte accumulator.
	INT				InOrdAcc,  OutOrdAcc;	// Out of order accumulator.
	INT				LagCount;				// Counter for lag measurement.
	INT				HighLossCount;			// Counts high packet loss.
	DOUBLE			LastTime, FrameTime;	// Monitors frame time.
	DOUBLE			CumulativeTime, AverageFrameTime;
	INT				CountedFrames;

	// Packet.
	FBitWriter		Out;					// Outgoing packet.
	DOUBLE			OutLagTime[256];		// For lag measuring.
	INT				OutLagPacketId[256];	// For lag measuring.
	INT				InPacketId;				// Full incoming packet index.
	INT				OutPacketId;			// Most recently sent packet.
	INT 			OutAckPacketId;			// Most recently acked outgoing packet.

	// Channel table.
	UChannel*  Channels     [ MAX_CHANNELS ];
	INT        OutReliable  [ MAX_CHANNELS ];
	INT        InReliable   [ MAX_CHANNELS ];
	INT		PendingOutRec[ MAX_CHANNELS ];	// Outgoing reliable unacked data from previous (now destroyed) channel in this slot.  This contains the first chsequence not acked
	TArray<INT> QueuedAcks, ResendAcks;
	TArray<UChannel*> OpenChannels;
	TArray<AActor*> SentTemporaries;
	TMap<AActor*,UActorChannel*> ActorChannels;

	// File Download
	UDownload*				Download;
	TArray<FDownloadInfo>	DownloadInfo;

	AActor*  Viewer;
	AActor **OwnedConsiderList;
	INT OwnedConsiderListSize;

#if DO_ENABLE_NET_TEST
	// For development.
	INT				PktLoss;
	INT				PktOrder;
	INT				PktDup;
	INT				PktLag;
	INT				PktLagVariance;
	TArray<DelayedPacket> Delayed;
#endif

	// Constructors and destructors.
	UNetConnection();
	UNetConnection( UNetDriver* Driver, const FURL& InURL );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// UPlayer interface.
	void ReadInput( FLOAT DeltaSeconds );

	// FArchive interface.
	void Serialize( const TCHAR* Data, EName MsgType );

	// FExec interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );

	// UNetConnection interface.
	virtual FString LowLevelGetRemoteAddress()=0;
	virtual FString LowLevelDescribe()=0;
	virtual void LowLevelSend( void* Data, INT Count )=0; //!! "Looks like an FArchive"
	virtual void InitOut();
	virtual void AssertValid();
	virtual void SendAck( INT PacketId, UBOOL FirstTime=1 );
	virtual void FlushNet();
	virtual void Tick();
	virtual INT IsNetReady( UBOOL Saturate );
	virtual void HandleClientPlayer( APlayerController* PC );
	void SetActorDirty( AActor* DirtyActor );

	// Functions.
	void PurgeAcks();
	void SendPackageMap();
	void PreSend( INT SizeBits );
	void PostSend();
	void ReceivedRawPacket( void* Data, INT Count );//!! "looks like an FArchive"
	INT SendRawBunch( FOutBunch& Bunch, UBOOL InAllowMerge );
	UNetDriver* GetDriver() {return Driver;}
	class UControlChannel* GetControlChannel();
	UChannel* CreateChannel( enum EChannelType Type, UBOOL bOpenedLocally, INT ChannelIndex=INDEX_NONE );
	void ReceivedPacket( FBitReader& Reader );
	void ReceivedNak( INT NakPacketId );
	void ReceiveFile( INT PackageIndex );
	void SlowAssertValid()
	{
#if DO_GUARD_SLOW
		AssertValid();
#endif
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

