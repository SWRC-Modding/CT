//=============================================================================
// LevelInfo contains information about the current level. There should 
// be one per level and it should be actor 0. UnrealEd creates each level's 
// LevelInfo automatically so you should never have to place one
// manually.
//
// The ZoneInfo properties in the LevelInfo are used to define
// the properties of all zones which don't themselves have ZoneInfo.
//=============================================================================
class LevelInfo extends ZoneInfo
	native
	nativereplication;

// Textures.
#exec Texture Import File=Textures\WhiteSquareTexture.pcx
#exec Texture Import File=Textures\S_Vertex.tga Name=LargeVertex

//-----------------------------------------------------------------------------
// Level time.

// Time passage.
var() float TimeDilation;          // Normally 1 - scales real time passage.

// Current time.
var           float	TimeSeconds;   // Time in seconds since level began play.
var transient int   Year;          // Year.
var transient int   Month;         // Month.
var transient int   Day;           // Day of month.
var transient int   DayOfWeek;     // Day of week.
var transient int   Hour;          // Hour.
var transient int   Minute;        // Minute.
var transient int   Second;        // Second.
var transient int   Millisecond;   // Millisecond.
var			  float	PauseDelay;		// time at which to start pause


enum ELevelCampaign
{
	LC_GEO,
	LC_RAS,
	LC_YYY,
};


//-----------------------------------------------------------------------------
// Level Summary Info

var(LevelSummary) localized String 	Title;
var(LevelSummary)           String 	Author;
var(LevelSummary)			int 	RecommendedNumPlayers;
var(LevelSummary)			ELevelCampaign		Campaign;

var(Audio)	array<Name>		XACTPackagesAllowed;
var(Audio)	BOOL			bAsyncCacheFiles;

var() config enum EPhysicsDetailLevel
{
	PDL_Low,
	PDL_Medium,
	PDL_High
} PhysicsDetailLevel;


// Karma - jag
var(Karma) float		KarmaTimeScale;			// Karma physics timestep scaling.
var(Karma) float		RagdollTimeScale;		// Ragdoll physics timestep scaling. This is applied on top of KarmaTimeScale.
var globalconfig int	MaxRagdolls;			// Maximum number of simultaneous rag-dolls.
var(Karma) float		KarmaGravScale;			// Allows you to make ragdolls use lower friction than normal.
var(Karma) bool			bKStaticFriction;		// Better rag-doll/ground friction model, but more CPU.

var globalconfig int	MaxKarmaAccessories;
var	int					ActiveKarmaAccessories;

var globalconfig int	MaxDeadBodies;
var array<Pawn>			DeadBodies;

var()	   bool bKNoInit;				// Start _NO_ Karma for this level. Only really for the Entry level.
// jag

var config float	DecalStayScale;		// 0 to 2 - affects decal stay time

var() localized string LevelEnterText;  // Message to tell players when they enter.
var()           string LocalizedPkg;    // Package to look in for localizations.
var             PlayerReplicationInfo Pauser;          // If paused, name of person pausing the game.
var		LevelSummary Summary;
var           string VisibleGroups;			// List of the group names which were checked when the level was last saved
var transient string SelectedGroups;		// A list of selected groups in the group browser (only used in editor)
//-----------------------------------------------------------------------------
// Flags affecting the level.

var(LevelSummary) bool HideFromMenus;
var() bool           bLonePlayer;     // No multiplayer coordination, i.e. for entranceways.
var bool             bBegunPlay;      // Whether gameplay has begun.
var bool             bPlayersOnly;    // Only update players.
var const EDetailMode	DetailMode;      // Client detail mode.
var bool			 bDropDetail;	  // frame rate is below DesiredFrameRate, so drop high detail actors
var bool			 bAggressiveLOD;  // frame rate is well below DesiredFrameRate, so make LOD more aggressive
var bool             bStartup;        // Starting gameplay.
var config bool		 bLowSoundDetail;
var	bool			 bPathsRebuilt;	  // True if path network is valid
var bool			 bHasPathNodes;
var globalconfig bool bCapFramerate;		// frame rate capped in net play if true (else limit number of servermove updates)
var	bool			bLevelChange;
var globalconfig bool bKickLiveIdlers;	// if true, even playercontrollers with pawns can be kicked for idling
var	bool			bUseBottleneckRegions;
var	bool			bDisableBlockedRegionRecompute;
var() bool			bLevelContainsHints;

