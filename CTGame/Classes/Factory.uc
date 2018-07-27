// ====================================================================
//  Class:  Factory
//  Parent: Engine.Actor
//
//  Factories contain the logic for spawning new actors at spawn points
// ====================================================================

class Factory extends Actor
	abstract
	native
	placeable	
	hidecategories(Collision,Force,Karma,LightColor,Lighting,Sound);

#exec Texture Import File=..\Engine\Textures\S_PawnFactory.bmp Name=S_PawnFactory Mips=Off MASKED=1

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

enum ESpawnMode
{
	SPM_Cyclic,
	SPM_Random,
	SPM_FromFactory,
	SPM_NearPlayer
};

var() bool			bDrawLinks;
var() bool			bCovert;
var() bool			bWaitForIllegalSpawns;
var() bool			bGenerateForever;
var() bool			bStartTimerAfterDeath;
var() int			Capacity;
var() int			MaxAlive;
var() float			MaxInWorld;
var() float			SpawnInterval;
var() float			SpawnIntervalMax;
var() ESpawnMode	SpawnMode;
var() float			MaxPlayerRange;
var() float			MinPlayerRange;
var() EPhysics		InitialPhysicsFromFactory;
var() float			InitialSpeedFromFactory;
var(Events) name	ExhaustedEvent;
var(Events) name	SpawnEvent;


//Internal
var class<Actor>		InternalPrototype;
var array<Actor>		SpawnedActors;
var array<SpawnPoint>	SpawnPoints;
var int					NextSpawnPoint;
var int					RemainingItems;
var	bool				bEndWaiting;
var	bool				bWasOnceOnline;

function PostBeginPlay()
{
	RemainingItems = Capacity;
	InitActors();

	if( InternalPrototype != None )
		DynamicLoadObject(string(InternalPrototype),class'Class');
	else
		Warn("Factory "$self$" has no prototype!");	
}

function InitActors()
{	
	local SpawnPoint ASpawnPoint;

	ForEach AllActors( class'SpawnPoint', ASpawnPoint, Tag )
	{
		SpawnPoints.Insert( SpawnPoints.Length, 1 );
		SpawnPoints[SpawnPoints.Length-1] = ASpawnPoint;
	}	
}

function bool SetupNewActor(Actor NewActor, SpawnPoint SpawnPt)
{
	if( NewActor != None )
	{		
		if( SpawnMode == SPM_FromFactory )
		{
			NewActor.SetPhysics(InitialPhysicsFromFactory);
			NewActor.velocity = ( vector(Rotation) * InitialSpeedFromFactory );
		}
		else
		{
			NewActor.SetPhysics(SpawnPt.InitialPhysics);
			NewActor.velocity = ( vector(SpawnPt.Rotation) * SpawnPt.InitialSpeed );
		}
		return true;
	}
	
	return false;
}

function bool RemoveDeadActor()
{
	local int i;
	local bool bRemoved;

	while( i < SpawnedActors.length )
	{		
		if( SpawnedActors[i] == None || SpawnedActors[i].bDeleteMe || ( SpawnedActors[i].IsA('Pawn') && Pawn(SpawnedActors[i]).Health <= 0 ) )
		{
			SpawnedActors.Remove(i,1);
			bRemoved = true;
			continue;
		}
		
		i++;
	}
	return bRemoved;
}

function bool GenerateActor()
{	
	local PlayerController P;
	local int i;
	local int NearestSpawn;
	local bool bSpawnOK;	
	local float DistToPlayer, TempDist;
	local int NumInWorld;
	local Actor Other;

	RemoveDeadActor();

	if( SpawnedActors.Length < MaxAlive && ( RemainingItems > 0 || bGenerateForever ) )
	{
		if( MaxInWorld > 0 )
		{
			NumInWorld = 0;
			ForEach DynamicActors( InternalPrototype, Other )
			{
				if( !Other.IsA('Pawn') || Pawn(Other).Health > 0 )
					++NumInWorld;
			}

			if( NumInWorld >= MaxInWorld )
				return false;
		}

		SpawnedActors.Insert(0,1);
		if (SpawnMode == SPM_FromFactory)
		{
			// Get distance to player
			if (PlayerRangeTest(Location))// In Range
			{
				if( !bCovert || IsLocationCovert(Location))
				{			
					SpawnedActors[0] = spawn(InternalPrototype,,Tag,Location,Rotation);
					bSpawnOK = SetupNewActor(SpawnedActors[0], None);				
				}
			}
		}
		else if (SpawnMode == SPM_NearPlayer)
		{
			// Find nearest spawn point to player and if within range, spawn
			DistToPlayer = 0.0;
			NearestSpawn = -1;
			ForEach DynamicActors(class'PlayerController', P)
			{
				for (i = 0; i < SpawnPoints.Length; ++i)
				{
					tempDist = VDistSq(P.Pawn.location, SpawnPoints[i].location);
					if (DistToPlayer == 0.0 || DistToPlayer > tempDist)
					{
						if (!bCovert || IsLocationCovert(SpawnPoints[i].location))
						{
							DistToPlayer = tempDist;
							NearestSpawn = i;
						}
					}
				}
			}
			if (NearestSpawn >= 0 && PlayerRangeTest(SpawnPoints[NearestSpawn].Location))
			{
				SpawnedActors[0] = spawn(InternalPrototype,,Tag,SpawnPoints[NearestSpawn].Location,SpawnPoints[NearestSpawn].Rotation);
				bSpawnOK = SetupNewActor(SpawnedActors[0], SpawnPoints[NearestSpawn]);
			}
		}
		else
		{
			// If next spawn point is in range, go ahead and spawn
			if (PlayerRangeTest(SpawnPoints[NextSpawnPoint].Location) && (!bCovert || IsLocationCovert(SpawnPoints[NextSpawnPoint].location)))
			{
				SpawnedActors[0] = spawn(InternalPrototype,,Tag,SpawnPoints[NextSpawnPoint].Location,SpawnPoints[NextSpawnPoint].Rotation);
				bSpawnOK = SetupNewActor(SpawnedActors[0], SpawnPoints[NextSpawnPoint]);
			}
		}

		if( bSpawnOK && !bGenerateForever )
			RemainingItems--;

		DetermineNextSpawnPoint(bSpawnOK);

		TriggerEvent( SpawnEvent, self, None );

		if ( bStartTimerAfterDeath && SpawnedActors.Length >= MaxAlive )
			GotoState('Waiting');

		return true;
	}

	return false;
}

