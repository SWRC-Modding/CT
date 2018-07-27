/*=============================================================================
	UnLevel.h: ULevel definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Network notification sink.
-----------------------------------------------------------------------------*/

//
// Accepting connection responses.
//
enum EAcceptConnection
{
	ACCEPTC_Reject,	// Reject the connection.
	ACCEPTC_Accept, // Accept the connection.
	ACCEPTC_Ignore, // Ignore it, sending no reply, while server travelling.
};

//
// The net code uses this to send notifications.
//
class ENGINE_API FNetworkNotify
{
public:
	virtual EAcceptConnection NotifyAcceptingConnection()=0;
	virtual void NotifyAcceptedConnection( class UNetConnection* Connection )=0;
	virtual UBOOL NotifyAcceptingChannel( class UChannel* Channel )=0;
	virtual ULevel* NotifyGetLevel()=0;
	virtual void NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )=0;
	virtual UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID )=0;
	virtual void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error, UBOOL Skipped )=0;
	virtual void NotifyProgress( const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds )=0;
};

/*-----------------------------------------------------------------------------
	FCollisionHashBase.
-----------------------------------------------------------------------------*/

class FCollisionHashBase
{
public:
	// FCollisionHashBase interface.
	virtual ~FCollisionHashBase(){};
	virtual void Tick() = 0;
	virtual void AddActor(AActor* Actor) = 0;
	virtual void RemoveActor(AActor* Actor) = 0;
	virtual FCheckResult* ActorLineCheck(FMemStack& Mem, FVector End, FVector Start, FVector Extent, BYTE ExtraNodeFlags ) = 0;
	virtual FCheckResult* ActorPointCheck(FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags ) = 0;
	virtual FCheckResult* ActorRadiusCheck(FMemStack& Mem, FVector Location, FLOAT Radius, DWORD ExtraNodeFlags ) = 0;
	virtual FCheckResult* ActorEncroachmentCheck(FMemStack& Mem, AActor* Actor, FVector Location, FRotator Rotation, DWORD ExtraNodeFlags ) = 0;
	virtual void ActorOverlapCheck() = 0;	//Incomplete signature!!!
	virtual void CheckActorNotReferenced(AActor* Actor) = 0;
	virtual void CheckIsEmpty() = 0;	//Incomplete signature!!!
	virtual void CheckActorLocation() = 0; //Incomplete signature!!!
	virtual void Serialize();	//Incomplete signature!!!
};

ENGINE_API FCollisionHashBase* GNewCollisionHash();

/*-----------------------------------------------------------------------------
	ULevel base.
-----------------------------------------------------------------------------*/

//
// A game level.
//
class ENGINE_API ULevelBase : public UObject, public FNetworkNotify{
	DECLARE_ABSTRACT_CLASS(ULevelBase,UObject,0,Engine)

	// Database.
	TTransArray<AActor*> Actors;

	// Variables.
	class UNetDriver*	NetDriver;
	class UEngine*		Engine;
	FURL				URL;
	class UNetDriver*	DemoRecDriver;

	char Padding[4];

	// Constructors.
	ULevelBase(UEngine* InOwner, const FURL& InURL=FURL(NULL));

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// FNetworkNotify interface.
	void NotifyProgress(const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds);

protected:
	ULevelBase() : Actors(this){}
};

/*-----------------------------------------------------------------------------
	ULevel class.
-----------------------------------------------------------------------------*/

//
// Trace options.
//
enum ETraceFlags{
	// Bitflags.
	TRACE_Pawns				= 0x00001, // Check collision with pawns.
	TRACE_Movers			= 0x00002, // Check collision with movers.
	TRACE_Level				= 0x00004, // Check collision with BSP level geometry.
	TRACE_Volumes			= 0x00008, // Check collision with soft volume boundaries.
	TRACE_Others			= 0x00010, // Check collision with all other kinds of actors.
	TRACE_OnlyProjActor		= 0x00020, // Check collision with other actors only if they are projectile targets
	TRACE_Blocking			= 0x00040, // Check collision with other actors only if they block the check actor
	TRACE_LevelGeometry		= 0x00080, // Check collision with other actors which are static level geometry
	TRACE_ShadowCast		= 0x00100, // Check collision with shadow casting actors
	TRACE_StopAtFirstHit	= 0x00200, // Stop when find any collision (for visibility checks)
	TRACE_SingleResult		= 0x00400, // Stop when find guaranteed first nearest collision (for SingleLineCheck)
	TRACE_Debug				= 0x00800, // used for debugging specific traces
	TRACE_Material			= 0x01000, // Request that Hit.Material return the material the trace hit.
	TRACE_Projectors		= 0x02000, // Check collision with projectors
	TRACE_AcceptProjectors	= 0x04000, // Check collision with Actors with bAcceptsProjectors == true
	TRACE_Visible			= 0x08000,
	TRACE_Terrain			= 0x10000, // Check collision with terrain
	TRACE_Water				= 0x20000, // Check collision with water volumes (must have TRACE_Volumes also set)