//-----------------------------------------------------------------------------
// Renderer Management.
var config bool bNeverPrecache;

//-----------------------------------------------------------------------------
// Legend - used for saving the viewport camera positions
var() vector  CameraLocationDynamic;
var() vector  CameraLocationTop;
var() vector  CameraLocationFront;
var() vector  CameraLocationSide;
var() rotator CameraRotationDynamic;

//-----------------------------------------------------------------------------
// Battle Engine properties.
var(BattleEngine) Sound	BattleMusic;
var(BattleEngine) int	BattleBaddieThreshold;
var(BattleEngine) float	BattleRadiusInner;
var(BattleEngine) float	BattleRadiusOuter;
var(BattleEngine) Actor.EBattleEngineBattleCriteria BattleCriteria;

//-----------------------------------------------------------------------------
// Miscellaneous information.

var() float Brightness;
var() texture Screenshot;
var texture DefaultTexture;
var texture WhiteSquareTexture;
var texture LargeVertex;
var int HubStackLevel;
var	bool bIsConsole;
var bool ShadowsEnabled;

var transient enum ELevelAction
{
	LEVACT_None,
	LEVACT_Loading,
	LEVACT_Saving,
	LEVACT_Connecting,
	LEVACT_Precaching
} LevelAction;

var transient GameReplicationInfo GRI;

//-----------------------------------------------------------------------------
// Networking.

var enum ENetMode
{
	NM_Standalone,        // Standalone game.
	NM_DedicatedServer,   // Dedicated server, no local client.
	NM_ListenServer,      // Listen server.
	NM_Client             // Client only, no local server.
} NetMode;
var string ComputerName;  // Machine's name according to the OS.
var string EngineVersion; // Engine version.
var string MinNetVersion; // Min engine version that is net compatible.

//-----------------------------------------------------------------------------
// Gameplay rules

var() string DefaultGameType;
var() autoload string PreCacheGame;
var GameInfo Game;
var float DefaultGravity;

//-----------------------------------------------------------------------------
// Navigation point and Pawn lists (chained using nextNavigationPoint and nextPawn).

var const NavigationPoint NavigationPointList;
var const BitArray2D RegionConnection[7];
var const Controller ControllerList;
var Actor	BactaDispenserList;
var Actor	AdditionalTargetsList;
var Actor	SquadInterestList;
var private PlayerController LocalPlayerController;		// player who is client here
var const NavigationPoint NavigationObjective;
//-----------------------------------------------------------------------------
// Custom level Anims

var(CustomAnims) array<MeshSetLinkup> CustomMeshSets;

//-----------------------------------------------------------------------------
// Server related.

var string NextURL;
var bool bNextItems;
var float NextSwitchCountdown;

//-----------------------------------------------------------------------------
// Global object recycling pool.

var transient ObjectPool	ObjectPool;

//-----------------------------------------------------------------------------
// Additional resources to precache (e.g. Playerskins).

var transient array<material>	PrecacheMaterials;
var transient array<staticmesh> PrecacheStaticMeshes;

//-----------------------------------------------------------------------------
// Replication
var float MoveRepSize;

// speed hack detection
var globalconfig float MaxTimeMargin; 
var globalconfig float TimeMarginSlack;
var globalconfig float MinTimeMargin;


// these two properties are valid only during replication
var const PlayerController ReplicationViewer;	// during replication, set to the playercontroller to
												// which actors are currently being replicated
var const Actor  ReplicationViewTarget;				// during replication, set to the viewtarget to
												// which actors are currently being replicated