function DetermineNextSpawnPoint(bool bSpawnOK)
{
	local int temp;

	if( SpawnMode == SPM_Random )
	{
		while( true )
		{
			temp = rand(SpawnPoints.length);
			if( temp != NextSpawnPoint || SpawnPoints.length == 1 )
				break;				
		}			
		NextSpawnPoint = temp;
	}
	else if( SpawnMode == SPM_Cyclic)
	{
		if (bSpawnOK || !bWaitForIllegalSpawns)
		{
			NextSpawnPoint++;
			if( NextSpawnPoint >= SpawnPoints.length )
			{
				NextSpawnPoint = 0;
			}
		}
	}
}

function bool PlayerRangeTest(Vector Location)
{
	local float MaxSq, MinSq, PlayDist;
	local PlayerController P;

	if (MinPlayerRange >= MaxPlayerRange)
	{
		return true;
	}

	MaxSq = MaxPlayerRange * MaxPlayerRange;
	MinSq = MinPlayerRange * MinPlayerRange;

	ForEach DynamicActors(class'PlayerController', P)
	{
		PlayDist = VDistSq(P.Pawn.Location, Location);
		if( PlayDist >= MinSq && PlayDist <= MaxSq)
		{
			return true;
		}
	}		

	return false;
}

function bool IsLocationCovert(Vector LocationToTest)
{
	local vector HitLocation, HitNormal, PlayerEyePosition;
	local Actor HitActor;
	local PlayerController P;

	ForEach DynamicActors(class'PlayerController', P)
	{	
		PlayerEyePosition = P.Pawn.Location + P.Pawn.EyePosition();		
		if( (vector(P.Rotation) Dot Normal(LocationToTest - PlayerEyePosition)) > Cos(P.FOVAngle * 0.5 * 0.01745) ) //Convert FOVAngle from degrees to radians 
		{
			HitActor = Trace( HitLocation, HitNormal, PlayerEyePosition, LocationToTest, false );			
			if( HitActor == None || HitActor == P.Pawn )
				return false;
		}
	}		
	return true;
}

// Ask whether an event belongs to this actor; used in the editor
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(ExhaustedEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	EventString = string(SpawnEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return Super.ContainsPartialEvent(StartOfEventName);
}

function SetFactoryTimer()
{
	if (SpawnInterval < SpawnIntervalMax)
		SetTimer( SpawnInterval + (FRand() * (SpawnIntervalMax - SpawnInterval)), false);
	else
		SetTimer( SpawnInterval, true );
}

function bool IsExhausted()
{
	if (bGenerateForever)
		return false;

	//MHi - since factories start offline, offline is only meaningful if they have ever been online
	if (IsInState('Offline') && bWasOnceOnline)
		return true;

	return (RemainingItems == 0);
}

state Waiting
{
	function BeginState()
	{
		bEndWaiting = false;
		SetTimer(1.0, false);
	}

	function Timer()
	{
		if (bEndWaiting)
			GotoState('Online');
		else if (RemoveDeadActor())
		{
			bEndWaiting = true;
			SetFactoryTimer();
		}
		else
			SetTimer(1.0, false);
	}
}

state Online
{
	function Timer()
	{
		SetFactoryTimer();
		if( !GenerateActor() && RemainingItems == 0 && !bGenerateForever && SpawnedActors.Length == 0 )	
		{
			TriggerEvent( ExhaustedEvent, self, None );
			GotoState('Offline');
		}
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		GotoState('Offline');
	}

Begin:
	if( SpawnMode == SPM_Random )
		NextSpawnPoint = rand(SpawnPoints.length);

	GenerateActor();
	SetFactoryTimer();

	bWasOnceOnline = true;
}

auto state Offline
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if( SpawnMode == SPM_FromFactory || SpawnPoints.length > 0 )
			GotoState('Online');
		else
			Log( "Factory has no Spawn Points" );
	}
}



cpptext
{
	void RenderEditorSelected(FLevelSceneNode* SceneNode,FRenderInterface* RI, FDynamicActor* FDA);

}

defaultproperties
{
     bDrawLinks=True
     Capacity=2
     MaxAlive=1
     SpawnInterval=1
     SpawnMode=SPM_FromFactory
     InitialPhysicsFromFactory=PHYS_Falling
     bHidden=True
     Texture=Texture'CTGame.S_PawnFactory'
     bDirectional=True
}