	// Combinations.
	TRACE_Actors		= TRACE_Pawns | TRACE_Movers | TRACE_Others | TRACE_LevelGeometry | TRACE_Terrain,
	TRACE_AllColliding  = TRACE_Level | TRACE_Actors | TRACE_Volumes,
	TRACE_ProjTargets	= TRACE_OnlyProjActor | TRACE_AllColliding,
	TRACE_AllBlocking	= TRACE_Blocking | TRACE_AllColliding,
	TRACE_World = TRACE_Level | TRACE_Movers | TRACE_LevelGeometry | TRACE_Terrain,
	TRACE_Hash = TRACE_Pawns | TRACE_Movers | TRACE_Volumes | TRACE_Others | TRACE_LevelGeometry | TRACE_Terrain,
};

//
// Level updating.
//
enum ELevelTick{
	LEVELTICK_TimeOnly		= 0,	// Update the level time only.
	LEVELTICK_ViewportsOnly	= 1,	// Update time and viewports.
	LEVELTICK_All			= 2,	// Update all.
};

//
// The level object.  Contains the level's actor list, Bsp information, and brush list.
//
class ENGINE_API ULevel : public ULevelBase
{
	DECLARE_CLASS(ULevel,ULevelBase,0,Engine)
	NO_DEFAULT_CONSTRUCTOR(ULevel)

	// Number of blocks of descriptive text to allocate with levels.
	enum {NUM_LEVEL_TEXT_BLOCKS=16};

	// Main variables, always valid.
	TArray<UReachSpec>		ReachSpecs;
	UModel*					Model;
	UTextBuffer*			TextBlocks[NUM_LEVEL_TEXT_BLOCKS];
	FTime                   TimeSeconds;
	TMap<FString,FString>	TravelInfo;

	// Only valid in memory.
	FCollisionHashBase* Hash;
	class FMovingBrushTrackerBase* BrushTracker;
	AActor* FirstDeleted;
	struct FActorLink* NewlySpawned;
	UBOOL InTick, Ticked;
	INT iFirstDynamicActor, iFirstNetRelevantActor, NetTag;
	BYTE ZoneDist[64][64];

	// Temporary stats.
	INT NetTickCycles, NetDiffCycles, ActorTickCycles, AudioTickCycles, FindPathCycles, MoveCycles, NumMoves, NumReps, NumPV, GetRelevantCycles, NumRPC, SeePlayer, Spawning, Unused;

	char Padding[56];

	// Constructor.
	ULevel( UEngine* InEngine, UBOOL RootOutside );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();
	void PostLoad();