// Here's some fun data for Music State
var IMuseStateInfo	MusicState;

//-----------------------------------------------------------------------------
// Functions.

native simulated function bool IsSplitScreen();
native simulated function DetailChange(EDetailMode NewDetailMode);
native simulated function bool IsEntry();
native simulated function bool IsDedicatedServer();
native simulated event bool IsSystemLink();
native event RegenerateBlockedRegions();

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	DecalStayScale = FClamp(DecalStayScale,0,2);	
	if (MusicState == None)
	{
		MusicState = new(outer) class'IMuseStateInfo';
		Log("MusicStat is "$MusicState.Name);
	}
}

simulated event FillPrecacheMaterialsArray()
{
	local Actor A;
	local class<GameInfo> G;
	
	if ( NetMode == NM_DedicatedServer )
		return;
	if ( Level.Game == None )
	{
		if ( (GRI != None) && (GRI.GameClass != "") )
			G = class<GameInfo>(DynamicLoadObject(GRI.GameClass,class'Class'));
		if ( (G == None) && (DefaultGameType != "") )
			G = class<GameInfo>(DynamicLoadObject(DefaultGameType,class'Class'));
		if ( G == None )
			G = class<GameInfo>(DynamicLoadObject(PreCacheGame,class'Class'));
		if ( G != None )
			G.Static.PreCacheGameTextures(self);
	}
	ForEach AllActors(class'Actor',A)
	{
		A.UpdatePrecacheMaterials();
	}
}

simulated event FillPrecacheStaticMeshesArray()
{
	local Actor A;
	local class<GameInfo> G;
	
	if ( NetMode == NM_DedicatedServer )
		return;
	if ( Level.Game == None )
	{
		if ( (GRI != None) && (GRI.GameClass != "") )
			G = class<GameInfo>(DynamicLoadObject(GRI.GameClass,class'Class'));
		if ( (G == None) && (DefaultGameType != "") )
			G = class<GameInfo>(DynamicLoadObject(DefaultGameType,class'Class'));
		if ( G == None )
			G = class<GameInfo>(DynamicLoadObject(PreCacheGame,class'Class'));
		if ( G != None )
			G.Static.PreCacheGameStaticMeshes(self);
	}
	
	ForEach AllActors(class'Actor',A)
		A.UpdatePrecacheStaticMeshes();
}

simulated function AddPrecacheMaterial(Material mat)
{
    local int Index;

	if ( NetMode == NM_DedicatedServer )
		return;
    if (mat == None)
        return;

    Index = Level.PrecacheMaterials.Length;
    PrecacheMaterials.Insert(Index, 1);
	PrecacheMaterials[Index] = mat;
}

simulated function AddPrecacheStaticMesh(StaticMesh stat)
{
    local int Index;

	if ( NetMode == NM_DedicatedServer )
		return;
    if (stat == None)
        return;

    Index = Level.PrecacheStaticMeshes.Length;
    PrecacheStaticMeshes.Insert(Index, 1);
	PrecacheStaticMeshes[Index] = stat;
}

function AddDeadBody( Pawn DeadBody )
{
	local int i;

	if( DeadBody == None || DeadBody.bDeleteMe )
		return;

	for( i = 0; i < DeadBodies.length; i++ )
	{
		if( DeadBodies[i] == DeadBody )
			return;
	}

	if( i == MaxDeadBodies )
	{		
		if( DeadBodies[i-1] != None )
			DeadBodies[i-1].Destroy(); // This will wind up calling RemoveDeadBody, so we don't need to call it here
		else
			DeadBodies.Remove(i-1,1);
	}

	if( DeadBodies.length < MaxDeadBodies )
	{
		DeadBodies.Insert(0,1);
		DeadBodies[0] = DeadBody;
	}
	else
	{
		Warn("Dead Body Tracking Error. Destroying "$DeadBody);
		DeadBody.Destroy();
	}
}

function RemoveDeadBody( Pawn DeadBody )
{
	local int i;
	for( i = 0; i < DeadBodies.length; i++ )
	{
		if( DeadBodies[i] == DeadBody )
		{
			DeadBodies.Remove(i,1);
			return;
		}
	}
}

