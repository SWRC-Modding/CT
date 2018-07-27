/*=============================================================================
	UnChan.h: Unreal datachannel class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UChannel base class.
-----------------------------------------------------------------------------*/

//
// Base class of communication channels.
//
class ENGINE_API UChannel : public UObject
{
	DECLARE_ABSTRACT_CLASS(UChannel,UObject,CLASS_Transient,Engine);

	// Variables.
	UNetConnection*	Connection;		// Owner connection.
	UBOOL			OpenAcked;		// Whether open has been acknowledged.
	UBOOL			Closing;		// State of the channel.
	INT             ChIndex;		// Index of this channel.
	INT				OpenedLocally;	// Whether channel was opened locally or by remote.
	INT				OpenPacketId;	// Packet the spawn message was sent in.
	UBOOL			OpenTemporary;	// Opened temporarily.
	EChannelType	ChType;			// Type of this channel.
	INT				NumInRec;		// Number of packets in InRec.
	INT				NumOutRec;		// Number of packets in OutRec.
	INT				NegotiatedVer;	// Negotiated version of engine = Min(client version, server version).
	FInBunch*		InRec;			// Incoming data with queued dependencies.
	FOutBunch*		OutRec;			// Outgoing reliable unacked data.
	UBOOL			Broken;			// Has encountered errors and is ignoring subsequent packets.

	// Statics.
	static UClass* ChannelClasses[CHTYPE_MAX];
	static UBOOL IsKnownChannelType( INT Type );

	// Constructor.
	UChannel();
	void Destroy();

	// UChannel interface.
	virtual void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );
	virtual void SetClosingFlag();
	virtual void Close();
	virtual FString Describe();
	virtual void ReceivedBunch( FInBunch& Bunch )=0;
	virtual void ReceivedNak( INT NakPacketId );
	virtual void Tick();

	// General channel functions.
	INT RouteDestroy();
	void ReceivedAcks();
	UBOOL ReceivedSequencedBunch( FInBunch& Bunch );
	void ReceivedRawBunch( FInBunch& Bunch );
	INT SendBunch( FOutBunch* Bunch, UBOOL Merge );
	INT IsNetReady( UBOOL Saturate );
	void AssertInSequenced();
	INT MaxSendBytes();
};

/*-----------------------------------------------------------------------------
	UControlChannel base class.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging text.
//
class ENGINE_API UControlChannel : public UChannel, public FOutputDevice
{
	DECLARE_CLASS(UControlChannel,UChannel,CLASS_Transient,Engine);

	// Constructor.
	void StaticConstructor()
	{
		guard(UControlChannel::StaticConstructor);
		ChannelClasses[CHTYPE_Control]        = GetClass();
		//GetDefault<UControlChannel>()->ChType = CHTYPE_Control;
		unguard;
	}
	UControlChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );
	void Destroy();

	// UChannel interface.
	void ReceivedBunch( FInBunch& Bunch );

	// FArchive interface.
	void Serialize( const TCHAR* Data, EName Event );

	// UControlChannel interface.
	FString Describe();
};

/*-----------------------------------------------------------------------------
	UActorChannel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging actor properties.
//
class ENGINE_API UActorChannel : public UChannel
{
	DECLARE_CLASS(UActorChannel,UChannel,CLASS_Transient,Engine);

	// Variables.
	ULevel*	Level;			// Level this actor channel is associated with.
	AActor* Actor;			// Actor this corresponds to.
	UClass* ActorClass;		// Class of the actor.
	FTime	RelevantTime;	// Last time this actor was relevant to client.
	FTime	LastUpdateTime;	// Last time this actor was replicated.
	UBOOL   SpawnAcked;	    // Whether spawn has been acknowledged.
	INT		NumReps;		// number of times actor has been replicated
	TArray<BYTE> Recent;	// Most recently sent values.
	TArray<BYTE> RepEval;	// Evaluated replication conditions.
	TArray<INT>  Dirty;     // Properties that are dirty and need resending.
	TArray<FPropertyRetirement> Retirement; // Property retransmission.

	// Constructor.
	void StaticConstructor()
	{
		guard(UActorChannel::StaticConstructor);
		ChannelClasses[CHTYPE_Actor]        = GetClass();
		//GetDefault<UActorChannel>()->ChType = CHTYPE_Actor;
		unguard;
	}
	UActorChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );
	void Destroy();

	// UChannel interface.
	void SetClosingFlag();
	void ReceivedBunch( FInBunch& Bunch );
	void ReceivedNak( INT NakPacketId );
	void Close();
	void Tick();

	// UActorChannel interface and accessors.
	AActor* GetActor() {return Actor;}
	FString Describe();
	void ReplicateActor();
	void SetChannelActor( AActor* InActor );
};

/*-----------------------------------------------------------------------------
	File transfer channel.
-----------------------------------------------------------------------------*/

//
// A channel for exchanging binary files.
//
class ENGINE_API UFileChannel : public UChannel
{
	DECLARE_CLASS(UFileChannel,UChannel,CLASS_Transient,Engine);

	// Receive Variables.
	UChannelDownload*	Download;		 // UDownload when receiving.

	// Send Variables.
	FArchive*			SendFileAr;		 // File being sent.
	TCHAR				SrcFilename[256];// Filename being sent.
	INT					PackageIndex;	 // Index of package in map.
	INT					SentData;		 // Number of bytes sent.

	// Constructor.
	void StaticConstructor()
	{
		guard(UFileChannel::StaticConstructor);
		ChannelClasses[CHTYPE_File]        = GetClass();
		//GetDefault<UFileChannel>()->ChType = CHTYPE_File;
		unguard;
	}
	UFileChannel();
	void Init( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );
	void Destroy();

	// UChannel interface.
	void ReceivedBunch( FInBunch& Bunch );

	// UFileChannel interface.
	FString Describe();
	void Tick();
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