	// ULevel interface.
	virtual void Modify(bool DoTransArrays = 0);
	virtual void SetActorCollision(UBOOL bCollision, int);
	virtual void Tick(ELevelTick TickType, FLOAT DeltaSeconds);
	virtual void TickNetClient(FLOAT DeltaSeconds);
	virtual void TickNetServer(FLOAT DeltaSeconds);
	virtual INT ServerTickClients(FLOAT DeltaSeconds);
	virtual void ReconcileActors();
	virtual void RememberActors();
	virtual UBOOL Exec(const char* Cmd, FOutputDevice& Ar = *GLog);
	virtual void ShrinkLevel();
	virtual void CompactActors();
	virtual UBOOL Listen(FString& Error);
	virtual UBOOL IsServer();
	virtual UBOOL IsDedicatedServer();
	virtual UBOOL MoveActor(AActor* Actor, const FVector& Delta, const FRotator& NewRotation, FCheckResult& Hit = FCheckResult(), UBOOL Test = 0, UBOOL IgnorePawns = 0, UBOOL bIgnoreBases = 0, UBOOL bNoFail = 0, unsigned long IDONTKNOWWHATTHISIS = 0);
	virtual UBOOL FarMoveActor(AActor* Actor, const FVector& DestLocation, UBOOL Test = 0, UBOOL bNoCheck = 0, UBOOL bAttachedMove = 0);
	virtual UBOOL DestroyActor(AActor* Actor, UBOOL bNetForce = 0);
	virtual void CleanupDestroyed(UBOOL bForce);
	virtual AActor* SpawnActor(UClass* Class, FName InName = NAME_None, const FVector& Location = FVector(0,0,0), FRotator Rotation = FRotator(0,0,0), AActor* Template = NULL, UBOOL bNoCollisionFail = 0, UBOOL bRemoteOwned = 0, AActor* Owner = NULL, class APawn* Instigator = NULL);
	virtual ABrush*	SpawnBrush();
	virtual void SpawnViewActor(UViewport* Viewport);
	virtual APlayerController* SpawnPlayActor(UPlayer* Viewport, ENetRole RemoteRole, const FURL& URL, FString& Error);
	virtual UBOOL FindSpot(const FVector& Extent, class FVector& Location);
	virtual int CheckSlice(FVector&, const FVector&, int&);
	virtual UBOOL CheckEncroachment(AActor* Actor, const FVector& TestLocation, const FRotator& TestRotation, UBOOL bTouchNotify);
	virtual UBOOL SinglePointCheck(FCheckResult& Hit, const FVector& Location, const FVector& Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors);
	virtual int EncroachingWorldGeometry(struct FCheckResult&, const FVector&, const FVector&, unsigned long, class ALevelInfo*);
	virtual UBOOL SingleLineCheck(FCheckResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, DWORD TraceFlags, const FVector& Extent = FVector(0,0,0));
	virtual FCheckResult* MultiPointCheck(FMemStack& Mem, const FVector& Location, const FVector& Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors, int, int);
	virtual FCheckResult* MultiLineCheck(FMemStack& Mem, const FVector& End, const FVector& Start, const FVector& Size, ALevelInfo* LevelInfo, DWORD ExtraNodeFlags, AActor*);
	virtual void DetailChange(EDetailMode);
	virtual INT TickDemoRecord(FLOAT DeltaSeconds);
	virtual INT TickDemoPlayback(FLOAT DeltaSeconds);
	virtual void UpdateTime(ALevelInfo* Info);
	virtual UBOOL IsPaused();
	virtual void WelcomePlayer(UNetConnection* Connection, char* Optional = "");
	virtual void BuildRenderData();
	virtual UBOOL IsAudibleAt(const FVector&, const FVector&, class AActor*);
	virtual float CalculateRadiusMultiplier(int, int);

	// FNetworkNotify interface.
	EAcceptConnection NotifyAcceptingConnection();
	void NotifyAcceptedConnection( class UNetConnection* Connection );
	UBOOL NotifyAcceptingChannel( class UChannel* Channel );
	ULevel* NotifyGetLevel() {return this;}
	void NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text );
	void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error, UBOOL Skipped );
	UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID );

	ABrush* Brush();
	int EditorDestroyActor(class AActor*);
	int GetActorIndex(class AActor*);
	ALevelInfo* GetLevelInfo();
	AZoneInfo* GetZoneActor(int);
	int ToFloor(class AActor*, int, class AActor*);
	void UpdateTerrainArrays();
};

/*-----------------------------------------------------------------------------
	Iterators.
-----------------------------------------------------------------------------*/

//
// Iterate through all static brushes in a level.
//
class FStaticBrushIterator
{
public:
	// Public constructor.
	FStaticBrushIterator( ULevel *InLevel )
	:	Level   ( InLevel   )
	,	Index   ( -1        )
	{
		checkSlow(Level!=NULL);
		++*this;
	}
	void operator++()
	{
		do
		{
			if( ++Index >= Level->Actors.Num() )
			{
				Level = NULL;
				break;
			}
		} while
		(	!Level->Actors[Index]
		||	!Level->Actors[Index]->IsStaticBrush() );
	}
	ABrush* operator* ()
	{
		checkSlow(Level);
		checkSlow(Index<Level->Actors.Num());
		checkSlow(Level->Actors[Index]);
		checkSlow(Level->Actors[Index]->IsStaticBrush());
		return (ABrush*)Level->Actors[Index];
	}
	ABrush* operator-> ()
	{
		checkSlow(Level);
		checkSlow(Index<Level->Actors.Num());
		checkSlow(Level->Actors[Index]);
		checkSlow(Level->Actors[Index]->IsStaticBrush());
		return (ABrush*)Level->Actors[Index];
	}
	operator UBOOL()
	{
		return Level != NULL;
	}
protected:
	ULevel*		Level;
	INT		    Index;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