//
// Return the URL of this level on the local machine.
//
native simulated function string GetLocalURL();

//
// Demo build flag
//
native simulated final function bool IsDemoBuild();  // True if this is a demo build.


//
// Return the URL of this level, which may possibly
// exist on a remote machine.
//
native simulated function string GetAddressURL();

native function string GetLevelURL(string option);

//
// Jump the server to a new level.
//
event ServerTravel( string URL, bool bItems )
{
	local PlayerController P;

	if( NextURL=="" )
	{
		//--Live
		if( (Game != None) && ((NetMode == NM_DedicatedServer) || (NetMode == NM_ListenServer) ) )
		{
			if( !Game.PreServerTravel() )
			{
				foreach DynamicActors( class'PlayerController', P )
				{
					if( NetConnection(P.Player)!=None )
						continue;

					log(">>> Network error for player" @ P.Player);
//					P.ClientTravel( "MenuLevel?Menu=XInterfaceLive.MenuLiveErrorMessage", TRAVEL_Absolute, false );
					return;
				}
				return;
			}
		}
		//Live--

		bLevelChange = true;
		bNextItems          = bItems;
		NextURL             = URL;
		if( Game!=None )
			Game.ProcessServerTravel( URL, bItems );
		else
			NextSwitchCountdown = 0;
	}
}


//
// ensure the DefaultPhysicsVolume class is loaded.
//
function ThisIsNeverExecuted()
{
	local DefaultPhysicsVolume P;
	P = None;
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	// perform garbage collection of objects (not done during gameplay)
	ConsoleCommand("OBJ GARBAGE");
	Super.Reset();
}

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	reliable if( bNetDirty && Role==ROLE_Authority )
		Pauser, TimeDilation, DefaultGravity;

	reliable if( bNetInitial && Role==ROLE_Authority )
		RagdollTimeScale, KarmaTimeScale, KarmaGravScale, GRI;
}

//
//	PreBeginPlay
//

simulated event PreBeginPlay()
{
	Super.PreBeginPlay();
	// Create the object pool.
	ObjectPool = new(none) class'ObjectPool';
}

simulated event PlayerController GetLocalPlayerController()
{
	local PlayerController PC;

	if ( Level.NetMode == NM_DedicatedServer )
		return None;
	if ( LocalPlayerController != None )
		return LocalPlayerController;

	ForEach DynamicActors(class'PlayerController', PC)
	{
		if ( Viewport(PC.Player) != None )
		{
			LocalPlayerController = PC;
			break;
		}
	}
	return LocalPlayerController;
}


defaultproperties
{
     TimeDilation=1
     Title="Untitled"
     PhysicsDetailLevel=PDL_Medium
     KarmaTimeScale=1
     RagdollTimeScale=1
     MaxRagdolls=3
     KarmaGravScale=1
     bKStaticFriction=True
     MaxKarmaAccessories=10
     MaxDeadBodies=8
     VisibleGroups="None"
     DetailMode=DM_SuperHigh
     bCapFramerate=True
     BattleBaddieThreshold=3
     BattleRadiusInner=1250
     BattleRadiusOuter=2500
     BattleCriteria=BATTLECRITERIA_AttackAndProximity
     Brightness=1
     DefaultTexture=Texture'Engine.DefaultTexture'
     WhiteSquareTexture=Texture'Engine.WhiteSquareTexture'
     LargeVertex=Texture'Engine.LargeVertex'
     PreCacheGame="ini:Engine.Engine.DefaultGame"
     DefaultGravity=-1100
     MoveRepSize=64
     MaxTimeMargin=1
     TimeMarginSlack=1.35
     MinTimeMargin=-1
     bWorldGeometry=True
     bAlwaysRelevant=True
     bBlockActors=True
     bBlockPlayers=True
     RemoteRole=ROLE_DumbProxy
     bHiddenEd=True
}

