//=============================================================================
// PlayerController
//
// PlayerControllers are used by human players to control pawns.
//
// This is a built-in Unreal class and it shouldn't be modified.
// for the change in Possess().
//=============================================================================
class PlayerController extends Controller
	config(user)
	native
    nativereplication;

// Types of Targets
enum ETargetType {
	TGT_Default,
	TGT_Friend,
	TGT_FriendInjured,
	TGT_FriendToHeal,
	TGT_Marker,
	TGT_MarkerCancel,
	TGT_Pickup,
	TGT_Enemy,
	TGT_Panel,
	TGT_DemolitionReady,
	TGT_KarmaProp,
	TGT_EngagedEnemy,
	TGT_MountTurret,
	TGT_SquadOffensiveStance,
	TGT_SquadDefensiveStance,
	TGT_SquadEngage,
	TGT_CancelAllMarkers
};

//const TimeToRevive = 2.0;
var ETargetType		TargetType;				// Type of target
var	ETargetType		HandSignalTargetType;	// Type of target at the time of giving the hand signal
var Actor			HandSignalTarget;		// Target at the time of giving the hand signal
var SquadMarker		HandSignalTargetMarker; // TargetMarker at the time of giving the hand signal
var SquadMarker		OldTargetMarker;
var SquadMarker		TargetMarker;
var float			TargetDuration;			// Time that the current target has been targeted
var float			HealingTime;
var float			TotalTimeToRevive;
var NavigationPoint OccupiedNode;

// Player info.
var const player Player;

// A simple index of that tracks the "latest" level
// the player has loaded.
var globalconfig	int		LevelProgress;

// player input control
var globalconfig	bool 	bLookUpStairs;	// look up/down stairs (player)
var globalconfig	bool	bSnapToLevel;	// Snap to level eyeheight when not mouselooking
var globalconfig	bool	bAlwaysMouseLook;
var globalconfig	bool	bKeyboardLook;	// no snapping when true
var					bool	bCenterView;
var	globalconfig	float	XTurnSlowdown;	// 0 to 1 - XBox only
var	globalconfig	float	XLookUpSlowdown;// 0 to 1 - XBox only

// customize character menu settings
var globalconfig	String	CustomizedClass;
var globalconfig	int		CloneSkin;
var globalconfig	int		TrandoSkin;
var	globalconfig	int		CloneAccessories[5];
var	globalconfig	int		TrandoAccessories[5];

//voice pack selection
var globalconfig int VoiceIndex[2];	//0 = Clone, 1 = Trando
var string CloneVoicePackNames[3];
var string TrandoVoicePackNames[3];

// Visor on/off
var globalconfig	bool	bVisor;

var globalconfig	int		VisorModeDefault;	// 0 == tactical, no normal
												// 1 == normal, no tactical
												// 2 == can cycle through both

var globalconfig	float	TacticalModeIntensity;	// 0.0 - 1.0

// Controller rumble on/off

var globalconfig	bool	bGlobalRumbleActive;
var					bool	bRumbleActive;

var	globalconfig	bool	bAutoPullManeuvers;

// Player control flags
var bool		bBehindView;    // Outside-the-player view.
var bool		bFrozen;		// set when game ends or player dies to temporarily prevent player from restarting (until cleared by timer)
var bool		bPressedJump;
var	bool		bDoubleJump;
var bool		bUpdatePosition;
var bool		bFixedCamera;	// used to fix camera in position (to view animations)
var bool		bJumpStatus;	// used in net games
var	bool		bUpdating;
var bool		bBriefing;		// are we in a briefing
var bool		bOkToSwitchWeapon;	// ok to switch weapons
var globalconfig bool	bNeverSwitchOnPickup;	// if true, don't automatically switch to picked up weapon

var bool		bZooming;		// actual act of zooming, since it's a gradual change in fov
var bool		bZoom;			// do we want to zoom in/out?
var	bool		bUse;			// use button is held down

var globalconfig bool bAlwaysLevel;
var bool		bSetTurnRot;
var bool		bCheatFlying;	// instantly stop in flying mode
var bool		bFreeCamera;	// free camera when in behindview mode (for checking out player models and animations)
var	bool		bZeroRoll;
var	bool		bCameraPositionLocked;
var	bool		bViewBot;
var bool		UseFixedVisibility;
var bool	bBlockCloseCamera;
var bool	bValidBehindCamera;
var bool	bForcePrecache;
var bool	bClientDemo;
var const bool bAllActorsRelevant;	// used by UTTV.  DO NOT SET THIS TRUE - it has a huge impact on network performance
var bool	bShortConnectTimeOut;	// when true, reduces connect timeout to 15 seconds
var bool	bPendingDestroy;		// when true, playercontroller is being destroyed

var bool	bShouldTravelInv;		// Should we travel the weapons from the previous level into this level

var globalconfig bool bNoVoiceMessages;
var globalconfig bool bNoVoiceTaunts;
var globalconfig bool bNoAutoTaunts;
var globalconfig bool bAutoTaunt;
var globalconfig bool bNoMatureLanguage;
var globalconfig bool bDynamicNetSpeed;

var globalconfig bool bKeepHintMenusAwfulHack;

var globalconfig byte AnnouncerLevel;  // 0=none, 1=no possession announcements, 2=all
var globalconfig byte AnnouncerVolume; // 1 to 4
var globalconfig byte AimingHelp;
var globalconfig byte MeleeAimingHelp;
var globalconfig float MaxResponseTime;		// how long server will wait for client move update before setting position
var float WaitDelay;			// Delay time until can restart
var pawn AcknowledgedPawn;				     // used in net games so client can acknowledge it possessed a pawn

var input float
	aBaseX, aBaseY, aBaseZ,	aMouseX, aMouseY,
	aForward, aTurn, aStrafe, aUp, aLookUp;

var input byte
	bStrafe, bSnapLevel, bLook, bFreeLook, bXAxis, bYAxis;

var EDoubleClickDir DoubleClickDir;		// direction of movement key double click (for special moves)

// Camera info.
var int ShowFlags;
var int Misc1,Misc2;
var int RendMap;
var float        OrthoZoom;     // Orthogonal/map view zoom factor.
var const actor ViewTarget;
var const Controller RealViewTarget;
var PlayerController DemoViewer;
var float CameraDist;		// multiplier for behindview camera dist
var vector OldCameraLoc;		// used in behindview calculations
var rotator OldCameraRot;
var transient array<CameraEffect> CameraEffects;	// A stack of camera effects.
var transient FrameFX FrameFX;		// Frame buffer effects class for this player
var transient Helmet Helmet;		// Helmet/HUD drawing class for this player
var DynamicLight PlayerSpotLight;

var float DesiredFOV;
var float DefaultFOV;
var float ZoomLevel;

// Fixed visibility.
var vector	FixedLocation;
var rotator	FixedRotation;
var matrix	RenderWorldToCamera;

// Screen flashes
var vector FlashScale, FlashFog;
var float ConstantGlowScale;
var vector ConstantGlowFog;

// Color Blending Vars
enum EColorBlendStage
{
	CBS_None,
	CBS_In,
	CBS_Sustain,
	CBS_Out,
};

struct ColorBlendStageInfo
{
	var Color BaseColor;
    var Color GoalColor;
	var float TotalTime;
	var float RemainingTime;
};

var Color					ColorAdd;			// Color tinting for entire frame
var EColorBlendStage		CurrentAddStage;
var ColorBlendStageInfo		AddStages[3];

var Color					ColorMultiply;		// Color tinting for entire frame
var EColorBlendStage		CurrentMultStage;
var ColorBlendStageInfo		MultStages[3];

var int						Blur;				// Color tinting for entire frame
var EColorBlendStage		CurrentBlurStage;
var ColorBlendStageInfo		BlurStages[3];

var int						Bloom;				// Color tinting for entire frame
var EColorBlendStage		CurrentBloomStage;
var ColorBlendStageInfo		BloomStages[3];

// Distance fog fading.
var color	LastDistanceFogColor;
var color	CurrentDistanceFogColor;
var float	LastDistanceFogStart;
var float	LastDistanceFogEnd;
var float	CurrentDistanceFogEnd;
var float	TimeSinceLastFogChange;
var int		LastZone;

// Remote Pawn ViewTargets
var rotator		TargetViewRotation;
var rotator     BlendedTargetViewRotation;
var float		TargetEyeHeight;
var vector		TargetWeaponViewOffset;

var HUD			myHUD;				// heads up display info
var	bool		bListObjectives;	// list all revealed objectives along w/ their status
var	MissionObjectives	Objectives;	// Level Objectives


var float LastPlaySound;
var float LastPlaySpeech;

// Move buffering for network games.  Clients save their un-acknowledged moves in order to replay them
// when they get position updates from the server.
var SavedMove SavedMoves;	// buffered moves pending position updates
var SavedMove FreeMoves;	// freed moves, available for buffering
var SavedMove PendingMove;
var float CurrentTimeStamp,LastUpdateTime,ServerTimeStamp,TimeMargin, ClientUpdateTime;
var globalconfig float MaxTimeMargin;
var Weapon OldClientWeapon;
var int WeaponUpdate;

// array of weapons indicated by their WeaponIndex: 1 if player hasn't switch to weapon yet, 0 if player has, -1 if player doesn't have weapon
// See EWeaponIndexType in Weapon.uc for list of weapon index numbers
var	travel int	FirstTimeSwitch[15];


// Progess Indicator - used by the engine to provide status messages (HUD is responsible for displaying these).
var string	ProgressMessage[4];
var color	ProgressColor[4];
var float	ProgressTimeOut;
var string	LastSave;

// Localized strings
var localized string NoPauseMessage;
var localized string ViewingFrom;
var localized string OwnCamera;

var localized string QuickSaveName;
var localized string AutoSaveName;

var localized string NoQuickOrAutoSave;

// ReplicationInfo
var GameReplicationInfo GameReplicationInfo;

// split screen multiplayer stuff
var bool bWasInvited; 	// If joined current game as a result of a game invite.
var int NetSplitID; 	// map playercontrollers to their correct controller ports (split screen)
var String Gamertag;
//--Live
var String xuid;
var bool bIsGuest;
var globalconfig int Skill;

/*
var int NumFriendRequests;
var int NumGameInvites;
// for drawing the live icons where appropriate:
var int LastNumFriendRequests;
var int LastNumGameInvites;
var float FriendRequestTimeout;
var float GameInviteTimeout;
var float NextMatchmakingQueryTime;
*/

var globalconfig float TimeBetweenMatchmakingQueries;
//Live--

// Stats Logging
var globalconfig string StatsUsername;
var globalconfig string StatsPassword;

var class<LocalMessage> LocalMessageClass;

// view shaking (affects roll, and offsets camera position)
var vector	MaxShakeOffset; // max magnitude to offset camera position
var vector	ShakeOffset; //current magnitude to offset camera from shake
var vector	ShakeRotMag;
var vector	ShakeOffsetMag;
var vector	ShakeRotLastNoise;
var vector	ShakeOffsetLastNoise;
var float	ShakeFrequency;
var float	ShakeTimeElapsed;
var float	ShakeTimeIn;
var float	ShakeTimeSustain;
var float	ShakeTimeOut;

var Pawn		TurnTarget;
var config int	EnemyTurnSpeed;
var int			GroundPitch;

var vector OldFloor;		// used by PlayerSpider mode - floor for which old rotation was based;

// Components ( inner classes )
var private transient CheatManager    CheatManager;   // Object within playercontroller that manages "cheat" commands
var class<CheatManager>		CheatClass;		// class of my CheatManager
var private transient PlayerInput	PlayerInput;	// Object within playercontroller that manages player input.
var config class<PlayerInput>       InputClass;     // class of my PlayerInput
var const vector FailedPathStart;

// Demo recording view rotation
var int DemoViewPitch;
var int DemoViewYaw;
var int	MinPlayerPitch, MaxPlayerPitch, MinPlayerYaw, MaxPlayerYaw;

var Security PlayerSecurity;	// Used for Cheat Protection
var float ForcePrecacheTime;

var float LastPingUpdate;
var float ExactPing;
var float OldPing;
var float SpectateSpeed;
var globalconfig float DynamicPingThreshold;
var float NextSpeedChange;
var int ClientCap;

var(ForceFeedback) globalconfig bool bEnablePickupForceFeedback;
var(ForceFeedback) globalconfig bool bEnableWeaponForceFeedback;
var(ForceFeedback) globalconfig bool bEnableDamageForceFeedback;
var(ForceFeedback) globalconfig bool bEnableGUIForceFeedback;
var(ForceFeedback) bool bForceFeedbackSupported;  // true if a device is detected

// For drawing player names
struct PlayerNameInfo
{
    var string mInfo;
    var color  mColor;
    var float  mXPos;
    var float  mYPos;
};

var(TeamBeacon) float      TeamBeaconMaxDist;
var(TeamBeacon) float      TeamBeaconPlayerInfoMaxDist;
var(TeamBeacon) Texture    TeamBeaconTexture;
var(TeamBeacon) Color      TeamBeaconTeamColors[2];

var private const array<PlayerNameInfo> PlayerNameArray;

// ClientAdjustPosition replication (event called at end of frame)
struct ClientAdjustment
{
	var float TimeStamp;
	var name newState;
	var EPhysics newPhysics;
	var vector NewLoc;
	var vector NewVel;
	var actor NewBase;
	var vector NewFloor;
};
var ClientAdjustment PendingAdjustment;

var float LastActiveTime;		// used to kick idlers
var float LastSpeedHackLog;
var bool bWasSpeedHack;

var transient int CachedNumSaves;

var globalconfig float	DifficultyDamageModifier;
var globalconfig float  DamageModifierEasy;
var globalconfig float  DamageModifierMedium;
var globalconfig float  DamageModifierHard;

var transient int		SavedVisionMode;

var transient bool		bBadCDKey;

var transient bool		bMissingContent;
var transient String	MissingContentName;

replication
{
	// Things the server should send to the client.
	reliable if( bNetDirty && bNetOwner && Role==ROLE_Authority )
        GameReplicationInfo;
	unreliable if ( bNetDirty && bNetOwner && Role==ROLE_Authority && (ViewTarget != Pawn) && (Pawn(ViewTarget) != None) )
		TargetViewRotation, TargetEyeHeight, TargetWeaponViewOffset;
	reliable if( bDemoRecording && Role==ROLE_Authority )
		DemoViewPitch, DemoViewYaw;

	// Functions server can call.
	reliable if( Role==ROLE_Authority )
		ClientSetHUD, FOV, StartZoom, ForceWeaponSwitch, SetOkToSwitchWeapon,
		ToggleZoom, StopZoom, EndZoom, ClientSetMusic, ClientRestart,
		ClientAdjustGlow,
		ClientSetBehindView, ClientSetFixedCamera, ClearProgressMessages,
        SetProgressMessage, SetProgressTime,
		GivePawn, ClientGotoState,
		ClientChangeVoiceChatter,
		ClientLeaveVoiceChat,
		ClientValidate,ClientSpeechMenuClose,
        ClientSetViewTarget, ClientCapBandwidth,
		ClientOpenMenu, ClientCloseMenu,
		ClientSetBadCDKey,
		ClientSetMissingContent;

	reliable if ( (Role == ROLE_Authority) && (!bDemoRecording || (bClientDemoRecording && bClientDemoNetFunc)) )
		ClientMessage, TeamMessage, ReceiveLocalizedMessage;
	unreliable if( Role==ROLE_Authority && !bDemoRecording )
        ClientPlaySoundLocally, ClientPlaySoundGlobally, PlayAnnouncement;
	reliable if( Role==ROLE_Authority && !bDemoRecording )
        ClientStopForceFeedback, ClientTravel;
	unreliable if( Role==ROLE_Authority )
        SetFOVAngle, ClientShake, ClientStopShake, ClientFlash,
		ClientAdjustPosition, ShortClientAdjustPosition, VeryShortClientAdjustPosition, LongClientAdjustPosition;
	unreliable if( (!bDemoRecording || bClientDemoRecording && bClientDemoNetFunc) && Role==ROLE_Authority )
		ClientHearSound;
    reliable if( bClientDemoRecording && ROLE==ROLE_Authority )
		DemoClientSetHUD;

	// Functions client can call.
	unreliable if( Role<ROLE_Authority )
        ServerUpdatePing, ShortServerMove, ServerMove, DualServerMove,
		RocketServerMove, Say, TeamSay, ServerSetHandedness,
		ServerViewNextPlayer, ServerViewSelf,ServerUse, ServerDrive, ServerToggleBehindView;
	reliable if( Role<ROLE_Authority )
		Speech, Pause, ServerSetPause, Mutate, ServerSetFOV, ServerAcknowledgePossession, ServerShortTimeout,
	    PrevItem, ActivateItem, ServerReStartGame, AskForPawn, ServerShowWeaponState, ServerShowPCState,
	    ChangeName, ChangeTeam, Suicide,
        ServerThrowWeapon, BehindView, Typing,
		ServerChangeVoiceChatter,
		ServerGetVoiceChatters,
		ServerChangeHasVoice,
		ServerValidationResponse, ServerVerifyViewTarget, ServerSpectateSpeed, ServerSetClientDemo,
		BroadcastSound,
		ServerUTrace, //UTrace
		ServerChooseTeam, ServerSpectate, BecomeSpectator, BecomeActivePlayer;

	reliable if (ROLE==ROLE_Authority)
    	ClientBecameSpectator, ClientBecameActivePlayer;

	unreliable if (ROLE==ROLE_Authority)
		TracerProxy;
}

native final event int GetNumFriendRequests();
native final event SetNumFriendRequests( int num );

native final event int GetNumGameInvites();
native final event SetNumGameInvites( int num );

native final event int GetLastNumFriendRequests();
native final event SetLastNumFriendRequests( int num );

native final event int GetLastNumGameInvites();
native final event SetLastNumGameInvites( int num );

native final event float GetFriendRequestTimeout();
native final event SetFriendRequestTimeout( float Timeout );

native final event float GetGameInviteTimeout();
native final event SetGameInviteTimeout( float Timeout );

native final event float GetNextMatchmakingQueryTime();
native final event SetNextMatchmakingQueryTime( float Time);

native final function SetNetSpeed(int NewSpeed);
native final function string GetPlayerNetworkAddress();
native final function string GetServerNetworkAddress();
native function string ConsoleCommand( string Command );
native final function LevelInfo GetEntryLevel();
native(544) final function ResetKeyboard();
native final function SetViewTarget(Actor NewViewTarget);
native function ClientTravel( string URL, ETravelType TravelType, bool bItems );
native(546) final function UpdateURL(string NewOption, string NewValue, bool bSaveDefault);
native final function string GetUrlOption(string Option);
native final function string GetDefaultURL(string Option);
// Execute a console command in the context of this player, then forward to Actor.ConsoleCommand.
native function CopyToClipboard( string Text );
native function string PasteFromClipboard();
native function string GetInitialMenuClass();		// Returns GameEngine.InitialMenuClass
native function string GetPauseMenuClass();			// Returns GameEngine.PauseMenuClass
native function string GetMultiplayerPauseMenuClass();	// Returns GameEngine.MultiplayerPauseMenuClass
native function string GetControllerLayoutMenuClass(); // Returns GameEngine.ControllerLayoutMenuClass
native function string GetLoadingMenuClass();		// Returns GameEngine.LoadingMenuClass
native function string GetTeamMenuClass();			// Returns GameEngine.TeamMenuClass
native function string GetProfileMenuClass();		// Returns GameEngine.ProfileMenuClass

native function GetNewLoadingHint( out String HintTitle, out String HintText, out int HintPicIndex );

native exec function UseControllerConfig( string fileName );

native function GetCurrentDateTimeString(out string Date, out string Time);
native function GetCurrentMapName(out string Name);

native function string GetLanguage();

native function bool HaveAdequateDiscSpace(bool bForSave, bool bForProfile, optional bool bForXboxLive, optional bool bTwoSaves);
native function int GetFreeDiscBlocks();
native function int GetSaveGameDiscBlocksRequired();
native function int GetProfileDiscBlocksRequired();
native function int GetXboxLiveDiscBlocksRequired();

native function float GetNormalizedGamma();
native function SetNormalizedGamma(float newGamma);

native function bool IsFullscreen();

native function float GetVoiceVolume();
native function SetVoiceVolume( float NewVolume );

native function float GetSoundVolume();
native function SetSoundVolume( float NewVolume );

native function float GetMusicVolume();
native function SetMusicVolume( float NewVolume );

native function bool GetUseLowQualitySound();
native function bool SetUseLowQualitySound( bool bValue );

native function bool Get3DSoundSupported();
native function bool GetUse3DSound();
native function bool SetUse3DSound( bool bValue );

native function bool GetUseEAX();
native function bool SetUseEAX( bool bValue );
native function int  GetMaxSupportedEAXVersion();
native function int  GetMaxCapsSupportedEAXVersion();
native function bool SetUseEAXVersion( int Version );
native function int  GetUseEAXVersion();

native function int GetNumAudioChannels();
native function bool SetNumAudioChannels( int NumChannels );

native function GetWindowedViewport(out int XSize, out int YSize);
native function SetWindowedViewport(int XSize, int YSize);

native function GetFullscreenViewport(out int XSize, out int YSize);
native function SetFullscreenViewport(int XSize, int YSize);

native function GetTextureDetail(out int Interface, out int Terrain, out int WeaponSkin, out int PlayerSkin, out int World, out int RenderMap, out int LightMap);
native function SetTextureDetail(int Interface, int Terrain, int WeaponSkin, int PlayerSkin, int World, int RenderMap, int LightMap);

native function int GetCharacterLODLevel();
native function SetCharacterLODLevel(int level);

native function bool GetShadowsEnabled();
native function EnableShadows(bool bEnable);

native function int GetBumpmappingQuality();
native function SetBumpmappingQuality(int Value);

native function bool GetBlurEnabled();
native function EnableBlur(bool bEnable);

native function int GetBloomQuality();
native function SetBloomQuality(int quality);

native function bool GetProjectorsEnabled();
native function EnableProjectors(bool bEnable);

native function bool GetVSyncEnabled();
native function EnableVSync(bool bEnable);

native function bool SupportsFSAALevel(int level);
native function int GetFSAALevel();
native function SetFSAALevel(int level);

native function bool GetConfigValue(string section, string key, out string value, optional string file );

function int GetDifficultyLevel()
{
	if ( DifficultyDamageModifier == DamageModifierEasy )
		return 0;
	else if ( DifficultyDamageModifier == DamageModifierMedium )
		return 1;
	else if ( DifficultyDamageModifier == DamageModifierHard )
		return 2;

	return -1;
}

function SetDifficultyLevel(int level)
{
	if ( level == 0 )
		DifficultyDamageModifier = DamageModifierEasy;
	else if ( level == 1 )
		DifficultyDamageModifier = DamageModifierMedium;
	else if ( level == 2 )
		DifficultyDamageModifier = DamageModifierHard;
}


simulated native event SetLevelProgress( String Level );
simulated native event bool HasReachedLevel( String Level );
simulated native event int GetLevelIndex( String Level );
simulated native function GetNextLevel( int Current, out string NextLevel );

native exec function PlayMovie( bool bLooping, bool bInterruptable, bool bPauseLevelMusic, bool bPauseLevelSound, string fileName, optional bool bDisassociatePads );
native exec function bool IsMoviePlaying();

native exec final function StartScreenFade(float FadeTime, Color FadeColor);
exec function Visor(bool bOn) { bVisor = bOn; }
native exec function NoHUDArms(bool bNoHudArms);
exec function ToggleHeadlamp();
exec function GotoVisionMode( byte NewMode );
exec native function PassOnTacticalIntensity();

native function bool IsLevelLoading();

exec function SetVisorModeDefault( int vmDefault )
{
	VisorModeDefault = vmDefault;

	if ( Level.NetMode != NM_Standalone )
		return;

	if ( Pawn == None )
		return;

	if ( ( VisorModeDefault == 0 ) && ( Pawn.CurrentUserVisionMode == 0 ) )
		GotoVisionMode( 1 );
	else if ( ( VisorModeDefault == 1 ) && ( Pawn.CurrentUserVisionMode == 1 ) )
		GotoVisionMode( 0 );
}

simulated event NoHelmetInit();

native exec function bool GetAllButtonMappings(string func, out array<int> keys);
native exec function bool ChangeKeyBinding(int key, string binding, bool bClearOtherKeysWithSameBinding);

native exec function SaveInputConfig();
native exec function SavePlayerInputConfig();
native exec function SaveClientConfig();
native exec function SaveRenderDeviceConfig();
native exec function SaveAudioConfig();

// vibration
native function bool SetRumble( bool bOnOff );
native function bool GetRumble();

native function GetPadInput( int PortNum, out array<BYTE> digitalButtons, out Vector LJoy, out Vector RJoy  );
native function bool ControllerAttached( int num );
native function DisassociateViewportPads();
native function IgnoreOtherViewportInput( bool bIgnore );
native function bool IgnoringViewportInput();
native function IgnoreViewportInput( bool bIgnore );

// Validation.
private native event X_ClientValidate(string C);
private native event X_ServerValidationResponse(string R);
event ClientValidate(string C)
{
	X_ClientValidate(C);
}

event ServerValidationResponse(string R)
{
	X_ServerValidationResponse(R);
}

native final function bool CheckSpeedHack(float DeltaTime);

/* FindStairRotation()
returns an integer to use as a pitch to orient player view along current ground (flat, up, or down)
*/
native(524) final function int FindStairRotation(float DeltaTime);

native event X_ClientHearSound(
	actor Actor,
	sound S,
	vector SoundLocation,
	int Flags
);

event ClientHearSound(
	actor Actor,
	sound S,
	vector SoundLocation,
	int Flags
)
{
	X_ClientHearSound( Actor, S, SoundLocation, Flags );
}

event PostNetBeginPlay()
{
	LastSave = "";
	Super.PostNetBeginPlay();
	SpawnDefaultHUD();
}

event PostBeginPlay()
{
	local MissionObjectives Obj;
	local int i;

	Super.PostBeginPlay();
	if (Level.LevelEnterText != "" )
		ClientMessage(Level.LevelEnterText);

	DesiredFOV = DefaultFOV;
	SetViewTarget(self);  // MUST have a view target!
	LastActiveTime = Level.TimeSeconds;

	if ( Level.NetMode == NM_Standalone )
		AddCheats();

    bForcePrecache = (Role < ROLE_Authority);
    ForcePrecacheTime = Level.TimeSeconds + 2;

	foreach AllActors(class'MissionObjectives', Obj)
		break;
	if (Obj != None)
		Objectives = Obj;

	for (i = 0; i < 10; i++)
		FirstTimeSwitch[i] = -1;

	bRumbleActive = bGlobalRumbleActive;

	if ( (DifficultyDamageModifier != DamageModifierEasy) &&
		 (DifficultyDamageModifier != DamageModifierMedium) &&
		 (DifficultyDamageModifier != DamageModifierHard) )
	{
		DifficultyDamageModifier = DamageModifierMedium;
	}

	// Refresh the number of existing saves
	if (Level.NetMode == NM_StandAlone)
		CurrentProfileNumSaves();
}

event KickWarning()
{
	ReceiveLocalizedMessage( class'GameMessage', 15 );
}

function ResetTimeMargin()
{
	TimeMargin = -0.1;
	MaxTimeMargin = Level.MaxTimeMargin;
}

function ServerShortTimeout()
{
	local Actor A;
	local float fDelay;

	bShortConnectTimeOut = true;
	ResetTimeMargin();

	fDelay = 0.5;
	if ( Level.Game.NumPlayers < 8 )
		fDelay = 0.2;

	// quick update of pickups and gameobjectives since this player is now relevant
	ForEach AllActors(class'Actor', A)
		if ( (A.NetUpdateFrequency < 1) && !A.bOnlyRelevantToOwner )
			A.NetUpdateTime = FMin(A.NetUpdateTime, Level.TimeSeconds + fDelay * FRand());
}

event PostLoadBeginPlay()
{
	Super.PostLoadBeginPlay();
	SpawnDefaultHUD();

	// Refresh the number of existing saves
	if (Level.NetMode == NM_StandAlone)
	{
		AddCheats();
		CurrentProfileNumSaves();
	}
}

exec function ToggleSpectatorMode()
{
	//Spectate/rejoin
	if(PlayerReplicationInfo.bOnlySpectator)
		BecomeActivePlayer();
	else
		BecomeSpectator();
}

exec function SetSpectateSpeed(Float F)
{
	SpectateSpeed = F;
	ServerSpectateSpeed(F);
}

function ServerSpectateSpeed(Float F)
{
	SpectateSpeed = F;
}

function ServerGivePawn()
{
	GivePawn(Pawn);
}

function ClientCapBandwidth(int Cap)
{
	ClientCap = Cap;
	if ( (Player != None) && (Player.CurrentNetSpeed > Cap) )
		SetNetSpeed(Cap);
}

function PendingStasis()
{
	bStasis = true;
	Pawn = None;
	GotoState('Scripting');
}

function AddCheats()
{
	// Assuming that this never gets called for NM_Client
	if ( CheatManager == None && (Level.NetMode == NM_Standalone) )
		CheatManager = new(Self) CheatClass;
}

function HandlePickup(Pickup pick)
{
	ReceiveLocalizedMessage(pick.MessageClass,,,,pick.class);

	if( myHUD != None )
	{
		// Track last pickup
		myHUD.LastPickup = pick;
		myHUD.LastPickupTime = Level.TimeSeconds;
	}
}

event ClientSetViewTarget( Actor a )
{
	if ( A == None )
		ServerVerifyViewTarget();
	SetViewTarget( a );
}

function ServerVerifyViewTarget()
{
	if (( ViewTarget == self ) || ( ViewTarget == None ))
		return;

	ClientSetViewTarget(ViewTarget);
}

/* SpawnDefaultHUD()
Spawn a HUD (make sure that PlayerController always has valid HUD, even if \
ClientSetHUD() hasn't been called\
*/
function SpawnDefaultHUD()
{
	myHUD = spawn(class'HUD',self);
}


simulated function TracerProxy( Vector Start, Vector End, class<Emitter> TracerEffect )
{
	local Emitter TracerEmitter;
	local Vector Dir;
	local float TraceDist;
	local int i;

	if (TracerEffect != None)
	{
		Dir = End - Start;
		TracerEmitter = Spawn(TracerEffect,,, Start,Rotator(Dir));
		TraceDist = VSize(Dir);
		for (i = 0; i < TracerEmitter.Emitters.Length; i++)
		{
			if( !VIsZero( TracerEmitter.Emitters[i].StartLocationOffset ) )
			{
				TracerEmitter.Emitters[i].StartSizeRange.Y.Min = TraceDist;
				TracerEmitter.Emitters[i].StartSizeRange.Y.Max = TraceDist;
				TracerEmitter.Emitters[i].StartLocationOffset.X = TraceDist;
			}
			else
				TracerEmitter.Emitters[i].StartLocationRange.X.Max = TraceDist;
		}
	}
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	if ( Pawn != None )
    	PawnDied(Pawn);
	Super.Reset();
	SetViewTarget(self);
	bBehindView = false;
	WaitDelay = Level.TimeSeconds + 2;
    FixFOV();
    if ( PlayerReplicationInfo.bOnlySpectator )
    	GotoState('Spectating');
    else
		GotoState('PlayerWaiting');
}

function CleanOutSavedMoves()
{
    local SavedMove Next;

	// clean out saved moves
	while ( SavedMoves != None )
	{
		Next = SavedMoves.NextMove;
		SavedMoves.Destroy();
		SavedMoves = Next;
	}
	if ( PendingMove != None )
	{
		PendingMove.Destroy();
		PendingMove = None;
	}
}

/* InitInputSystem()
Spawn the appropriate class of PlayerInput
Only called for playercontrollers that belong to local players
*/
event InitInputSystem()
{
	PlayerInput = new(Self) InputClass;
}

/* ClientGotoState()
server uses this to force client into NewState
*/
function ClientGotoState(name NewState, name NewLabel)
{
	GotoState(NewState,NewLabel);
}

function AskForPawn()
{
	if ( IsInState('GameEnded') )
		ClientGotoState('GameEnded', 'Begin');
	else if ( Pawn != None )
		GivePawn(Pawn);
	else
	{
		bFrozen = false;
		ServerRestartPlayer();
	}
}

function GivePawn(Pawn NewPawn)
{
	if ( NewPawn == None )
		return;
	Pawn = NewPawn;
	NewPawn.Controller = self;
	ClientRestart(Pawn);

	PassOnTacticalIntensity();

	if ( ( VisorModeDefault != 1 ) && ( Level.NetMode == NM_Standalone ) )
		GotoVisionMode( 1 );
}

// Possess a pawn
function Possess(Pawn aPawn)
{
    if ( PlayerReplicationInfo.bOnlySpectator )
		return;

	SetRotation(aPawn.Rotation);
	aPawn.PossessedBy(self);
	Pawn = aPawn;
	Pawn.bStasis = false;
	ResetTimeMargin();
    CleanOutSavedMoves();  // don't replay moves previous to possession
	ServerSetHandedness(Handedness);
	Restart();

	PassOnTacticalIntensity();

	if ( ( VisorModeDefault != 1 ) && ( Level.NetMode == NM_Standalone ) )
		GotoVisionMode( 1 );
}

function AcknowledgePossession(Pawn P)
{
	if ( Viewport(Player) != None )
	{
		AcknowledgedPawn = P;
		// TODO CL: Need to check this later
		//if ( P != None )
		//	P.SetBaseEyeHeight();
		ServerAcknowledgePossession(P, Handedness, bAutoTaunt);
	}
}


function ServerAcknowledgePossession(Pawn P, float NewHand, bool bNewAutoTaunt)
{
	ResetTimeMargin();
	AcknowledgedPawn = P;
	ServerSetHandedness(NewHand);
}

// unpossessed a pawn (not because pawn was killed)
function UnPossess()
{
	if ( Pawn != None )
	{
		SetLocation(Pawn.Location);
		Pawn.RemoteRole = ROLE_SimulatedProxy;
		Pawn.UnPossessed();
		CleanOutSavedMoves();  // don't replay moves previous to unpossession
		if ( Viewtarget == Pawn )
			SetViewTarget(self);
	}
	Pawn = None;
	GotoState('Spectating');
}

function ViewNextBot()
{
	if ( CheatManager != None )
		CheatManager.ViewBot();
}

// unpossessed a pawn (because pawn was killed)
function PawnDied(Pawn P)
{
	if ( P != Pawn )
		return;

	if ( Pawn != None )
		Pawn.RemoteRole = ROLE_SimulatedProxy;

	/*
	if ( ViewTarget == Pawn )
		bBehindView = true;
	*/

	ClientSpeechMenuClose();

	if(Level.NetMode > NM_Standalone || Level.IsSplitScreen())
		Super.PawnDied(P);

	if ( !IsInState('GameEnded') )
		GotoState('Dead'); // can respawn
}

simulated function ClientSpeechMenuClose()
{
	if(Player != None && Player.Console != None && ExtendedConsole(Player.Console) != None)
		ExtendedConsole(Player.Console).SpeechMenuClose();
}

simulated function PawnIncapacitated(Pawn P)
{
	if (P != Pawn)
		return;

	EndZoom();
	Super.PawnIncapacitated(P);
	if (!IsInState('GameEnded'))
		GotoState('CtrlIncapacitated');
}

simulated function PawnRevived(Pawn P)
{
	//Log("PlayerController::PawnRevived");
	Super.PawnRevived(P);
	CleanOutSavedMoves();
	EnterStartState();
}

function ClientSetHUD(class<HUD> newHUDType, class<Scoreboard> newScoringType)
{
	local HUD NewHUD;

	if ( (myHUD == None) || ((newHUDType != None) && (newHUDType != myHUD.Class)) )
	{
		NewHUD = spawn(newHUDType, self);
		if ( NewHUD != None )
		{
			if ( myHUD != None )
				myHUD.Destroy();
			myHUD = NewHUD;
		}
	}
	if ( (myHUD != None) && (newScoringType != None) )
		MyHUD.SpawnScoreBoard(newScoringType);
}

// jdf ---
// Server ignores this call, client plays effect
simulated function ClientPlayForceFeedback( String EffectName )
{
    if (bForceFeedbackSupported && Viewport(Player) != None)
        PlayFeedbackEffect( EffectName );
}

simulated function StopForceFeedback( optional String EffectName )
{
    if (bForceFeedbackSupported && Viewport(Player) != None)
    {
		if (EffectName != "")
			StopFeedbackEffect( EffectName );
		else
			StopFeedbackEffect();
	}
}

function ClientStopForceFeedback( optional String EffectName )
{
    if (bForceFeedbackSupported && Viewport(Player) != None)
    {
		if (EffectName != "")
			StopFeedbackEffect( EffectName );
		else
			StopFeedbackEffect();
	}
}
// --- jdf

final function float UpdateFlashComponent(float current, float Step, float goal)
{
	if ( goal > current )
		return FMin(current + Step, goal);
	else
		return FMax(current - Step, goal);
}

event ViewFlash(float DeltaTime)
{
    local vector goalFog;
    local float goalscale, delta, Step;

    delta = FMin(0.1, DeltaTime);
    goalScale = 1; // + ConstantGlowScale;
    goalFog = vect(0,0,0); // ConstantGlowFog;

    if ( Pawn != None )
    {
        goalScale += Pawn.HeadVolume.ViewFlash.X;
        goalFog += Pawn.HeadVolume.ViewFog;
    }
	Step = 0.6 * delta;
	FlashScale.X = UpdateFlashComponent(FlashScale.X,step,goalScale);
    FlashScale = FlashScale.X * vect(1,1,1);

	FlashFog.X = UpdateFlashComponent(FlashFog.X,step,goalFog.X);
	FlashFog.Y = UpdateFlashComponent(FlashFog.Y,step,goalFog.Y);
	FlashFog.Z = UpdateFlashComponent(FlashFog.Z,step,goalFog.Z);
}

simulated event ReceiveLocalizedMessage( class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	// Wait for player to be up to date with replication when joining a server, before stacking up messages
	if ( Level.NetMode == NM_DedicatedServer || GameReplicationInfo == None )
		return;

	Message.Static.ClientReceive( Self, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );


	if ( Message.default.bIsConsoleMessage && (Player != None) && (Player.Console != None) )
		Player.Console.Message(Message.Static.GetString(Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject),0 );

	ConsoleCommand("GAMELOG " @ Message.Static.GetString(Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject));

}

event ClientMessage( coerce string S, optional Name Type )
{
	// Wait for player to be up to date with replication when joining a server, before stacking up messages
	if ( Level.NetMode == NM_DedicatedServer || GameReplicationInfo == None )
		return;

	if (Type == '')
		Type = 'Event';

	TeamMessage(PlayerReplicationInfo, S, Type);
}

event TeamMessage( PlayerReplicationInfo PRI, coerce string S, name Type  )
{
	// Wait for player to be up to date with replication when joining a server, before stacking up messages
	if ( Level.NetMode == NM_DedicatedServer || GameReplicationInfo == None )
		return;

	if ( myHUD != None )
	myHUD.Message( PRI, S, Type );

    if(Type == 'Say' && PRI != None)
	{
		S = Class'Console'.default.cSay$">"@PRI.GetPlayerName()$": "$S;
	}
	else if(Type == 'TeamSay' && PRI != None)
	{
		S = Class'Console'.default.cTeamSay$">"@PRI.GetPlayerName()$": "$S;
	}
    Player.Console.Message( S, 6.0 );
}

simulated function PlayBeepSound();

simulated function PlayAnnouncement(sound ASound, byte AnnouncementLevel, optional bool bForce)
{
//gdr - Removed this since it was causing issues.
//	log("PlayAnnouncement - AnnouncementLevel ="@AnnouncementLevel$", AnnouncerLevel ="@AnnouncerLevel);
//	log("PlayAnnouncement - Level.TimeSeconds = "@Level.TimeSeconds$", LastPlaySound ="@LastPlaySound);
//	if ( AnnouncementLevel > AnnouncerLevel )
//		return;

	if ( !bForce && (Level.TimeSeconds - LastPlaySound < 1) )
		return;

	LastPlaySound  = Level.TimeSeconds;	// so voice messages won't overlap
	LastPlaySpeech = Level.TimeSeconds;	// don't want chatter to overlap announcements

	ASound = CustomizeAnnouncer(ASound);

	ClientPlaySoundLocally(ASound);
}

function Sound CustomizeAnnouncer(Sound AnnouncementSound)
{
	return AnnouncementSound;
}

function bool AllowVoiceMessage(name MessageType)
{
	if ( Level.NetMode == NM_Standalone )
		return true;

	if ( Level.TimeSeconds - OldMessageTime < 3 )
	{
		if ( (MessageType == 'TAUNT') || (MessageType == 'AUTOTAUNT') )
			return false;
		if ( Level.TimeSeconds - OldMessageTime < 1 )
			return false;
	}
	if ( Level.TimeSeconds - OldMessageTime < 6 )
		OldMessageTime = Level.TimeSeconds + 3;
	else
		OldMessageTime = Level.TimeSeconds;
	return true;
}

//Play a sound client side (so only client will hear it
simulated function ClientPlaySoundLocally(sound ASound)
{
	if ( ViewTarget != None )
		ViewTarget.PlaySound(ASound);
}

simulated function ClientPlaySoundGlobally(Sound theSound)
{
	//first, play the sound for the client here and now. The exception is the listen server, who will play the sound during the broadcast.
	if ( Level.NetMode != NM_ListenServer )
		PlaySound(theSound);

	//now, tell the server about it. The server will not tell ME to play the sound, since it assumes I already did that.
	//except if it's a listen server, then it assumes I did not already play it myself.
	BroadcastSound(theSound);
}

function BroadcastSound(Sound theSound)
{
	if ( Level.NetMode != NM_Standalone )
		PlayOwnedSound(theSound);
}

simulated event Destroyed()
{
	local SavedMove Next;

	// cheatmanager, adminmanager, and playerinput cleaned up in C++ PostScriptDestroyed()

    StopFeedbackEffect();

	if ( Pawn != None )
	{
		Pawn.Health = 0;
		Pawn.Unpossessed();
		Pawn.Died( self, class'Suicided', Pawn.Location );
	}

	if( PlayerSpotLight != None )
		PlayerSpotLight.Destroy();

	myHud.Destroy();

	while ( FreeMoves != None )
	{
		Next = FreeMoves.NextMove;
		FreeMoves.Destroy();
		FreeMoves = Next;
	}
	while ( SavedMoves != None )
	{
		Next = SavedMoves.NextMove;
		SavedMoves.Destroy();
		SavedMoves = Next;
	}

    if( PlayerSecurity != None )
    {
        PlayerSecurity.Destroy();
        PlayerSecurity = None;
    }

    Super.Destroyed();
}

function ClientSetMusic( Sound NewMusic )
{
	PlayMusic( NewMusic );
}

// ------------------------------------------------------------------------
// Zooming/FOV change functions

function ToggleZoom()
{
	if ( DefaultFOV != DesiredFOV )
		EndZoom();
	else
		StartZoom();
}

function StartZoom()
{
	ZoomLevel = 0.0;
	bZooming = true;
}

function StopZoom()
{
	bZooming = false;
}

function EndZoom()
{
	if (bZoom)
	{
		bZooming = false;
		DesiredFOV = DefaultFOV;
		if( Pawn != None && Pawn.Weapon != None )
		{
			Pawn.Weapon.SetWeapFOV(FOVAngle);
			Pawn.Weapon.Zoom(false);
			NoHudArms(false);
		}
	}
}

function FixFOV()
{
	FOVAngle = Default.DefaultFOV;
	DesiredFOV = Default.DefaultFOV;
	DefaultFOV = Default.DefaultFOV;
	if( Pawn != None && Pawn.Weapon != None )
	{
		Pawn.Weapon.SetWeapFOV(FOVAngle);
	}
}

function SetFOV(float NewFOV)
{
	DesiredFOV = NewFOV;
	ServerSetFOV(NewFOV);

	if( (NewFOV >= 80.0) || (Level.Netmode==NM_Standalone) )
		DesiredFOV = FClamp(NewFOV, 1, 170);
}

function ServerSetFOV(float NewFOV)
{
	if (Role == ROLE_Authority && Level.NetMode != NM_Standalone)
		FOVAngle = NewFOV;
}

function ResetFOV()
{
	DesiredFOV = DefaultFOV;
	FOVAngle = DefaultFOV;
	if( Pawn != None && Pawn.Weapon != None )
	{
		Pawn.Weapon.SetWeapFOV(FOVAngle);
		NoHUDArms(false);
	}
}

function Timer()
{
	if ( bZoom && Pawn.Weapon.bWeaponZoom )
	{
		SetFOV(Pawn.Weapon.ZoomFOVs[Pawn.Weapon.CurrentZoomFOVIndex]);
		NoHUDArms(Pawn.Weapon.bZoomedUsesNoHUDArms);
	}
	else
		NoHUDArms(false);

}

exec function FOV(float F)
{
	if (bBriefing)
		return;

	if (F <= 0.0)
	{
		F = Pawn.Weapon.GetNextZoomFOV();

		if (FOVAngle > F)
		{
			bZoom = true;
			Pawn.Weapon.Zoom(true);
			SetTimer(0.2, false);
		}
		else
		{
			bZoom = false;
			SetFOV(F);
			Pawn.Weapon.Zoom(false);
			// Weapons with no HUD arms need the timer callback
			if (Pawn.Weapon.bZoomedUsesNoHUDArms)
				SetTimer(0.1, false);
		}
	}

	if( FOVAngle > F)
	{
		// zooming in
		PlaySound( myHUD.ZoomIn );
	}
	else if (FOVAngle < F)
	{
		// zooming out
		PlaySound( myHUD.ZoomOut );
	}
	// Consider playing "no zoom" sound here if others not appropriate
}

exec function DrawHUD( bool bDraw )
{
	Helmet.bDraw = bDraw;
}

exec function Mutate(string MutateString)
{
	if( Level.NetMode == NM_Client )
		return;
	Level.Game.BaseMutator.Mutate(MutateString, Self);
}

exec function SetSensitivity(float F)
{
	PlayerInput.UpdateSensitivity(F);
}

exec function SetMouseSmoothing( int Mode )
{
    PlayerInput.UpdateSmoothing( Mode );
}

exec function SetMouseAccel(float F)
{
	PlayerInput.UpdateAccel(F);
}

exec function bool GetInvertLook()
{
	return PlayerInput.bInvertLook;
}

exec function SetInvertLook(bool bInvert)
{
	PlayerInput.bInvertLook = bInvert;
}

exec function float GetMouseSensitivity()
{
	return PlayerInput.MouseSensitivity;
}

exec function SetMouseSensitivity(float F)
{
	PlayerInput.UpdateSensitivity(F);
}

exec function GetJoySensitivity(out float XY, out float UV)
{
	XY = PlayerInput.JoySensitivityXY;
	UV = PlayerInput.JoySensitivityUV;
}

exec function SetJoySensitivity(float XY, float UV)
{
	PlayerInput.JoySensitivityXY = XY;
	PlayerInput.JoySensitivityUV = UV;
}

native exec function PropagateSettings();
native exec function PropagateBadCDKey();
native exec function PropagateMissingContent();

exec function ForceReload()
{
	if ( (Pawn != None) && (Pawn.Weapon != None) )
	{
		Pawn.Weapon.ForceReload();
	}
}


// added by Demiurge Studios (MemStat)
exec function TraceForMemStats()
{
	local actor HitActor;
	local vector HitLocation, HitNormal;

	HitActor = Trace(HitLocation, HitNormal, pawn.location + (vect(0,0,1) * pawn.baseEyeHeight) + 10000.0 * vector(pawn.getViewRotation()), pawn.location + (vect(0,0,1) * pawn.baseEyeHeight), true);
	//log("hit Actor = "$HitActor);

	if(HitActor != NONE)
	{
		//log("class="$HitActor.Class$" name="$HitActor.name);
		consoleCommand("stat setAsset class="$HitActor.Class$" name="$HitActor.name);
	}
}
// end Demiurge

// ------------------------------------------------------------------------
// Messaging functions

// Send a message to all players.
exec function Say( string Msg )
{
	if (PlayerReplicationInfo.bAdmin && left(Msg,1) == "#" )
	{
		Level.Game.AdminSay(right(Msg,len(Msg)-1));
		return;
	}
	Level.Game.Broadcast(self, Msg, 'Say');
}

exec function TeamSay( string Msg )
{
	if( !GameReplicationInfo.bTeamGame )
	{
		Say( Msg );
		return;
	}

    Level.Game.BroadcastTeam( self, Level.Game.ParseMessageString( Level.Game.BaseMutator , self, Msg ) , 'TeamSay');
}

exec function ShowGameStats()
{
	if( Level.NetMode == NM_Standalone && !Level.IsSplitScreen() )
	{
		//ShowObjectives();
		ToggleCurrentObjective();
	}
	else
		myHUD.ShowScores();
}

// Mission Objectives------------------------------------------------------
exec function ShowObjectives()
{
	bListObjectives = !bListObjectives;
}
// ------------------------------------------------------------------------

exec function ToggleCurrentObjective()
{
	myHUD.bShowCurrentObjective = !(myHUD.bShowCurrentObjective);
}

function ServerSetHandedness( float hand)
{
	Handedness = hand;
    if ( (Pawn != None) && (Pawn.Weapon != None) )
		Pawn.Weapon.SetHand(Handedness);
}

function SetHand(int IntValue)
{
    class'PlayerController'.default.Handedness = IntValue;
    class'PlayerController'.static.StaticSaveConfig();
    Handedness = IntValue;
    if( (Pawn != None) && (Pawn.Weapon != None) )
	Pawn.Weapon.SetHand(Handedness);

	ServerSetHandedness(Handedness);
}

exec function SetWeaponHand ( string S )
{
	if ( S ~= "Left" )
		Handedness = -1;
	else if ( S~= "Right" )
		Handedness = 1;
	else if ( S ~= "Center" )
		Handedness = 0;
	else if ( S ~= "Hidden" )
		Handedness = 2;
    SetHand(Handedness);
}

function bool IsDead()
{
	return false;
}

exec function ShowGun ()
{
    if( Handedness == 2 )
        Handedness = 1;
    else
        Handedness = 2;

    SetHand(Handedness);
}

event PreClientTravel()
{
    ClientStopForceFeedback();  // jdf
}

function ClientSetFixedCamera(bool B)
{
	bFixedCamera = B;
}

function ClientSetBehindView(bool B)
{
	bBehindView = B;
}

/* ForceDeathUpdate()
Make sure ClientAdjustPosition immediately informs client of pawn's death
*/
function ForceDeathUpdate()
{
	LastUpdateTime = Level.TimeSeconds - 10;
}

/* RocketServerMove()
compressed version of server move for PlayerRocketing state
*/
function RocketServerMove
(
	float TimeStamp,
	vector InAccel,
	vector ClientLoc,
	byte ClientRoll,
	int View
)
{
	ServerMove(TimeStamp,InAccel,ClientLoc,false,false,false,false, DCLICK_NONE,ClientRoll,View);
}

/* ShortServerMove()
compressed version of server move for bandwidth saving
*/
function ShortServerMove
(
	float TimeStamp,
	vector ClientLoc,
	bool NewbRun,
	bool NewbDuck,
	bool NewbJumpStatus,
	byte ClientRoll,
	int View
)
{
    ServerMove(TimeStamp,vect(0,0,0),ClientLoc,NewbRun,NewbDuck,NewbJumpStatus,false,DCLICK_None,ClientRoll,View);
}

/* DualServerMove()
- replicated function sent by client to server - contains client movement and firing info for two moves
*/
function DualServerMove
(
 float TimeStamp0,
 vector InAccel0,
 byte PendingCompress,
 eDoubleClickDir DoubleClickMove0,
 int View0,
 float TimeStamp,
 vector InAccel,
 vector ClientLoc,
 eDoubleClickDir DoubleClickMove,
 byte ClientRoll,
 int View,
 optional byte OldTimeDelta,
 optional int OldAccel
 )
{
	local bool NewbRun0,NewbDuck0,NewbJumpStatus0,NewbDoubleJump0,
		NewbRun,NewbDuck,NewbJumpStatus,NewbDoubleJump;

	NewbRun0 = (PendingCompress & 1) != 0;
	NewbDuck0 = (PendingCompress & 2) != 0;
	NewbJumpStatus0 = (PendingCompress & 4) != 0;
	NewbDoubleJump0 = (PendingCompress & 8) != 0;
	NewbRun = (PendingCompress & 16) != 0;
	NewbDuck = (PendingCompress & 32) != 0;
	NewbJumpStatus = (PendingCompress & 64) != 0;
	NewbDoubleJump = (PendingCompress & 128) != 0;

	ServerMove(TimeStamp0,InAccel0,vect(0,0,0),NewbRun0,NewbDuck0,NewbJumpStatus0,NewbDoubleJump0,DoubleClickMove0,
		ClientRoll,View0);
	if ( ClientLoc == vect(0,0,0) )
		ClientLoc = vect(0.1,0,0);
	ServerMove(TimeStamp,InAccel,ClientLoc,NewbRun,NewbDuck,NewbJumpStatus,NewbDoubleJump,DoubleClickMove,ClientRoll,View,OldTimeDelta,OldAccel);
}

/* ServerMove()
- replicated function sent by client to server - contains client movement and firing info.
*/
function ServerMove
(
 float TimeStamp,
 vector InAccel,
 vector ClientLoc,
 bool NewbRun,
 bool NewbDuck,
 bool NewbJumpStatus,
 bool NewbDoubleJump,
 eDoubleClickDir DoubleClickMove,
 byte ClientRoll,
 int View,
 optional byte OldTimeDelta,
 optional int OldAccel
 )
{
	local float DeltaTime, clientErr, OldTimeStamp;
	local rotator DeltaRot, Rot, ViewRot;
	local vector Accel, LocDiff;
	local int maxPitch, ViewPitch, ViewYaw;
	local bool NewbPressedJump, OldbRun, OldbDoubleJump;
	local eDoubleClickDir OldDoubleClickMove;

	// If this move is outdated, discard it.
	if ( CurrentTimeStamp >= TimeStamp )
		return;

	if ( AcknowledgedPawn != Pawn )
	{
		OldTimeDelta = 0;
		InAccel = vect(0,0,0);
		GivePawn(Pawn);
	}

	// if OldTimeDelta corresponds to a lost packet, process it first
	if (  OldTimeDelta != 0 )
	{
		OldTimeStamp = TimeStamp - float(OldTimeDelta)/500 - 0.001;
		if ( CurrentTimeStamp < OldTimeStamp - 0.001 )
		{
			// split out components of lost move (approx)
			Accel.X = OldAccel >>> 23;
			if ( Accel.X > 127 )
				Accel.X = -1 * (Accel.X - 128);
			Accel.Y = (OldAccel >>> 15) & 255;
			if ( Accel.Y > 127 )
				Accel.Y = -1 * (Accel.Y - 128);
			Accel.Z = (OldAccel >>> 7) & 255;
			if ( Accel.Z > 127 )
				Accel.Z = -1 * (Accel.Z - 128);
			Accel *= 20;

			OldbRun = ( (OldAccel & 64) != 0 );
			OldbDoubleJump = ( (OldAccel & 32) != 0 );
			NewbPressedJump = ( (OldAccel & 16) != 0 );
			if ( NewbPressedJump )
				bJumpStatus = NewbJumpStatus;
			switch (OldAccel & 7)
			{
			case 0:
				OldDoubleClickMove = DCLICK_None;
				break;
			case 1:
				OldDoubleClickMove = DCLICK_Left;
				break;
			case 2:
				OldDoubleClickMove = DCLICK_Right;
				break;
			case 3:
				OldDoubleClickMove = DCLICK_Forward;
				break;
			case 4:
				OldDoubleClickMove = DCLICK_Back;
				break;
			}
			//log("Recovered move from "$OldTimeStamp$" acceleration "$Accel$" from "$OldAccel);
			OldTimeStamp = FMin(OldTimeStamp, CurrentTimeStamp + MaxResponseTime);
			MoveAutonomous(OldTimeStamp - CurrentTimeStamp, OldbRun, (bDuck == 1), NewbPressedJump, OldbDoubleJump, OldDoubleClickMove, Accel, rot(0,0,0));
			CurrentTimeStamp = OldTimeStamp;
		}
	}

	// View components
	ViewPitch = View/32768;
	ViewYaw = 2 * (View - 32768 * ViewPitch);
	ViewPitch *= 2;
	// Make acceleration.
	Accel = InAccel * 0.1;

	NewbPressedJump = (bJumpStatus != NewbJumpStatus);
	bJumpStatus = NewbJumpStatus;

	// Save move parameters.
	DeltaTime = FMin(MaxResponseTime,TimeStamp - CurrentTimeStamp);

	if ( Pawn == None )
	{
		ResetTimeMargin();
	}
	else if ( !CheckSpeedHack(DeltaTime) )
	{
		if ( !bWasSpeedHack )
		{
			if ( Level.TimeSeconds - LastSpeedHackLog > 20 )
			{
				log("Possible speed hack by "$PlayerReplicationInfo.PlayerName);
				LastSpeedHackLog = Level.TimeSeconds;
			}
			ClientMessage( "Speed Hack Detected!",'CriticalEvent' );
		}

		bWasSpeedHack = true;
		DeltaTime = 0;
		Pawn.Velocity = vect(0,0,0);
	}
	else if ( bWasSpeedHack )
	{
		// if have had a speedhack detection, then modify deltatime if getting too far ahead again
		if ( (TimeMargin > 0.5 * Level.MaxTimeMargin) && (Level.MaxTimeMargin > 0) )
			DeltaTime *= 0.8;
	}


	CurrentTimeStamp = TimeStamp;
	ServerTimeStamp = Level.TimeSeconds;
	ViewRot.Pitch = ViewPitch;
	ViewRot.Yaw = ViewYaw;
	ViewRot.Roll = 0;
	SetRotation(ViewRot);

	if ( NewbPressedJump || (InAccel != vect(0,0,0)) )
		LastActiveTime = Level.TimeSeconds;

	if ( AcknowledgedPawn != Pawn )
		return;

	if ( Pawn != None )
	{
		Rot.Roll = 256 * ClientRoll;
		Rot.Yaw = ViewYaw;
		if ( (Pawn.Physics == PHYS_Swimming) || (Pawn.Physics == PHYS_Flying) )
			maxPitch = 2;
		else
			maxPitch = 0;
		if ( (ViewPitch > maxPitch * RotationRate.Pitch) && (ViewPitch < 65536 - maxPitch * RotationRate.Pitch) )
		{
			If (ViewPitch < 32768)
				Rot.Pitch = maxPitch * RotationRate.Pitch;
		else
			Rot.Pitch = 65536 - maxPitch * RotationRate.Pitch;
		}
		else
			Rot.Pitch = ViewPitch;
		DeltaRot = (Rotation - Rot);
		Pawn.SetRotation(Rot);
		Pawn.DesiredRotation = Rotation;
	}

	// Perform actual movement
	if ( (Level.Pauser == None) && (DeltaTime > 0) )
		MoveAutonomous(DeltaTime, NewbRun, NewbDuck, NewbPressedJump, NewbDoubleJump, DoubleClickMove, Accel, DeltaRot);

	// Accumulate movement error.
	if ( ClientLoc == vect(0,0,0) )
		return;		// first part of double servermove
	else if ( Level.TimeSeconds - LastUpdateTime > 0.3 )
		ClientErr = 10000;
	else if ( Level.TimeSeconds - LastUpdateTime > 180.0/Player.CurrentNetSpeed )
	{
		if ( Pawn == None )
			LocDiff = Location - ClientLoc;
		else
			LocDiff = Pawn.Location - ClientLoc;
		ClientErr = LocDiff Dot LocDiff;
	}

	// If client has accumulated a noticeable positional error, correct him.
	if ( ClientErr > 3 )
	{
		if ( Pawn == None )
		{
			PendingAdjustment.newPhysics = Physics;
			PendingAdjustment.NewLoc = Location;
			PendingAdjustment.NewVel = Velocity;
		}
		else
		{
			PendingAdjustment.newPhysics = Pawn.Physics;
			PendingAdjustment.NewVel = Pawn.Velocity;
			PendingAdjustment.NewBase = Pawn.Base;
			if ( Mover(Pawn.Base) != None )
				PendingAdjustment.NewLoc = Pawn.Location - Pawn.Base.Location;
			else
				PendingAdjustment.NewLoc = Pawn.Location;
			PendingAdjustment.NewFloor = Pawn.Floor;
		}
		//log(" Client Error at "$TimeStamp$" is "$ClientErr$" with acceleration "$Accel$" LocDiff "$LocDiff$" Physics "$Pawn.Physics);
		LastUpdateTime = Level.TimeSeconds;

		PendingAdjustment.TimeStamp = TimeStamp;
		PendingAdjustment.newState = GetStateName();
	}
	//log("Server moved stamp "$TimeStamp$" location "$Pawn.Location$" Acceleration "$Pawn.Acceleration$" Velocity "$Pawn.Velocity);
}

/* Called on server at end of tick when PendingAdjustment has been set.
Done this way to avoid ever sending more than one ClientAdjustment per server tick.
*/
event SendClientAdjustment()
{
	if ( AcknowledgedPawn != Pawn )
	{
		PendingAdjustment.TimeStamp = 0;
		return;
	}

    if ( (Pawn == None) || (Pawn.Physics != PHYS_Spider) )
    {
        if ( PendingAdjustment.NewVel == vect(0,0,0) )
        {
            ShortClientAdjustPosition
            (
                PendingAdjustment.TimeStamp,
                PendingAdjustment.newState,
                PendingAdjustment.newPhysics,
                PendingAdjustment.NewLoc.X,
                PendingAdjustment.NewLoc.Y,
                PendingAdjustment.NewLoc.Z,
                PendingAdjustment.NewBase
            );
        }
        else
			ClientAdjustPosition
            (
                PendingAdjustment.TimeStamp,
                PendingAdjustment.newState,
                PendingAdjustment.newPhysics,
                PendingAdjustment.NewLoc.X,
                PendingAdjustment.NewLoc.Y,
                PendingAdjustment.NewLoc.Z,
                PendingAdjustment.NewVel.X,
                PendingAdjustment.NewVel.Y,
                PendingAdjustment.NewVel.Z,
                PendingAdjustment.NewBase
            );
		}
    else
		LongClientAdjustPosition
        (
            PendingAdjustment.TimeStamp,
            PendingAdjustment.newState,
            PendingAdjustment.newPhysics,
            PendingAdjustment.NewLoc.X,
            PendingAdjustment.NewLoc.Y,
            PendingAdjustment.NewLoc.Z,
            PendingAdjustment.NewVel.X,
            PendingAdjustment.NewVel.Y,
            PendingAdjustment.NewVel.Z,
            PendingAdjustment.NewBase,
            PendingAdjustment.NewFloor.X,
            PendingAdjustment.NewFloor.Y,
            PendingAdjustment.NewFloor.Z
        );

	PendingAdjustment.TimeStamp = 0;
}

// Only executed on server
function ServerDrive(float InForward, float InStrafe, float InUp, bool InJump)
{
	ProcessDrive(InForward, InStrafe, InUp, InJump);
}

function ProcessDrive(float InForward, float InStrafe, float InUp, bool InJump);
//{
	//Log("ProcessDrive Not Valid Outside State PlayerDriving");
//}

function ProcessMove ( float DeltaTime, vector newAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
{
    if ( (Pawn != None) && (Pawn.Acceleration != newAccel) )
		Pawn.Acceleration = newAccel;
}

final function MoveAutonomous
(
	float DeltaTime,
	bool NewbRun,
	bool NewbDuck,
	bool NewbPressedJump,
    bool NewbDoubleJump,
	eDoubleClickDir DoubleClickMove,
	vector newAccel,
	rotator DeltaRot
)
{
	if ( NewbRun )
		bRun = 1;
	else
		bRun = 0;

	if ( NewbDuck )
		bDuck = 1;
	else
		bDuck = 0;
	bPressedJump = NewbPressedJump;
    bDoubleJump = NewbDoubleJump;
	HandleWalking();
	ProcessMove(DeltaTime, newAccel, DoubleClickMove, DeltaRot);
	if ( Pawn != None )
		Pawn.AutonomousPhysics(DeltaTime);
	else
		AutonomousPhysics(DeltaTime);
    bDoubleJump = false;
	//log("Role "$Role$" moveauto time "$100 * DeltaTime$" ("$Level.TimeDilation$")");
}

/* VeryShortClientAdjustPosition
bandwidth saving version, when velocity is zeroed, and pawn is walking
*/
function VeryShortClientAdjustPosition
(
	float TimeStamp,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	Actor NewBase
)
{
	local vector Floor;

	if ( Pawn != None )
		Floor = Pawn.Floor;
	LongClientAdjustPosition(TimeStamp,'PlayerWalking',PHYS_Walking,NewLocX,NewLocY,NewLocZ,0,0,0,NewBase,Floor.X,Floor.Y,Floor.Z);
}

/* ShortClientAdjustPosition
bandwidth saving version, when velocity is zeroed
*/
function ShortClientAdjustPosition
(
	float TimeStamp,
	name newState,
	EPhysics newPhysics,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	Actor NewBase
)
{
	local vector Floor;

	if ( Pawn != None )
		Floor = Pawn.Floor;
	LongClientAdjustPosition(TimeStamp,newState,newPhysics,NewLocX,NewLocY,NewLocZ,0,0,0,NewBase,Floor.X,Floor.Y,Floor.Z);
}

/* ClientAdjustPosition
- pass newloc and newvel in components so they don't get rounded
*/
function ClientAdjustPosition
(
	float TimeStamp,
	name newState,
	EPhysics newPhysics,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	float NewVelX,
	float NewVelY,
	float NewVelZ,
	Actor NewBase
)
{
	local vector Floor;

	if ( Pawn != None )
		Floor = Pawn.Floor;
	LongClientAdjustPosition(TimeStamp,newState,newPhysics,NewLocX,NewLocY,NewLocZ,NewVelX,NewVelY,NewVelZ,NewBase,Floor.X,Floor.Y,Floor.Z);
}

/* LongClientAdjustPosition
long version, when care about pawn's floor normal
*/
function LongClientAdjustPosition
(
	float TimeStamp,
	name newState,
	EPhysics newPhysics,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	float NewVelX,
	float NewVelY,
	float NewVelZ,
	Actor NewBase,
	float NewFloorX,
	float NewFloorY,
	float NewFloorZ
)
{
    local vector NewLocation, NewVelocity, NewFloor;
	local Actor MoveActor;
    local SavedMove CurrentMove;
    local float NewPing;

	// update ping
	if ( (PlayerReplicationInfo != None) && !bDemoOwner )
	{
		NewPing = FMin(1.5, Level.TimeSeconds - TimeStamp);

		if ( ExactPing < 0.004 )
			ExactPing = FMin(0.3,NewPing);
		else
		{
			if ( NewPing > 2 * ExactPing )
				NewPing = FMin(NewPing, 3*ExactPing);
			ExactPing = FMin(0.99, 0.99 * ExactPing + 0.008 * NewPing); // placebo effect
		}
		PlayerReplicationInfo.Ping = Min(250.0 * ExactPing, 255);
		PlayerReplicationInfo.bReceivedPing = true;
		if ( Level.TimeSeconds - LastPingUpdate > 4 )
		{
			if ( bDynamicNetSpeed && (OldPing > DynamicPingThreshold * 0.001) && (ExactPing > DynamicPingThreshold * 0.001) )
			{
				if ( Level.MoveRepSize < 64 )
					Level.MoveRepSize += 8;
				else if ( Player.CurrentNetSpeed >= 6000 )
					SetNetSpeed(Player.CurrentNetSpeed - 1000);
				OldPing = 0;
			}
			else
				OldPing = ExactPing;
			LastPingUpdate = Level.TimeSeconds;
			ServerUpdatePing(1000 * ExactPing);
		}
	}
	if ( Pawn != None )
	{
		if ( Pawn.bTearOff )
		{
			Pawn = None;
			if ( !IsInState('GameEnded') && !IsInState('Dead') )
			{
			GotoState('Dead');
            }
			return;
		}
		MoveActor = Pawn;
        if ( (ViewTarget != Pawn)
			&& ((ViewTarget == self) || ((Pawn(ViewTarget) != None) && (Pawn(ViewTarget).Health <= 0))) )
		{
			bBehindView = false;
			SetViewTarget(Pawn);
		}
	}
	else
    {
		MoveActor = self;
 	   	if( GetStateName() != newstate )
		{
		    if ( NewState == 'GameEnded' )
			    GotoState(NewState);
			else if ( IsInState('Dead') )
			{
		    	if ( (NewState != 'PlayerWalking') && (NewState != 'PlayerSwimming') )
		        {
				    GotoState(NewState);
		        }
		        return;
			}
			else if ( NewState == 'Dead' )
				GotoState(NewState);
		}
	}
    if ( CurrentTimeStamp >= TimeStamp )
		return;
	CurrentTimeStamp = TimeStamp;

	NewLocation.X = NewLocX;
	NewLocation.Y = NewLocY;
	NewLocation.Z = NewLocZ;
    NewVelocity.X = NewVelX;
    NewVelocity.Y = NewVelY;
    NewVelocity.Z = NewVelZ;

	// skip update if no error
    CurrentMove = SavedMoves;
    while ( CurrentMove != None )
    {
        if ( CurrentMove.TimeStamp <= CurrentTimeStamp )
        {
            SavedMoves = CurrentMove.NextMove;
            CurrentMove.NextMove = FreeMoves;
            FreeMoves = CurrentMove;
			if ( CurrentMove.TimeStamp == CurrentTimeStamp )
			{
				FreeMoves.Clear();
				if ( (Mover(NewBase) != None) && (NewBase == CurrentMove.EndBase) )
				{
					if ( (GetStateName() == NewState)
						&& IsInState('PlayerWalking')
						&& ((MoveActor.Physics == PHYS_Walking) || (MoveActor.Physics == PHYS_Falling))
						&& ( VSize(CurrentMove.SavedRelativeLocation - NewLocation) < 3 ) )
					{

						CurrentMove = None;
						return;
					}
				}
				else if ( (VSize(CurrentMove.SavedLocation - NewLocation) < 3)
					&& (VSize(CurrentMove.SavedVelocity - NewVelocity) < 3)
					&& (GetStateName() == NewState)
					&& IsInState('PlayerWalking')
					&& ((MoveActor.Physics == PHYS_Walking) || (MoveActor.Physics == PHYS_Falling)) )
				{
					CurrentMove = None;
					return;
				}
				CurrentMove = None;
			}
			else
			{
				FreeMoves.Clear();
				CurrentMove = SavedMoves;
			}
        }
        else
			CurrentMove = None;
    }

	NewFloor.X = NewFloorX;
	NewFloor.Y = NewFloorY;
	NewFloor.Z = NewFloorZ;
	if ( Mover(NewBase) != None )
		NewLocation += NewBase.Location;

	if ( !bDemoOwner )
	{
	//log("Client "$Role$" adjust "$self$" stamp "$TimeStamp$" location "$MoveActor.Location);
	MoveActor.bCanTeleport = false;
    if ( !MoveActor.SetLocation(NewLocation) && (Pawn(MoveActor) != None)
		&& (MoveActor.CollisionHeight > Pawn(MoveActor).CrouchHeight)
		&& !Pawn(MoveActor).bIsCrouched
		&& (newPhysics == PHYS_Walking)
		&& (MoveActor.Physics != PHYS_Karma) && (MoveActor.Physics != PHYS_KarmaRagDoll) )
	{
		MoveActor.SetPhysics(newPhysics);
		Pawn(MoveActor).ForceCrouch();
	MoveActor.SetLocation(NewLocation);
	}
	MoveActor.bCanTeleport = true;
	}
	// Hack. Don't let network change physics mode of karma stuff on the client.
	if( (MoveActor.Physics != newPhysics) && (MoveActor.Physics != PHYS_Karma) && (MoveActor.Physics != PHYS_KarmaRagDoll)
		&& (newPhysics != PHYS_Karma) && (newPhysics != PHYS_KarmaRagDoll) )
	{
		MoveActor.SetPhysics(newPhysics);
	}
	if ( MoveActor != self )
		MoveActor.SetBase(NewBase, NewFloor);

    MoveActor.Velocity = NewVelocity;

	if( GetStateName() != newstate )
		GotoState(newstate);

	bUpdatePosition = true;
}

function ServerUpdatePing(int NewPing)
{
	PlayerReplicationInfo.Ping = Min(0.25 * NewPing, 250);
	PlayerReplicationInfo.bReceivedPing = true;
}

function ClientUpdatePosition()
{
	local SavedMove CurrentMove;
	local int realbRun, realbDuck;
	local bool bRealJump;

	// Dont do any network position updates on things running PHYS_Karma
	if( Pawn != None && (Pawn.Physics == PHYS_Karma || Pawn.Physics == PHYS_KarmaRagDoll) )
		return;

	bUpdatePosition = false;
	realbRun= bRun;
	realbDuck = bDuck;
	bRealJump = bPressedJump;
	CurrentMove = SavedMoves;
	bUpdating = true;

	while ( CurrentMove != None )
	{
		if ( (PendingMove == CurrentMove) && (Pawn != None) )
			PendingMove.SetInitialPosition(Pawn);
		if ( CurrentMove.TimeStamp <= CurrentTimeStamp )
		{
			SavedMoves = CurrentMove.NextMove;
			CurrentMove.NextMove = FreeMoves;
			FreeMoves = CurrentMove;
			FreeMoves.Clear();
			CurrentMove = SavedMoves;
		}
		else
		{
            MoveAutonomous(CurrentMove.Delta, CurrentMove.bRun, CurrentMove.bDuck, CurrentMove.bPressedJump, CurrentMove.bDoubleJump, CurrentMove.DoubleClickMove, CurrentMove.Acceleration, rot(0,0,0));
			if ( Pawn != None )
			{
				CurrentMove.SavedLocation = Pawn.Location;
				CurrentMove.SavedVelocity = Pawn.Velocity;
				CurrentMove.EndBase = Pawn.Base;
				if ( (CurrentMove.EndBase != None) && !CurrentMove.EndBase.bWorldGeometry )
					CurrentMove.SavedRelativeLocation = Pawn.Location - CurrentMove.EndBase.Location;
			}
			CurrentMove = CurrentMove.NextMove;
		}
	}

	bUpdating = false;
	bDuck = realbDuck;
	bRun = realbRun;
	bPressedJump = bRealJump;
}

final function SavedMove GetFreeMove()
{
	local SavedMove s, first;
	local int i;

	if ( FreeMoves == None )
	{
        // don't allow more than 100 saved moves
		For ( s=SavedMoves; s!=None; s=s.NextMove )
		{
			i++;
            if ( i > 100 )
			{
				first = SavedMoves;
				SavedMoves = SavedMoves.NextMove;
				first.Clear();
				first.NextMove = None;
				// clear out all the moves
				While ( SavedMoves != None )
				{
					s = SavedMoves;
					SavedMoves = SavedMoves.NextMove;
					s.Clear();
					s.NextMove = FreeMoves;
					FreeMoves = s;
				}
				return first;
			}
		}
		return Spawn(class'SavedMove');
	}
	else
	{
		s = FreeMoves;
		FreeMoves = FreeMoves.NextMove;
		s.NextMove = None;
		return s;
	}
}

function int CompressAccel(int C)
{
	if ( C >= 0 )
		C = Min(C, 127);
	else
		C = Min(abs(C), 127) + 128;
	return C;
}

/*
========================================================================
Here's how player movement prediction, replication and correction works in network games:

Every tick, the PlayerTick() function is called.  It calls the PlayerMove() function (which is implemented
in various states).  PlayerMove() figures out the acceleration and rotation, and then calls ProcessMove()
(for single player or listen servers), or ReplicateMove() (if its a network client).

ReplicateMove() saves the move (in the PendingMove list), calls ProcessMove(), and then replicates the move
to the server by calling the replicated function ServerMove() - passing the movement parameters, the client's
resultant position, and a timestamp.

ServerMove() is executed on the server.  It decodes the movement parameters and causes the appropriate movement
to occur.  It then looks at the resulting position and if enough time has passed since the last response, or the
position error is significant enough, the server calls ClientAdjustPosition(), a replicated function.

ClientAdjustPosition() is executed on the client.  The client sets its position to the servers version of position,
and sets the bUpdatePosition flag to true.

When PlayerTick() is called on the client again, if bUpdatePosition is true, the client will call
ClientUpdatePosition() before calling PlayerMove().  ClientUpdatePosition() replays all the moves in the pending
move list which occured after the timestamp of the move the server was adjusting.
*/

//
// Replicate this client's desired movement to the server.
//
function ReplicateMove
(
 float DeltaTime,
 vector NewAccel,
 eDoubleClickDir DoubleClickMove,
 rotator DeltaRot
 )
{
	local SavedMove NewMove, OldMove, AlmostLastMove, LastMove;
	local byte ClientRoll;
	local float OldTimeDelta, NetMoveDelta;
	local int OldAccel;
	local vector BuildAccel, AccelNorm, MoveLoc, CompareAccel;
	local bool bPendingJumpStatus;

	DeltaTime = FMin(DeltaTime, MaxResponseTime);
	// find the most recent move, and the most recent interesting move
	if ( SavedMoves != None )
	{
		LastMove = SavedMoves;
		AlmostLastMove = LastMove;
		AccelNorm = Normal(NewAccel);
		while ( LastMove.NextMove != None )
		{
			// find most recent interesting move to send redundantly
			if ( LastMove.IsJumpMove() )
			{
				OldMove = LastMove;
			}
			else if ( (Pawn != None) && ((OldMove == None) || !OldMove.IsJumpMove()) )
			{
				// see if acceleration direction changed
				if ( OldMove != None )
					CompareAccel = Normal(OldMove.Acceleration);
				else
					CompareAccel = AccelNorm;

				if ( (LastMove.Acceleration != CompareAccel) && ((normal(LastMove.Acceleration) Dot CompareAccel) < 0.95) )
				OldMove = LastMove;
			}

			AlmostLastMove = LastMove;
			LastMove = LastMove.NextMove;
		}
		if ( LastMove.IsJumpMove() )
		{
			OldMove = LastMove;
		}
		else if ( (Pawn != None) && ((OldMove == None) || !OldMove.IsJumpMove()) )
		{
			// see if acceleration direction changed
			if ( OldMove != None )
				CompareAccel = Normal(OldMove.Acceleration);
			else
				CompareAccel = AccelNorm;

			if ( (LastMove.Acceleration != CompareAccel) && ((normal(LastMove.Acceleration) Dot CompareAccel) < 0.95) )
				OldMove = LastMove;
		}
	}

    // Get a SavedMove actor to store the movement in.
	NewMove = GetFreeMove();
	if ( NewMove == None )
		return;
	NewMove.SetMoveFor(self, DeltaTime, NewAccel, DoubleClickMove);

	// Simulate the movement locally.
	bDoubleJump = false;
	ProcessMove(NewMove.Delta, NewMove.Acceleration, NewMove.DoubleClickMove, DeltaRot);

	// see if the two moves could be combined
	if ( (PendingMove != None) && (Pawn != None) && (Pawn.Physics == PHYS_Walking)
		&& (NewMove.Delta + PendingMove.Delta < MaxResponseTime)
		&& (NewAccel != vect(0,0,0))
		&& (PendingMove.SavedPhysics == PHYS_Walking)
		&& !PendingMove.bPressedJump && !NewMove.bPressedJump
		&& (PendingMove.bRun == NewMove.bRun)
		&& (PendingMove.bDuck == NewMove.bDuck)
		&& (PendingMove.bDoubleJump == NewMove.bDoubleJump)
		&& (PendingMove.DoubleClickMove == DCLICK_None)
		&& (NewMove.DoubleClickMove == DCLICK_None)
		&& ((Normal(PendingMove.Acceleration) Dot Normal(NewAccel)) > 0.99) )
	{
		Pawn.SetLocation(PendingMove.GetStartLocation());
		Pawn.Velocity = PendingMove.StartVelocity;
		if ( PendingMove.StartBase != Pawn.Base);
		Pawn.SetBase(PendingMove.StartBase);
		Pawn.Floor = PendingMove.StartFloor;
		NewMove.Delta += PendingMove.Delta;

		NewMove.SetInitialPosition( Pawn );

		// remove pending move from move list
		if ( LastMove == PendingMove )
		{
			if ( SavedMoves == PendingMove )
			{
				SavedMoves.NextMove = FreeMoves;
				FreeMoves = SavedMoves;
				SavedMoves = None;
			}
			else
			{
				PendingMove.NextMove = FreeMoves;
				FreeMoves = PendingMove;
				if ( AlmostLastMove != None )
				{
					AlmostLastMove.NextMove = None;
					LastMove = AlmostLastMove;
				}
			}
			FreeMoves.Clear();
		}
		PendingMove = None;
	}

	if ( Pawn != None )
		Pawn.AutonomousPhysics(NewMove.Delta);
	else
		AutonomousPhysics(DeltaTime);
	NewMove.PostUpdate(self);

	if ( SavedMoves == None )
		SavedMoves = NewMove;
	else
		LastMove.NextMove = NewMove;

	if ( PendingMove == None )
	{
		// Decide whether to hold off on move
		if ( (Player.CurrentNetSpeed > 10000) && (GameReplicationInfo != None) && (GameReplicationInfo.PRIArray.Length <= 10) )
			NetMoveDelta = 0;	// full rate
		else
			NetMoveDelta = FMax(0.0222,2 * Level.MoveRepSize/Player.CurrentNetSpeed);

		if ( Level.TimeSeconds - ClientUpdateTime < NetMoveDelta )
		{
			PendingMove = NewMove;
			return;
		}
	}

	ClientUpdateTime = Level.TimeSeconds;

	// check if need to redundantly send previous move
	if ( OldMove != None )
	{
		// old move important to replicate redundantly
		OldTimeDelta = FMin(255, (Level.TimeSeconds - OldMove.TimeStamp) * 500);
		BuildAccel = 0.05 * OldMove.Acceleration + vect(0.5, 0.5, 0.5);
		OldAccel = (CompressAccel(BuildAccel.X) << 23)
			+ (CompressAccel(BuildAccel.Y) << 15)
			+ (CompressAccel(BuildAccel.Z) << 7);
		if ( OldMove.bRun )
			OldAccel += 64;
		if ( OldMove.bDoubleJump )
			OldAccel += 32;
		if ( OldMove.bPressedJump )
			OldAccel += 16;
		OldAccel += OldMove.DoubleClickMove;
	}

	// Send to the server
	ClientRoll = (Rotation.Roll >> 8) & 255;
	if ( PendingMove != None )
	{
		if ( PendingMove.bPressedJump )
			bJumpStatus = !bJumpStatus;
		bPendingJumpStatus = bJumpStatus;
	}
	if ( NewMove.bPressedJump )
		bJumpStatus = !bJumpStatus;

	if ( Pawn == None )
		MoveLoc = Location;
	else
		MoveLoc = Pawn.Location;

	CallServerMove
		(
		NewMove.TimeStamp,
		NewMove.Acceleration * 10,
		MoveLoc,
		NewMove.bRun,
		NewMove.bDuck,
		bPendingJumpStatus,
		bJumpStatus,
		NewMove.bDoubleJump,
		NewMove.DoubleClickMove,
		ClientRoll,
		(32767 & (Rotation.Pitch/2)) * 32768 + (32767 & (Rotation.Yaw/2)),
		OldTimeDelta,
		OldAccel
		);
	PendingMove = None;
}

function CallServerMove
(
    float TimeStamp,
    vector InAccel,
    vector ClientLoc,
    bool NewbRun,
    bool NewbDuck,
    bool NewbPendingJumpStatus,
    bool NewbJumpStatus,
    bool NewbDoubleJump,
    eDoubleClickDir DoubleClickMove,
    byte ClientRoll,
    int View,
    optional byte OldTimeDelta,
    optional int OldAccel
)
{
	local byte PendingCompress;
	local bool bCombine;

	if ( PendingMove != None )
	{
		if ( PendingMove.bRun )
			PendingCompress = 1;
		if ( PendingMove.bDuck )
			PendingCompress += 2;
		if ( NewbPendingJumpStatus )
			PendingCompress += 4;
		if ( PendingMove.bDoubleJump )
			PendingCompress += 8;
		if ( NewbRun )
			PendingCompress += 16;
		if ( NewbDuck )
			PendingCompress += 32;
		if ( NewbJumpStatus )
			PendingCompress += 64;
		if ( NewbDoubleJump )
			PendingCompress += 128;

		// send two moves simultaneously
		if ( (InAccel == vect(0,0,0))
			&& (PendingMove.StartVelocity == vect(0,0,0))
			&& (DoubleClickMove == DCLICK_None)
			&& (PendingMove.Acceleration == vect(0,0,0)) && (PendingMove.DoubleClickMove == DCLICK_None) && !PendingMove.bDoubleJump )
		{
			if ( Pawn == None )
				bCombine = (Velocity == vect(0,0,0));
			else
				bCombine = (Pawn.Velocity == vect(0,0,0));

			if ( bCombine )
			{
				if ( OldTimeDelta == 0 )
				{
		ShortServerMove
		(
						TimeStamp,
						ClientLoc,
						NewbRun,
						NewbDuck,
						NewbJumpStatus,
						ClientRoll,
						View
					);
				}
				else
				{
					ServerMove
					(
						TimeStamp,
						InAccel,
						ClientLoc,
						NewbRun,
						NewbDuck,
						NewbJumpStatus,
						NewbDoubleJump,
						DoubleClickMove,
						ClientRoll,
						View,
						OldTimeDelta,
						OldAccel
					);
				}
				return;
			}
		}

		if ( OldTimeDelta == 0 )
			DualServerMove
			(
				PendingMove.TimeStamp,
				PendingMove.Acceleration * 10,
				PendingCompress,
				PendingMove.DoubleClickMove,
				(32767 & (PendingMove.Rotation.Pitch/2)) * 32768 + (32767 & (PendingMove.Rotation.Yaw/2)),
				TimeStamp,
				InAccel,
				ClientLoc,
				DoubleClickMove,
			ClientRoll,
				View
		);
	else
			DualServerMove
			(
				PendingMove.TimeStamp,
				PendingMove.Acceleration * 10,
				PendingCompress,
				PendingMove.DoubleClickMove,
				(32767 & (PendingMove.Rotation.Pitch/2)) * 32768 + (32767 & (PendingMove.Rotation.Yaw/2)),
				TimeStamp,
				InAccel,
				ClientLoc,
				DoubleClickMove,
				ClientRoll,
				View,
				OldTimeDelta,
				OldAccel
			);
	}
	else if ( OldTimeDelta != 0 )
	{
		ServerMove
		(
            TimeStamp,
            InAccel,
            ClientLoc,
            NewbRun,
            NewbDuck,
            NewbJumpStatus,
            NewbDoubleJump,
            DoubleClickMove,
			ClientRoll,
            View,
			OldTimeDelta,
			OldAccel
		);
}
    else if ( (InAccel == vect(0,0,0)) && (DoubleClickMove == DCLICK_None) && !NewbDoubleJump )
    {
		ShortServerMove
		(
			TimeStamp,
			ClientLoc,
			NewbRun,
			NewbDuck,
			NewbJumpStatus,
			ClientRoll,
			View
		);
    }
    else
		ServerMove
        (
            TimeStamp,
            InAccel,
            ClientLoc,
            NewbRun,
            NewbDuck,
            NewbJumpStatus,
            NewbDoubleJump,
            DoubleClickMove,
            ClientRoll,
            View
        );
}

function HandleWalking()
{
	if ( Pawn != None )
		Pawn.SetWalking( (bRun != 0) && !Region.Zone.IsA('WarpZoneInfo') );
}

function ServerRestartGame()
{
}

function SetFOVAngle(float newFOV)
{
	FOVAngle = newFOV;
	if( Pawn != None && Pawn.Weapon != None )
	{
		Pawn.Weapon.SetWeapFOV(FOVAngle);
	}
}

function ClientFlash( float scale, vector fog )
{
    FlashScale = scale * vect(1,1,1);
    flashfog = 0.001 * fog;
}

function ClientAdjustGlow( float scale, vector fog )
{
	ConstantGlowScale += scale;
	ConstantGlowFog += 0.001 * fog;
}

exec function StopShake()
{
	ClientStopShake();
}

private function ClientStopShake()
{
	ShakeOffsetMag = vect(0,0,0);
	ShakeRotMag = vect(0,0,0);
	ShakeFrequency = 1.0;
	ShakeTimeIn = 0;
	ShakeTimeSustain = 0;
	ShakeTimeOut = 0;
	ShakeTimeElapsed = -1;
}

exec event ShakeView(float InTime, float SustainTime, float OutTime, float XMag, float YMag, float ZMag, float YawMag, float PitchMag, float Frequency )
{
	local vector NewShakeOffset;
	local vector NewShakeRotation;
	local float LerpFactor;

	if( ShakeTimeElapsed != -1 )
	{
		NewShakeOffset.X = XMag;
		NewShakeOffset.Y = YMag;
		NewShakeOffset.Z = ZMag;

		NewShakeRotation.X = YawMag;
		NewShakeRotation.Y = PitchMag;
		NewShakeRotation.Z = 0;

		if( ShakeTimeElapsed < ShakeTimeIn )
			LerpFactor = ShakeTimeElapsed / ShakeTimeIn;
		else if( ShakeTimeElapsed < ShakeTimeSustain )
			LerpFactor = 1;
		else if( ShakeTimeElapsed < ShakeTimeOut )
			LerpFactor = 1 - ShakeTimeElapsed / ShakeTimeOut;

		if( VSize( NewShakeOffset ) < VSize( ShakeOffsetMag * LerpFactor ) ||
			VSize( NewShakeRotation ) < VSize( ShakeRotMag * LerpFactor ) )
		{
			return;
		}
	}

	ClientShake( InTime, SustainTime, OutTime, XMag, YMag, ZMag, YawMag, PitchMag, Frequency );
}

private function ClientShake( float InTime, float SustainTime, float OutTime, float XMag, float YMag, float ZMag, float YawMag, float PitchMag, float Frequency )
{
	ShakeOffsetMag.X = XMag;
	ShakeOffsetMag.Y = YMag;
	ShakeOffsetMag.Z = ZMag;

	ShakeRotMag.X = YawMag;
	ShakeRotMag.Y = PitchMag;
	ShakeRotMag.Z = 0;

	ShakeFrequency = Frequency;
	ShakeTimeElapsed = 0;
	ShakeTimeIn = InTime;
	ShakeTimeSustain = ShakeTimeIn + SustainTime;
	ShakeTimeOut = ShakeTimeSustain + OutTime;

	ShakeRotLastNoise = vect(0,0,0);
	ShakeOffsetLastNoise = vect(0,0,0);
}

event ViewShake(float DeltaTime)
{
	local Rotator ViewRotation;
	local vector NewRotNoise;
	local vector NewOffsetNoise;
	local float LerpFactor;

	if(	ShakeTimeElapsed == -1 )
		return;

	if( ShakeTimeElapsed < ShakeTimeIn )
		LerpFactor = ShakeTimeElapsed / ShakeTimeIn;
	else if( ShakeTimeElapsed < ShakeTimeSustain )
		LerpFactor = 1;
	else if( ShakeTimeElapsed < ShakeTimeOut )
		LerpFactor = 1 - ShakeTimeElapsed / ShakeTimeOut;

	NewRotNoise.X = FClamp( (frand() * 2 - 1) - ShakeRotLastNoise.X, -ShakeFrequency, ShakeFrequency );
	NewRotNoise.Y = FClamp( (frand() * 2 - 1) - ShakeRotLastNoise.Y, -ShakeFrequency, ShakeFrequency );
	NewRotNoise.Z = FClamp( (frand() * 2 - 1) - ShakeRotLastNoise.Z, -ShakeFrequency, ShakeFrequency );

	ViewRotation = Rotation;
	ViewRotation.Pitch	+= ShakeRotMag.X * ( ShakeRotLastNoise.X + NewRotNoise.X ) * LerpFactor;
	ViewRotation.Yaw	+= ShakeRotMag.Y * ( ShakeRotLastNoise.Y + NewRotNoise.Y ) * LerpFactor;

	NewOffsetNoise.X = FClamp( (frand() * 2 - 1) - ShakeOffsetLastNoise.X, -ShakeFrequency, ShakeFrequency );
	NewOffsetNoise.Y = FClamp( (frand() * 2 - 1) - ShakeOffsetLastNoise.Y, -ShakeFrequency, ShakeFrequency );
	NewOffsetNoise.Z = FClamp( (frand() * 2 - 1) - ShakeOffsetLastNoise.Z, -ShakeFrequency, ShakeFrequency );

	ShakeOffset.X = ShakeOffsetMag.X * ( ShakeOffsetLastNoise.X + NewOffsetNoise.X ) * LerpFactor;
	ShakeOffset.Y = ShakeOffsetMag.Y * ( ShakeOffsetLastNoise.Y + NewOffsetNoise.Y ) * LerpFactor;
	ShakeOffset.Z = ShakeOffsetMag.Z * ( ShakeOffsetLastNoise.Z + NewOffsetNoise.Z ) * LerpFactor;

	ShakeRotLastNoise = ShakeRotLastNoise + NewRotNoise;
	ShakeOffsetLastNoise = ShakeOffsetLastNoise + NewOffsetNoise;

	SetRotation(ViewRotation);

	ShakeTimeElapsed += DeltaTime;
}

function Typing( bool bTyping )
{
}

function LimitRotation( int MaxPitch, int MinPitch, int MaxYaw, int MinYaw )
{
	MaxPlayerPitch = MaxPitch;
	MinPlayerPitch = MinPitch;
	MaxPlayerYaw = MaxYaw;
	MinPlayerYaw = MinYaw;
}

//************************************************************************************
// Normal gameplay execs
// Type the name of the exec function at the console to execute it

exec function Jump( optional float F )
{
	if ( Level.Pauser == PlayerReplicationInfo )
		SetPause(False);
	else
		bPressedJump = true;
}

// Send a voice message of a certain type to a certain player.
exec function Speech( name Type, int Index, string Callsign )
{
	if (PlayerReplicationInfo.VoiceType != None)
		class'VoicePack'.static.PlayerSpeech( Type, Index, Callsign, Self );
}

exec function RestartLevel()
{
	if( Level.Netmode==NM_Standalone )
		ClientTravel( "?restart", TRAVEL_Relative, false );
}

exec function LocalTravel( string URL )
{
	if( Level.Netmode==NM_Standalone )
		ClientTravel( URL, TRAVEL_Relative, true );
}

function bool RestartGame()
{
	if (IsDead())
	{
		// Override game info.
		if (LastSave != "")
			LoadGame(LastSave);
		else
			RestartLevel();
		return true;
	}
	else
		return false;
}

// ------------------------------------------------------------------------
// Loading and saving

simulated native function GetSaveGames( string Prefix, out Array<string> SaveGames, out Array<string> DateTime );
simulated native function DeleteSaveGame( string SaveGame );
simulated native function bool SaveGameExists( string SaveGame );
simulated native function int CompareSaveGameTimes( string Save1, string Save2);
simulated native function GetMostRecentSaveGame( string Prefix, out string SaveGame );
simulated native function bool VerifySaveGame( string SaveGame );

simulated native function string GetCurrentProfileName();

simulated native function string GetGlobalSettingsSaveName();

simulated native function ExitLevel(optional String MenuClass);
simulated native function string GetMissionStart( string Level );

simulated function RestartMission()
{
	local string CurrentLevel;
	local string MissionStartLevel;

	GetCurrentMapName( CurrentLevel );
	MissionStartLevel = GetMissionStart( CurrentLevel );
	if ( MissionStartLevel != "" )
	{
		ConsoleCommand( "open " $ MissionStartLevel );
	}
}

simulated function int CurrentProfileNumSaves()
{
	local string profileName;
	local string prefix;
	local array<string> SaveGames;
	local array<string> DateTimes;

	// Check to see if there are any saves
	profileName = GetCurrentProfileName();
	prefix = profileName $ "_";

	GetSaveGames( prefix, SaveGames, DateTimes );

	CachedNumSaves = SaveGames.Length;

	return CachedNumSaves;
}

exec function QuickSave()
{
	local string saveName;
	local string LevelName;

	// Quick and dirty hack to disable saving in the prologue
	GetCurrentMapName( LevelName );
	LevelName = Caps( LevelName );
	// Strip any extension off the level name
	if ( Caps(Right(LevelName, 4)) == ".CTM" )
		LevelName = Left(LevelName, Len(LevelName) - 4);
	if ( LevelName == "PRO" )
		return;

	saveName = GetCurrentProfileName() $ "_" $ QuickSaveName;
	LastSave = saveName;
	ConsoleCommand("SaveGame " $ saveName);
}

exec function bool HasQuickSave()
{
	local string qSaveName;

	qSaveName = GetCurrentProfileName() $ "_" $ QuickSaveName;

	return SaveGameExists( qSaveName );
}

function bool CanAutoSave()
{
	local bool bDVDDemo;
	local bool bGotValue;
	local string value;
	local string saveName;

	bGotValue = GetConfigValue("Engine.GameEngine", "DVDDemo", value );
	if ( bGotValue )
	{
		bDVDDemo = bool(value);
		if ( bDVDDemo )
			return False;
	}

	saveName = GetCurrentProfileName() $ "_" $ AutoSaveName;

	if ( IsOnConsole() )
	{
		if ( !SaveGameExists( saveName ) )
		{
			// Check to make sure we've got space for it...
			if ( !HaveAdequateDiscSpace( True, False ) )
			{
				return False;
			}
		}
	}

	return True;
}


function AutoSave()
{
	local bool bDVDDemo;
	local bool bGotValue;
	local string value;
	local string saveName;

	bGotValue = GetConfigValue("Engine.GameEngine", "DVDDemo", value );
	if ( bGotValue )
	{
		bDVDDemo = bool(value);
		if ( bDVDDemo )
			return;
	}

	saveName = GetCurrentProfileName() $ "_" $ AutoSaveName;

	if ( IsOnConsole() )
	{
		if ( !SaveGameExists( saveName ) )
		{
			// Check to make sure we've got space for it...
			if ( !HaveAdequateDiscSpace( True, False ) )
			{
				//** No need to show this, as they've already seen this and
				//	 opted to continue from the menus...
				//ClientShowSingularMenu( "XInterfaceCommon.MenuLowStorage", "" );
				//**
				return;
			}
		}
	}

	LastSave = saveName;
	ConsoleCommand("SaveGame " $ saveName);
	// Refresh the number of existing saves
	CurrentProfileNumSaves();
}

exec function LoadGame( string Name )
{
	if ( Level.NetMode == NM_Standalone )
		ClientTravel( "?load=" $Name, TRAVEL_Absolute, false);
}

exec function QuickLoad()
{
	local string qSaveName;

	qSaveName = GetCurrentProfileName() $ "_" $ QuickSaveName;

	if ( SaveGameExists( qSaveName ) )
		LoadGame( qSaveName );
}

exec event LoadMostRecent()
{
	local String SaveName;
	local String Prefix;

	Prefix = GetCurrentProfileName() $ "_";

	GetMostRecentSaveGame( Prefix, SaveName );

	if ( SaveName != "" )
		LoadGame( SaveName );


/*
	local string qSaveName;
	local string aSaveName;
	local bool bQSaveExists;
	local bool bASaveExists;

	local int result;

	qSaveName = GetCurrentProfileName() $ "_" $ QuickSaveName;
	aSaveName = GetCurrentProfileName() $ "_" $ AutoSaveName;

	bQSaveExists = SaveGameExists( qSaveName );
	bASaveExists = SaveGameExists( aSaveName );

	if ( bQSaveExists && bASaveExists )
	{
		result = CompareSaveGameTimes( qSaveName, aSaveName );
		if ( result >= 0 )
		{
log("Loading "$qSaveName$", because it's newer than "$aSaveName);
			LoadGame( qSaveName );
		}
		else
		{
log("Loading "$aSaveName$", because it's newer than "$qSaveName);
			LoadGame( aSaveName );
		}
	}
	else if ( bQSaveExists )
	{
log("Loading "$qSaveName$", because there's no AutoSave");
		LoadGame( qSaveName );
	}
	else if ( bASaveExists )
	{
log("Loading "$aSaveName$", because there's no QuickSave");
		LoadGame( aSaveName );
	}
	else
	{
log("No QuickSave or AutoSave");
		//ClientOpenMenu( "XInterfaceCommon.MenuWarning", false, NoQuickOrAutoSave );
	}
	*/
}

exec function Reload()
{
	if (LastSave != "")
		LoadGame(LastSave);
	else
		RestartLevel();
}

native function bool ReleaseAllButtons( );

/* SetPause()
 Try to pause game; returns success indicator.
 Replicated to server in network games.
 */
function bool ServerSetPause(BOOL bPause)
{
    bFire = 0;
    bAltFire = 0;
	return Level.Game.SetPause(bPause, self);
}

function bool SetPause( BOOL bPause )
{
	if( bPause == True )
	{
		ReleaseAllButtons();
		// addition
		bFire = 0;
		bAltFire = 0;
		bDuck = 0;
	}

    return ServerSetPause( bPause );
}

/* Pause()
Command to try to pause the game.
*/
exec function Pause()
{
	// Pause if not already
	if(Level.Pauser == None)
		SetPause(true);
	else
		SetPause(false);
}

exec function bool IsPaused()
{
	return (Level.Pauser != None);
}

exec function ShowTitleCardMenu( String MenuClass, String Args )
{
	if (/* NO_GUI_UI
		Player.GUIController == None &&
		*/
		Player.Actor == None )
		return;

	// Pause if not already
	if(Level.Pauser == None)
		SetPause(true);

	StopForceFeedback();  // jdf - no way to pause feedback

	// Open menu
	ClientOpenMenu( MenuClass, False, Args );
}

exec event ShowMenu(optional String MenuClass, optional String Title, optional String Text, optional String PicName, optional String NewLevel)
{
	local bool bIPaused;
	if (/* NO_GUI_UI
		Player.GUIController == None &&
		*/
		Player.Actor == None )
		return;

	if ( Level.TimeSeconds <= 0 )
		return;

	// Pause if not already
	if(Level.Pauser == None)
	{
		SetPause(true);
		bIPaused = true;
	}
	else
		bIPaused = false;

	StopForceFeedback();  // jdf - no way to pause feedback

	// Open menu
	if ( MenuClass != "" )
		ClientOpenMenu( MenuClass );
	else if (bIPaused)   // don't allow other playercontrollers other than the pausing one to do this.
		ClientOpenMenu( GetPauseMenuClass() );

	if ( Player != None &&
	     Player.Console != None &&
	     Player.Console.CurMenu != None )
	{
		Player.Console.CurMenu.SetInfoOptions( PicName, Title, Text, NewLevel, False );
	}
}

// Activate specific inventory item
exec function ActivateInventoryItem( class InvItem )
{
	local Powerups Inv;

	Inv = Powerups(Pawn.FindInventoryType(InvItem));
	if ( Inv != None )
		Inv.Activate();
}

// ------------------------------------------------------------------------
// Weapon changing functions

/* ThrowWeapon()
Throw out current weapon, and switch to a new weapon
*/
exec function ThrowWeapon()
{
    if ( (Pawn == None) || (Pawn.Weapon == None) )
		return;

    ServerThrowWeapon(Pawn.Weapon);
}

function ServerThrowWeapon(Weapon WeaponToThrow)
{
    local Vector TossVel;

    if (Pawn.CanThrowWeapon(WeaponToThrow))
    {
        TossVel = Vector(GetViewRotation());
        TossVel = TossVel * ((Pawn.Velocity Dot TossVel) + 500) + Vect(0,0,200);
        Pawn.TossWeapon(WeaponToThrow, TossVel);
        ClientSwitchToBestWeapon();
    }
}

/* PrevWeapon()
- switch to previous inventory group weapon
*/
exec function PrevWeapon()
{
	if( Level.Pauser!=None )
		return;

	if ( Pawn.Weapon == None )
	{
		SwitchToBestWeapon();
		return;
	}

	if( Level.NetMode == NM_StandAlone )
	{
		if ( Pawn.PendingWeapon != None )
			Pawn.PendingWeapon = Pawn.Inventory.PrevWeapon(None, Pawn.PendingWeapon);
		else
			Pawn.PendingWeapon = Pawn.Inventory.PrevWeapon(None, Pawn.Weapon);

		if ( Pawn.PendingWeapon != None )
			Pawn.Weapon.PutDown(Pawn.PendingWeapon);
	}
	else
		ToggleWeapon();
}

/* NextWeapon()
- switch to next inventory group weapon
*/
exec function NextWeapon()
{
	if( Level.Pauser!=None )
		return;

	if ( Pawn.Weapon == None )
	{
		SwitchToBestWeapon();
		return;
	}

	if( Level.NetMode == NM_StandAlone )
	{
		if ( Pawn.PendingWeapon != None )
			Pawn.PendingWeapon = Pawn.Inventory.NextWeapon(None, Pawn.PendingWeapon);
		else
			Pawn.PendingWeapon = Pawn.Inventory.NextWeapon(None, Pawn.Weapon);

		if ( Pawn.PendingWeapon != None )
			Pawn.Weapon.PutDown(Pawn.PendingWeapon);
	}
	else
		ToggleWeapon();

}

exec function PipedSwitchWeapon(byte F)
{
	if ( (Pawn == None) || (Pawn.PendingWeapon != None) )
		return;
	SwitchWeapon(F);
}

simulated function SetOkToSwitchWeapon(bool bOK)
{
	bOkToSwitchWeapon = bOK;
}

exec function ToggleWeapon()
{
	if( Level.NetMode == NM_StandAlone && !Level.IsSplitScreen() )
	{
        if( Pawn.Weapon.InventoryGroup == 5 )
			SwitchWeapon( 1 );
		else
			SwitchWeapon( 5 );
	}
	else
		SwitchWeapon( 5 );

}

// The player wants to switch to weapon group number F.
exec function SwitchWeapon (byte F )
{
	if ( (Level.Pauser!=None) || (Pawn == None) || (Pawn.Inventory == None) || !bOkToSwitchWeapon )
		return;

	ForceWeaponSwitch(F);
}

simulated function ForceWeaponSwitch(byte F)
{
	local weapon newWeapon;

	if ( (Pawn != None) && (Pawn.Inventory != None) )
		newWeapon = Pawn.Inventory.WeaponChange(F, false);
	else
		newWeapon = None;

	if ( newWeapon == None )
		return;

	if ( Pawn.Weapon == None )
	{
		Pawn.PendingWeapon = newWeapon;
		Pawn.ChangedWeapon();
	}
    else if ( Pawn.Weapon != newWeapon || Pawn.PendingWeapon != None )
	{
		Pawn.PendingWeapon = newWeapon;
		if ( !Pawn.Weapon.PutDown(newWeapon) )
			Pawn.PendingWeapon = None;
	}
}

exec function GetWeapon(class<Weapon> NewWeaponClass )
{
	local Inventory Inv;
    local int Count;

	if ( (Pawn.Inventory == None) || (NewWeaponClass == None)
		|| ((Pawn.Weapon != None) && (Pawn.Weapon.Class == NewWeaponClass)) )
		return;

	for ( Inv=Pawn.Inventory; Inv!=None; Inv=Inv.Inventory )
    {
		if ( Inv.Class == NewWeaponClass )
		{
			Pawn.PendingWeapon = Weapon(Inv);
			if ( !Pawn.PendingWeapon.HasAmmo() )
			{
				ClientMessage( Pawn.PendingWeapon.ItemName$Pawn.PendingWeapon.MessageNoAmmo );
				Pawn.PendingWeapon = None;
				return;
			}
			Pawn.Weapon.PutDown(Pawn.PendingWeapon);
			return;
		}
		Count++;
		if ( Count > 1000 )
			return;
    }
}

// The player wants to select previous item
exec function PrevItem()
{
	local Inventory Inv;
	local Powerups LastItem;

    if ( (Level.Pauser!=None) || (Pawn == None) )
		return;
	if (Pawn.SelectedItem==None)
	{
		Pawn.SelectedItem = Pawn.Inventory.SelectNext();
		Return;
	}
	if (Pawn.SelectedItem.Inventory!=None)
		for( Inv=Pawn.SelectedItem.Inventory; Inv!=None; Inv=Inv.Inventory )
		{
			if (Inv==None) Break;
			if ( Inv.IsA('Powerups') && Powerups(Inv).bActivatable) LastItem=Powerups(Inv);
		}
	for( Inv=Pawn.Inventory; Inv!=Pawn.SelectedItem; Inv=Inv.Inventory )
	{
		if (Inv==None) Break;
		if ( Inv.IsA('Powerups') && Powerups(Inv).bActivatable) LastItem=Powerups(Inv);
	}
	if (LastItem!=None)
		Pawn.SelectedItem = LastItem;
}

// The player wants to active selected item
exec function ActivateItem()
{
	if( Level.Pauser!=None )
		return;
	if ( (Pawn != None) && (Pawn.SelectedItem!=None) )
		Pawn.SelectedItem.Activate();
}

exec function StopFire( optional float F )
{
	if (Pawn == None)
		return;
    Pawn.StopFire(F);
}

// The player wants to fire.
exec function Fire( optional float F )
{
	if ( Level.Pauser == PlayerReplicationInfo )
	{
		SetPause(false);
		return;
	}
	if( bDemoOwner || (Pawn == None) )
		return;
    Pawn.Fire(F);
}


// The player wants to alternate-fire.
// This is used for melee.  Will rename from AltFire later.
exec function AltFire( optional float F )
{
	if ( Level.Pauser == PlayerReplicationInfo )
	{
		SetPause(false);
		return;
	}
	if( bDemoOwner || (Pawn == None) )
		return;
	if( Pawn.Weapon!=None )
		Pawn.Weapon.AltFire(F);
}

exec function ThrowGrenade( optional float F )
{
	if( bDemoOwner || (Pawn == None) )
		return;
    if (Pawn.Weapon != None)
		Pawn.Weapon.ThrowGrenade(F);
}

// The player wants to use something in the level.
exec function Use()
{
	bUse = true;
	ServerUse();
}

// The player lets go of the Use button
exec function StopUse()
{
	bUse = false;
}

function ServerUse()
{
	local Actor A;

	if ( Level.Pauser == PlayerReplicationInfo )
	{
		SetPause(false);
		return;
	}

	if (Pawn==None)
		return;

	// Send the 'DoUse' event to each actor player is touching.
	ForEach Pawn.TouchingActors(class'Actor', A)
	{
		if (A.IsA('Pickup'))
		{
			if (Pickup(A).PickupUsedBy(Pawn))
				break;
		}
		else
			A.UsedBy(Pawn);
	}
}

exec function ListInventory()
{
	local Inventory Inv;

	if ( Pawn == None )
		return;
	Inv = Pawn.Inventory;
	while (Inv != None)
	{
		Log("ListInventory: "$Inv);
		Inv = Inv.Inventory;
	}
}

exec function ShowPCState()
{
	Log("State "$GetStateName());
	ServerShowPCState();
}

function ServerShowPCState()
{
	Log("State "$GetStateName());
}

exec function ShowWeaponState()
{
	if ( Pawn != None && Pawn.Weapon != None )
		Log("ShowWeaponState: Weapon "$Pawn.Weapon$" State "$Pawn.Weapon.GetStateName()$" ReloadCount "$Pawn.Weapon.ReloadCount$" AmmoAmount "$Pawn.Weapon.AmmoType.AmmoAmount);
	else
		Log("Pawn "$Pawn$" Pawn.Weapon "$Pawn.Weapon);
	ServerShowWeaponState();
}

function ServerShowWeaponState()
{
	if ( Pawn != None && Pawn.Weapon != None )
		Log("ShowWeaponState: Weapon "$Pawn.Weapon$" State "$Pawn.Weapon.GetStateName()$" ReloadCount "$Pawn.Weapon.ReloadCount$" AmmoAmount "$Pawn.Weapon.AmmoType.AmmoAmount);
}

//CHEAT Function
exec function Revive()
{
	if ((Pawn != None) && Pawn.bIncapacitated)
		Pawn.Revive();
}

exec function Suicide()
{
	if ( (Pawn != None) && (Level.TimeSeconds - Pawn.LastStartTime > 1) )
    Pawn.KilledBy( Pawn );
}

exec function Name( coerce string S )
{
	SetName(S);
}

simulated event UpdateName(string newName)
{
	ChangeName( newName );
}


exec function SetName( coerce string S)
{
	if ( Len(S) > 20 )
		S = left(S,20);
	ReplaceText(S, " ", "_");
	ReplaceText(S, "\"", "");

	ChangeName(S);
	UpdateURL("Name", S, true);
	SaveConfig();
}

function ChangeName( coerce string S )
{
    if ( Len(S) > 20 )
        S = left(S,20);
	ReplaceText(S, " ", "_");
    ReplaceText(S, "\"", "");
    Level.Game.ChangeName( self, S, true );
}

exec function SwitchTeam()
{
	if ( (PlayerReplicationInfo.Team == None) || (PlayerReplicationInfo.Team.TeamIndex == 1) )
		ChangeTeam(0, true);
	else
		ChangeTeam(1, true);
}

exec function ChangeTeam( int N, optional bool bSwitch )
{
	local TeamInfo OldTeam;

	OldTeam = PlayerReplicationInfo.Team;
	Level.Game.ChangeTeam(self, N, true, bSwitch);
	if ( Level.Game.bTeamGame && (PlayerReplicationInfo.Team != OldTeam) )
	{
		if ( Pawn != None )
			Pawn.Died( None, class'DamageType', Pawn.Location );
	}
	else
	{
		if (( N == 0 ) || (N == 1))
			ReceiveLocalizedMessage(class'GameMessage', 17 + N );
	}
}

exec function SwitchLevel( string URL )
{
	if( Level.NetMode==NM_Standalone || Level.netMode==NM_ListenServer )
		Level.ServerTravel( URL, false );
}

exec function ClearProgressMessages()
{
	local int i;

	for (i=0; i<ArrayCount(ProgressMessage); i++)
	{
		ProgressMessage[i] = "";
		ProgressColor[i] = class'Canvas'.Static.MakeColor(255,255,255);
	}
}

exec event SetProgressMessage( int Index, string S, color C )
{
	if ( Index < ArrayCount(ProgressMessage) )
	{
		ProgressMessage[Index] = S;
		ProgressColor[Index] = C;
	}
}

exec event SetProgressTime( float T )
{
	ProgressTimeOut = T + Level.TimeSeconds;
}

function Restart()
{
	Super.Restart();
	ServerTimeStamp = 0;
	ResetTimeMargin();
	EnterStartState();
	bBehindView = false;
	SetViewTarget(Pawn);
	ClientRestart(Pawn);
}

function EnterStartState()
{
	local name NewState;

	if (Machine != None && Machine.IsA('Turret'))
		NewState = 'PlayerManningTurret';
	else if ( Pawn.CurrentPhysicsVolume.bWaterVolume )
	{
		NewState = Pawn.WaterMovementState;
	}
	else
		NewState = Pawn.LandMovementState;

	if ( IsInState(NewState) )
		BeginState();
	else
		GotoState(NewState);
}

function ClientRestart(Pawn NewPawn)
{
	local bool bNewViewTarget;

	Pawn = NewPawn;
	if ( (Pawn != None) && Pawn.bTearOff )
	{
		Pawn.Controller = None;
		Pawn = None;
	}
	AcknowledgePossession(Pawn);
	if ( Pawn == None )
	{
		GotoState('WaitingForPawn');
		return;
	}
	Pawn.ClientRestart();
	bDuck = 0;
	bNewViewTarget = (ViewTarget != Pawn);
	SetViewTarget(Pawn);
	bBehindView = false;
	BehindView(bBehindView);
	CleanOutSavedMoves();
	EnterStartState();
}

exec function BehindView( Bool B )
{
	if ( (Level.NetMode == NM_Standalone) || Level.Game.bAllowBehindView || PlayerReplicationInfo.bOnlySpectator || PlayerReplicationInfo.bAdmin || IsA('Admin') )
	{
		if( Machine != None && Machine.IsA('Turret') )
			Machine.bHidden = !B;

	    bBehindView = B;
	    ClientSetBehindView(bBehindView);
    }
}

exec function ToggleBehindView()
{
	ServerToggleBehindview();
}

function ServerToggleBehindView()
{
	bBehindView = !bBehindView;
	ClientSetBehindView(bBehindView);
}

//=============================================================================
// functions.

// Just changed to pendingWeapon
function ChangedWeapon()
{
    if ( Pawn.Weapon != None )
    {
        Pawn.Weapon.SetHand(Handedness);
        LastPawnWeapon = Pawn.Weapon.Class;
    }
}

event TravelPostAccept()
{
	if ( Pawn.Health <= 0 )
		Pawn.Health = Pawn.Default.Health;
}

event PlayerTick( float DeltaTime )
{
	local vector FlashlightLocation;

	if ( bForcePrecache )
	{
		if ( Level.TimeSeconds > ForcePrecacheTime )
	{
		bForcePrecache = false;
		Level.FillPrecacheMaterialsArray();
		Level.FillPrecacheStaticMeshesArray();
	}
	}
	else if ( !bShortConnectTimeOut )
	{
		bShortConnectTimeOut = true;
		ServerShortTimeout();
	}

	if ( Pawn != AcknowledgedPawn )
	{
		if ( Role < ROLE_Authority )
		{
			// make sure old pawn controller is right
			if ( (AcknowledgedPawn != None) && (AcknowledgedPawn.Controller == self) )
				AcknowledgedPawn.Controller = None;
		}
		AcknowledgePossession(Pawn);
	}
	PlayerInput.PlayerInput(DeltaTime);
	if ( bUpdatePosition )
		ClientUpdatePosition();
	PlayerMove(DeltaTime);

	// Update the position of the flashlight]
	if( PlayerSpotLight != None )
	{
		FlashlightLocation = Pawn.Location;
		FlashlightLocation.Z += Pawn.EyeHeight;
		PlayerSpotLight.SetLocation( FlashlightLocation );
		PlayerSpotLight.SetRotation( Rotation );
		if( PlayerSpotLight.LightBrightness < 200 )
		{
			PlayerSpotLight.LightBrightness += DeltaTime * 350;
			if( PlayerSpotLight.LightBrightness > 200 )
				PlayerSpotLight.LightBrightness = 200;
		}
	}

	// Update frame effect blending
	UpdateBlending( DeltaTime );
}

function UpdateBlending( float DeltaTime )
{
	local float Lerp;
	local int	StageIndex;

	// Update Add Color
	if( CurrentAddStage != CBS_None )
	{
		StageIndex = CurrentAddStage - 1;
		AddStages[StageIndex].RemainingTime -= DeltaTime;
		AddStages[StageIndex].RemainingTime = FMax( AddStages[StageIndex].RemainingTime, 0 );

		switch( CurrentAddStage )
		{
		case CBS_In:
		case CBS_Out:
			Lerp = 1 - ( AddStages[StageIndex].RemainingTime / AddStages[StageIndex].TotalTime );
			ColorAdd.R = ( AddStages[StageIndex].GoalColor.R * Lerp ) + ( AddStages[StageIndex].BaseColor.R * ( 1 - Lerp ) );
			ColorAdd.G = ( AddStages[StageIndex].GoalColor.G * Lerp ) + ( AddStages[StageIndex].BaseColor.G * ( 1 - Lerp ) );
			ColorAdd.B = ( AddStages[StageIndex].GoalColor.B * Lerp ) + ( AddStages[StageIndex].BaseColor.B * ( 1 - Lerp ) );
			break;
		case CBS_Sustain:
			ColorAdd = AddStages[StageIndex].GoalColor;
			break;
		}

		if( AddStages[StageIndex].RemainingTime == 0 )
			CurrentAddStage = EColorBlendStage( ( CurrentAddStage + 1 ) % 4 );
	}

	// Update Mult Color
	if( CurrentMultStage != CBS_None )
	{
		StageIndex = CurrentMultStage - 1;
		MultStages[StageIndex].RemainingTime -= DeltaTime;
		MultStages[StageIndex].RemainingTime = FMax( MultStages[StageIndex].RemainingTime, 0 );

		switch( CurrentMultStage )
		{
		case CBS_In:
		case CBS_Out:
			Lerp = 1 - ( MultStages[StageIndex].RemainingTime / MultStages[StageIndex].TotalTime );
			ColorMultiply.R = ( MultStages[StageIndex].GoalColor.R * Lerp ) + ( MultStages[StageIndex].BaseColor.R * ( 1 - Lerp ) );
			ColorMultiply.G = ( MultStages[StageIndex].GoalColor.G * Lerp ) + ( MultStages[StageIndex].BaseColor.G * ( 1 - Lerp ) );
			ColorMultiply.B = ( MultStages[StageIndex].GoalColor.B * Lerp ) + ( MultStages[StageIndex].BaseColor.B * ( 1 - Lerp ) );
			break;
		case CBS_Sustain:
			ColorMultiply = MultStages[StageIndex].GoalColor;
			break;
		}

		if( MultStages[StageIndex].RemainingTime == 0 )
			CurrentMultStage = EColorBlendStage( ( CurrentMultStage + 1 ) % 4 );
	}

	// Update Blur
	if( CurrentBlurStage != CBS_None )
	{
		StageIndex = CurrentBlurStage - 1;
		BlurStages[StageIndex].RemainingTime -= DeltaTime;
		BlurStages[StageIndex].RemainingTime = FMax( BlurStages[StageIndex].RemainingTime, 0 );

		switch( CurrentBlurStage )
		{
		case CBS_In:
		case CBS_Out:
			Lerp = 1 - ( BlurStages[StageIndex].RemainingTime / BlurStages[StageIndex].TotalTime );
			Blur = ( BlurStages[StageIndex].GoalColor.R * Lerp ) + ( BlurStages[StageIndex].BaseColor.R * ( 1 - Lerp ) );
			break;
		case CBS_Sustain:
			Lerp = BlurStages[StageIndex].GoalColor.R;
			break;
		}

		if( BlurStages[StageIndex].RemainingTime == 0 )
			CurrentBlurStage = EColorBlendStage( ( CurrentBlurStage + 1 ) % 4 );
	}

	// Update BloomFilter
	if( CurrentBloomStage != CBS_None )
	{
		StageIndex = CurrentBloomStage - 1;
		BloomStages[StageIndex].RemainingTime -= DeltaTime;
		BloomStages[StageIndex].RemainingTime = FMax( BloomStages[StageIndex].RemainingTime, 0 );

		switch( CurrentBloomStage )
		{
		case CBS_In:
		case CBS_Out:
			Lerp = 1 - ( BloomStages[StageIndex].RemainingTime / BloomStages[StageIndex].TotalTime );
			Bloom = ( BloomStages[StageIndex].GoalColor.R * Lerp ) + ( BloomStages[StageIndex].BaseColor.R * ( 1 - Lerp ) );
			break;
		case CBS_Sustain:
			Bloom = BloomStages[StageIndex].GoalColor.R;
			break;
		}

		if( BloomStages[StageIndex].RemainingTime == 0 )
			CurrentBloomStage = EColorBlendStage( ( CurrentBloomStage + 1 ) % 4 );
	}
}

function PlayerMove(float DeltaTime);

//
/* AdjustAim()
Calls this version for player aiming help.
Aimerror not used in this version.
Only adjusts aiming at pawns
*/
function rotator AdjustAim(Ammunition FiredAmmunition, vector projStart, float aimerror, optional out int ShouldFire)
{
	//performs no auto-aiming, but instead adjusts aim to hit reticle
	local vector ViewDir, FireDir, AimSpot, HitLocation, HitNormal, EyeLoc, AimTarget, AdjustedTarget;
	local Actor HitActor, PickTargetActor;
	local float bestAim, bestDist, MaxAimRange;
	local name NearestBoneName;

	ViewDir = vector(Rotation);
	FireDir = ViewDir;
	EyeLoc = Pawn.Location + Pawn.EyePosition();
	AimSpot = EyeLoc + 10000 * FireDir; //ammo should return TraceLength (Max Range)
	// adjust for the shift to the right for the FPFireOffset
	if (Pawn != None && Pawn.Weapon != None)
		AimSpot.Y -= Pawn.Weapon.FPFireOffset.Y;

	HitActor = Trace(HitLocation, HitNormal, AimSpot, EyeLoc, true);
	if ( HitActor == None || !HitActor.CanBeAutoAimedBy(Pawn) )
	{
		if( AimingHelp != 0 || FiredAmmunition == None )
		{
			// Multiply the global aim adjustment by the per-ammo adjustment
			if (FiredAmmunition != None)
			{
				bestAim = 1.0 - ( AimingHelp * FiredAmmunition.AimAdjustment ) / 65025.0;//cg: why not opt.!? (255.0 * 255.0);
				if ( FOVAngle < DefaultFOV )
					bestAim = 1.0 - ( AimingHelp * FiredAmmunition.ZoomedAimAdjustment) / 65025.0;//cg : (255.0 * 255.0);
				MaxAimRange = FiredAmmunition.MaxAutoAimRange;
			}
			else if ( Pawn != None && Pawn.Weapon != None )
			{
				bestAim = MeleeAimingHelp;
				MaxAimRange = Pawn.Weapon.MeleeRange;
			}

			PickTargetActor = PickTarget(bestAim, bestDist, FireDir, EyeLoc, MaxAimRange);
			if(PickTargetActor != None)
			{
				if (PickTargetActor.CanBeAutoAimedBy(Pawn))
				{
					AimSpot = EyeLoc + bestDist * FireDir;
					NearestBoneName = PickTargetActor.FindNearestBone(AimSpot);
					AimTarget = PickTargetActor.GetBoneLocation(NearestBoneName);
					FireDir = AimTarget - projStart;
				}
				else
					PickTargetActor = None;
			}
		}
		else
			AimTarget = HitLocation;

		// if we still have no target
		if (HitActor == None)
			AimTarget = AimSpot;
		else if (PickTargetActor == None)
			AimTarget = HitLocation;
	}
	// direct hit - no need for autoaim - set HitLocation to the one that got cached from SelectTargetType
	else
		AimTarget = HitLocation;

	// If the target is behind or to the side of projStart, just fire straight out of the barrel
	// Fixes visual artifacts of bolt coming out of barrel at oblique angles
	// The threshold number was arrived at through emperical observation
	if( Normal(AimTarget - projStart) Dot ViewDir < 0.4 )
		AimTarget = projStart + ViewDir * 100;

	AdjustedTarget = AdjustAimError(projStart, AimTarget, aimerror);
	return Rotator(AdjustedTarget - projStart);
}

function vector AdjustAimError(vector projStart, vector target, float aimerror)
{
	local vector AdjustedTarget;
	local float TargetDist, rand, RealAimError;

	AdjustedTarget = target;
	RealAimError = AimError;
	if ( FOVAngle < DefaultFOV )
	{
		if (Pawn.Weapon != None)
			RealAimError = Pawn.Weapon.ZoomedAimError;
	}

	// we want a cone effect on the aimerror based on distance
	TargetDist = VSize(target - projStart);
	TargetDist *= 0.001;
	// get a random int between -1 and 1
	rand = 2 * (FRand() - 0.5f);
	AdjustedTarget.Z = target.Z + rand * RealAimError * TargetDist;
	// get it again so it's not in a diagonal pattern
	rand = 2 * (FRand() - 0.5f);
	AdjustedTarget.Y = target.Y + rand * RealAimError * TargetDist;

	return AdjustedTarget;
}

function bool NotifyLanded(vector HitNormal)
{
	return bUpdating;
}

//=============================================================================
// Player Control

// Player view.
// Compute the rendering viewpoint for the player.
//

function AdjustView(float DeltaTime )
{
	// client side effect.  Server just keeps track of the final FovAngle in ServerSetFOV
	if ( (Role == ROLE_Authority) && (Level.NetMode != NM_Standalone) )
		return;

	// teleporters affect your FOV, so adjust it back down
	if ( FOVAngle != DesiredFOV )
	{
		if ( FOVAngle > DesiredFOV )
			FOVAngle = FOVAngle - FMax(7, 0.19 * DeltaTime * (FOVAngle - DesiredFOV));
		else
			FOVAngle = FOVAngle - FMin(-7, 0.19 * DeltaTime * (FOVAngle - DesiredFOV));
		if ( Abs(FOVAngle - DesiredFOV) <= 10 )
			FOVAngle = DesiredFOV;

		if( Pawn != None && Pawn.Weapon != None )
		{
			Pawn.Weapon.SetWeapFOV(FOVAngle);
		}
	}

	// adjust FOV for weapon zooming
	if ( bZooming )
	{
		ZoomLevel += DeltaTime * 1.0;
		if (ZoomLevel > 0.19)
			ZoomLevel = 0.19;
		DesiredFOV = FClamp(90.0 - (ZoomLevel * 88.0), 1, 170);
	}
}

function CalcBehindView(out vector CameraLocation, out rotator CameraRotation, float Dist)
{
	local vector View,HitLocation,HitNormal;
    local float ViewDist,RealDist;

	CameraRotation = Rotation;
	if ( bBlockCloseCamera )
		CameraLocation.Z += 12;

	View = vect(1,0,0) >> CameraRotation;

    // add view radius offset to camera location and move viewpoint up from origin (amb)
    RealDist = Dist;

    if( Trace( HitLocation, HitNormal, CameraLocation - Dist * vector(CameraRotation), CameraLocation,false,vect(10,10,10) ) != None )
		ViewDist = FMin( (CameraLocation - HitLocation) Dot View, Dist );
	else
		ViewDist = Dist;

    if ( !bBlockCloseCamera || !bValidBehindCamera || (ViewDist > 10 + FMax(ViewTarget.CollisionRadius, ViewTarget.CollisionHeight)) )
	{
		//Log("Update Cam ");
		bValidBehindCamera = true;
		OldCameraLoc = CameraLocation - ViewDist * View;
		OldCameraRot = CameraRotation;
	}
	else
	{
		//Log("Dont Update Cam "$bBlockCloseCamera@bValidBehindCamera@ViewDist);
		SetRotation(OldCameraRot);
	}

    CameraLocation = OldCameraLoc;
    CameraRotation = OldCameraRot;
}

function CalcFirstPersonView( out vector CameraLocation, out rotator CameraRotation )
{
	// First-person view.
	CameraRotation = Rotation;
	CameraLocation = CameraLocation + Pawn.EyePosition();	// ShakeOffset moved into EyePosition()
}

event AddCameraEffect(CameraEffect NewEffect,optional bool RemoveExisting)
{
	if(RemoveExisting)
		RemoveCameraEffect(NewEffect);

	CameraEffects.Length = CameraEffects.Length + 1;
	CameraEffects[CameraEffects.Length - 1] = NewEffect;
}

event RemoveCameraEffect(CameraEffect ExEffect)
{
	local int	EffectIndex;

	for(EffectIndex = 0;EffectIndex < CameraEffects.Length;EffectIndex++)
		if(CameraEffects[EffectIndex] == ExEffect)
		{
			CameraEffects.Remove(EffectIndex,1);
			return;
		}
}

exec function CCE(class<CameraEffect> EffectClass)
{
	AddCameraEffect(new EffectClass);
}

exec function CreateCameraEffect(class<CameraEffect> EffectClass)
{
	AddCameraEffect(new EffectClass);
}

function rotator GetViewRotation()
{
	if ( bBehindView && (Pawn != None) )
		return Pawn.Rotation;
	return Rotation;
}

event PlayerCalcView(out actor ViewActor, out vector CameraLocation, out rotator CameraRotation )
{
	local Pawn PTarget;

	if ( (ViewTarget == None) || ViewTarget.bDeleteMe )
	{
        if ( bViewBot && (CheatManager != None) )
			CheatManager.ViewBot();
        else if ( (Pawn != None) && !Pawn.bDeleteMe )
			SetViewTarget(Pawn);
        else if ( RealViewTarget != None )
            SetViewTarget(RealViewTarget);
		else
			SetViewTarget(self);
	}

	ViewActor = ViewTarget;
	CameraLocation = ViewTarget.Location;

	if ( ViewTarget == Pawn )
	{
		if( bBehindView ) //up and behind
			CalcBehindView(CameraLocation, CameraRotation, CameraDist * Pawn.Default.CollisionRadius);
		else
			CalcFirstPersonView( CameraLocation, CameraRotation );
		return;
	}
	else if ( ViewTarget == self )
	{
		if ( bCameraPositionLocked )
			CameraRotation = CheatManager.LockedRotation;
		else
			CameraRotation = Rotation;
		return;
	}
    else if ( ViewTarget.IsA('Projectile') && !bBehindView )
    {
        CameraLocation += (ViewTarget.CollisionHeight) * vect(0,0,1);
        CameraRotation = Rotation;
	    return;
    }

	CameraRotation = ViewTarget.Rotation;
	PTarget = Pawn(ViewTarget);
	if ( PTarget != None )
	{
		if ( Level.NetMode == NM_Client )
		{
			PTarget.SetViewRotation(TargetViewRotation);
            CameraRotation = BlendedTargetViewRotation;

			PTarget.EyeHeight = TargetEyeHeight;
			if ( PTarget.Weapon != None )
				PTarget.Weapon.PlayerViewOffset = TargetWeaponViewOffset;
		}
		else if ( PTarget.IsPlayerPawn() )
			CameraRotation = PTarget.GetViewRotation();
		if ( !bBehindView )
			CameraLocation += PTarget.EyePosition();
	}
	if ( bBehindView )
	{
		CameraLocation = CameraLocation + (ViewTarget.Default.CollisionHeight - ViewTarget.CollisionHeight) * vect(0,0,1);
		CalcBehindView(CameraLocation, CameraRotation, CameraDist * ViewTarget.Default.CollisionRadius);
	}
}

function int BlendRot(float DeltaTime, int BlendC, int NewC)
{
	if ( Abs(BlendC - NewC) > 32767 )
	{
		if ( BlendC > NewC )
			NewC += 65536;
		else
			BlendC += 65536;
	}
	if ( Abs(BlendC - NewC) > 4096 )
		BlendC = NewC;
	else
		BlendC = BlendC + (NewC - BlendC) * FMin(1,24 * DeltaTime);

	return (BlendC & 65535);
}

native event function UpdateRotation(float DeltaTime, float maxPitch);

function ClearDoubleClick()
{
	if (PlayerInput != None)
		PlayerInput.DoubleClickTimer = 0.0;
}

// Player movement.
// Player Standing, walking, running, falling.
state PlayerWalking
{
ignores SeePlayer, HearNoise, Bump;

	function bool NotifyPhysicsVolumeChange( PhysicsVolume NewVolume )
	{
		if ( NewVolume.bWaterVolume )
			GotoState(Pawn.WaterMovementState);
		return false;
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local vector OldAccel;
		local bool OldCrouch;

		if ( Pawn == None )
			return;
		OldAccel = Pawn.Acceleration;
		if ( Pawn.Acceleration != NewAccel )
			Pawn.Acceleration = NewAccel;
		if ( bPressedJump )
			Pawn.DoJump(bUpdating);
		if ( Pawn.Physics != PHYS_Falling )
		{
			OldCrouch = Pawn.bWantsToCrouch;
			if (bDuck == 0)
				Pawn.ShouldCrouch(false);
			else if ( Pawn.bCanCrouch )
				Pawn.ShouldCrouch(true);
		}
	}

	function PlayerMove( float DeltaTime )
	{
		local vector X,Y,Z, NewAccel;
		local eDoubleClickDir DoubleClickMove;
		local rotator OldRotation, ViewRotation;
		local bool	bSaveJump;

		ViewRotation = Rotation;
		ViewRotation.Pitch = 0;  ViewRotation.Roll = 0;
		GetAxes(ViewRotation, X,Y,Z);

		// Update acceleration.
		NewAccel = aForward*X + aStrafe*Y;
		NewAccel.Z = 0;
		if ( VSizeSq(NewAccel) < 1.0 )
			NewAccel = vect(0,0,0);
		DoubleClickMove = PlayerInput.CheckForDoubleClickMove(DeltaTime);

		GroundPitch = 0;
		ViewRotation = Rotation;

		if ( (Pawn != None) && (Pawn.Physics == PHYS_Walking) )
		{
			// tell pawn about any direction changes to give it a chance to play appropriate animation
			//if walking, look up/down stairs - unless player is rotating view
			if ( (bLook == 0)
                && (((Pawn.Acceleration != Vect(0,0,0)) && bSnapToLevel) || !bKeyboardLook) )
			{
				if ( bLookUpStairs || bSnapToLevel )
				{
					GroundPitch = FindStairRotation(deltaTime);
					ViewRotation.Pitch = GroundPitch;
				}
				else if ( bCenterView )
				{
					ViewRotation.Pitch = ViewRotation.Pitch & 65535;
					if (ViewRotation.Pitch > 32768)
						ViewRotation.Pitch -= 65536;
					ViewRotation.Pitch = ViewRotation.Pitch * (1 - 12 * FMin(0.0833, deltaTime));
                    if ( Abs(ViewRotation.Pitch) < 200 )
						ViewRotation.Pitch = 0;
				}
			}
		}
		else
		{
			if ( !bKeyboardLook && (bLook == 0) && bCenterView )
			{
				ViewRotation.Pitch = ViewRotation.Pitch & 65535;
				if (ViewRotation.Pitch > 32768)
					ViewRotation.Pitch -= 65536;
				ViewRotation.Pitch = ViewRotation.Pitch * (1 - 12 * FMin(0.0833, deltaTime));
                if ( Abs(ViewRotation.Pitch) < 200 )
					ViewRotation.Pitch = 0;
			}
		}

		if ( Pawn != None )
			Pawn.CheckBob(DeltaTime, Y);

		// Update rotation.
		SetRotation(ViewRotation);
		OldRotation = Rotation;
		UpdateRotation(DeltaTime, 1);
		bDoubleJump = false;

		if ( bPressedJump && (Pawn != None) && Pawn.CannotJumpNow() )
		{
			bSaveJump = true;
			bPressedJump = false;
		}
		else
			bSaveJump = false;

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
		else
			ProcessMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
		bPressedJump = bSaveJump;
	}

	function BeginState()
	{
       	DoubleClickDir = DCLICK_None;
       	bPressedJump = false;
       	GroundPitch = 0;
		if ( Pawn != None )
		{
		if ( Pawn.Mesh == None )
			Pawn.SetMesh();
		Pawn.ShouldCrouch(false);
		if (Pawn.Physics != PHYS_Falling && Pawn.Physics != PHYS_Karma) // FIXME HACK!!!
			Pawn.SetPhysics(PHYS_Walking);
		}
	}

	function EndState()
	{
		GroundPitch = 0;
		if ( Pawn != None && bDuck==0 )
		{
			Pawn.ShouldCrouch(false);
		}
	}
}

// player is climbing ladder
state PlayerClimbing
{
ignores SeePlayer, HearNoise, Bump;

	function bool NotifyPhysicsVolumeChange( PhysicsVolume NewVolume )
	{
		if ( NewVolume.bWaterVolume )
			GotoState(Pawn.WaterMovementState);
		else
			GotoState(Pawn.LandMovementState);
		return false;
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local vector OldAccel;

		OldAccel = Pawn.Acceleration;
		Pawn.Acceleration = NewAccel;

		if ( bPressedJump )
		{
			Pawn.DoJump(bUpdating);
			if ( Pawn.Physics == PHYS_Falling )
				GotoState('PlayerWalking');
		}
	}

	function PlayerMove( float DeltaTime )
	{
		local vector X,Y,Z, NewAccel;
		local eDoubleClickDir DoubleClickMove;
		local rotator OldRotation, ViewRotation;
		local bool	bSaveJump;

		GetAxes(Rotation,X,Y,Z);

		NewAccel = aForward*X + aStrafe*Y;
		if ( VSizeSq(NewAccel) < 1.0 )
			NewAccel = vect(0,0,0);

		ViewRotation = Rotation;

		// Update rotation.
		SetRotation(ViewRotation);
		OldRotation = Rotation;
		UpdateRotation(DeltaTime, 1);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
		else
			ProcessMove(DeltaTime, NewAccel, DoubleClickMove, OldRotation - Rotation);
		bPressedJump = bSaveJump;
	}

	function BeginState()
	{
		Pawn.ShouldCrouch(false);
		bPressedJump = false;
	}

	function EndState()
	{
		if ( Pawn != None )
			Pawn.ShouldCrouch(false);
	}
}

// A Scav Droid is on your head
// Machine in this case is the ScavDroid Pawn
state PlayerPossessed extends PlayerWalking
{
	ignores ToggleHeadlamp;

	function ThrowScav()
	{
		local vector NewMachineLoc;
		local rotator Rot;

		if( Machine != None )
		{
			Machine.SetBase(None);
			NewMachineLoc = Pawn.Location;
			CalcFirstPersonView( NewMachineLoc, Rot );
			NewMachineLoc.Z += 10;
			Machine.SetLocation(NewMachineLoc);
		}

		Super.ThrowScav();

		GotoState('PlayerWalking');
	}

	// no weapon switching in this mode
	exec function PrevWeapon() {}
	exec function NextWeapon() {}
	exec function PipedSwitchWeapon(byte F) {}

	// NathanM: This is duped from the main state function
	exec function SwitchWeapon (byte F )
	{
		if( Machine != None && F != 0 )
			return;
		else
			Global.SwitchWeapon( F );
	}

	function BeginState()
	{
		Pawn.CurrentIdleState = AS_Possessed;
		Pawn.ChangeAnimation();
		Pawn.CreateInventory( "CTInventory.AttachedScavDroid" );
		SwitchWeapon(0);
		//Pawn.ChangedWeapon();
		if (Pawn.PendingWeapon != None)
			myHUD.bScavangerHead=true;
		Target = None;
		TargetType = TGT_Default;
	}

	function EndState()
	{
		if( Pawn.Health > 0 )
		{
			Pawn.SetPhysics(PHYS_Falling);
			Pawn.CurrentIdleState = AS_Alert;
			Pawn.ChangeAnimation();
		}
		else
			Pawn.TossWeapon(Pawn.Weapon,vect(0,0,0));

		myHUD.bScavangerHead=false;
	}

	function WeaponFired()
	{
	}
}

state PlayerManningTurret
{
	ignores SeePlayer, HearNoise, Bump, SwitchWeapon, NextWeapon, PrevWeapon, ForceReload;

	function BeginState()
	{
		local Turret MannedTurret;

		MannedTurret = Turret(Machine);

		if(MannedTurret == None)
		{
			Log("Machine is broken");
			return;
		}
		MannedTurret.SetAttachmentCollision(false);
		Pawn.Velocity = vect(0,0,0);
		Pawn.Acceleration = vect(0,0,0);
		SetRotation(MannedTurret.Rotation);
	}

	function EndState()
	{
		local Turret MannedTurret;

		MannedTurret = Turret(Machine);
		MannedTurret.SetAttachmentCollision(true);

		if(MannedTurret == None)
		{
			Log("Machine is broken");
			return;
		}
		SetRotation(MannedTurret.Rotation);
		Machine = None;
	}

	function PlayerMove( float DeltaTime )
	{
		local Turret MannedTurret;
		local vector Loc;

		MannedTurret = Turret(Machine);

		if(MannedTurret == None)
		{
			Log("Machine is broken");
			return;
		}
		//if the pawn has been incapacitated on the turret,
		if (Pawn.bIncapacitatedOnTurret)
		{
			MannedTurret.UsedBy(Pawn);
			/*
			if (MannedTurret.SitToUse)
				Pawn.PlayAnim('TurretSitImmobile');
			else
				Pawn.PlayAnim('TurretStandImmobile');
				*/
			Pawn.bIncapacitatedOnTurret=false;
			Pawn.DieAgain();
			return;
		}

		MannedTurret.ChangeAngle(DeltaTime * aTurn * 32, DeltaTime * aLookUp * 32, DeltaTime);
		MannedTurret.ChangeLocation(int(DeltaTime * aStrafe));

		SetRotation( MannedTurret.GetCameraRotation() );
		Loc = MannedTurret.Location;
		Loc.Z += MannedTurret.TurretHeight;
		Pawn.SetLocation( Loc );
		if (Pawn.Weapon != None)
			Pawn.Weapon.PlayTurnAnim(aTurn, aLookUp);
		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);
	}

	exec function Use()
	{
		switch(TargetType)
		{
			case TGT_Marker:
			case TGT_MarkerCancel:
			case TGT_Enemy:
			case TGT_EngagedEnemy:
				Global.Use();
				break;
			default:
				Machine.UsedBy(Pawn);
		}
	}
}

state PlayerFlying
{
ignores SeePlayer, HearNoise, Bump;

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;

		GetAxes(Rotation,X,Y,Z);

		Pawn.Acceleration = aForward*X + aStrafe*Y;
		if ( VSize(Pawn.Acceleration) < 1.0 )
			Pawn.Acceleration = vect(0,0,0);
		if ( bCheatFlying && (Pawn.Acceleration == vect(0,0,0)) )
			Pawn.Velocity = vect(0,0,0);
		// Update rotation.
		UpdateRotation(DeltaTime, 2);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
	}

	function BeginState()
	{
		Pawn.SetPhysics(PHYS_Flying);
	}
}

//REVISIT: NathanM: We may want to delete this state
state PlayerRocketing
{
ignores SeePlayer, HearNoise, Bump;

	/* ServerMove()
	- replicated function sent by client to server - contains client movement and firing info
	Passes acceleration in components so it doesn't get rounded.
	IGNORE VANILLA SERVER MOVES
	*/
	function ServerMove
	(
		float TimeStamp,
		vector InAccel,
		vector ClientLoc,
		bool NewbRun,
		bool NewbDuck,
		bool NewbJumpStatus,
		bool NewbDoubleJump,
		eDoubleClickDir DoubleClickMove,
		byte ClientRoll,
		int View,
		optional byte OldTimeDelta,
		optional int OldAccel
	)
	{
		if ( CurrentTimeStamp < TimeStamp )
	       Pawn.AutonomousPhysics(TimeStamp - CurrentTimeStamp);
		CurrentTimeStamp = TimeStamp;
		ServerTimeStamp = Level.TimeSeconds;
	}

	function RocketServerMove
	(
		float TimeStamp,
		vector InAccel,
		vector ClientLoc,
		byte ClientRoll,
		int View
	)
	{
		//if ( InAccel Dot Pawn.Velocity < 0 )
		//	InAccel = 0.1 * Pawn.AccelRate * Normal(Pawn.Velocity);
		Global.ServerMove(TimeStamp,InAccel,ClientLoc,false,false,false,false, DCLICK_NONE,ClientRoll,View);
	}

    function PlayerMove(float DeltaTime)
    {
		//Pawn.UpdateRocketAcceleration(DeltaTime,aTurn,aLookUp);
		SetRotation(Pawn.Rotation);
		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
	}

    function BeginState()
    {
        Pawn.SetPhysics(PHYS_Flying);
    }
}

function bool IsSpectating()
{
	return false;
}

state BaseSpectating
{
	function bool IsSpectating()
	{
		return true;
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		Acceleration = NewAccel;
        MoveSmooth(SpectateSpeed * Normal(Acceleration) * DeltaTime);
	}

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;

		if ( (Pawn(ViewTarget) != None) && (Level.NetMode == NM_Client) )
		{
			if ( Pawn(ViewTarget).bSimulateGravity )
				TargetViewRotation.Roll = 0;
			BlendedTargetViewRotation.Pitch = BlendRot(DeltaTime, BlendedTargetViewRotation.Pitch, TargetViewRotation.Pitch & 65535);
			BlendedTargetViewRotation.Yaw = BlendRot(DeltaTime, BlendedTargetViewRotation.Yaw, TargetViewRotation.Yaw & 65535);
			BlendedTargetViewRotation.Roll = BlendRot(DeltaTime, BlendedTargetViewRotation.Roll, TargetViewRotation.Roll & 65535);
		}
		GetAxes(Rotation,X,Y,Z);

		Acceleration = 0.02 * (aForward*X + aStrafe*Y + aUp*vect(0,0,1));

		UpdateRotation(DeltaTime, 1);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, Acceleration, DCLICK_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, Acceleration, DCLICK_None, rot(0,0,0));
	}
}

state Scripting
{
	// FIXME - IF HIT FIRE, AND NOT bInterpolating, Leave script
	exec function Fire( optional float F )
	{
	}

	exec function AltFire( optional float F )
	{
		Fire(F);
	}

	exec function ThrowGrenade(optional float F)
	{
		Fire(F);
	}
}

function ServerViewNextPlayer()
{
    local Controller C;
    local Pawn Pick;
    local bool bFound, bRealSpec;

    bRealSpec = PlayerReplicationInfo.bOnlySpectator;
    PlayerReplicationInfo.bOnlySpectator = true;

    // view next player
    for ( C=Level.ControllerList; C!=None; C=C.NextController )
    {
        if ( (C.Pawn != None) && Level.Game.CanSpectate(self,true,C.Pawn) )
        {
            if ( Pick == None )
                Pick = C.Pawn;
            if ( bFound )
            {
                Pick = C.Pawn;
                break;
            }
            else
                bFound = ( ViewTarget == C.Pawn );
        }
    }
    SetViewTarget(Pick);
    ClientSetViewTarget(Pick);
    if ( ViewTarget == self )
        bBehindView = false;
    else
        bBehindView = true; //bChaseCam;
	ClientSetBehindView(bBehindView);
    PlayerReplicationInfo.bOnlySpectator = bRealSpec;
}

function ServerViewSelf()
{
	bBehindView = false;
    SetViewTarget(self);
    ClientSetViewTarget(self);
	ClientMessage(OwnCamera, 'Event');
}

function LoadPlayers()
{
	local int i;

	if ( GameReplicationInfo == None )
		return;

	for ( i=0; i<GameReplicationInfo.PRIArray.Length; i++ )
		GameReplicationInfo.PRIArray[i].UpdatePrecacheMaterials();
}

function ServerSpectate()
{
	GotoState('Spectating');
	bBehindView = true;
	ServerViewNextPlayer();
}

//active player wants to become a spectator
function BecomeSpectator()
{
	if (Role < ROLE_Authority)
		return;

	if ( !Level.Game.BecomeSpectator(self) )
		return;

	if ( Pawn != None )
		Pawn.Died(self, class'DamageType', Pawn.Location);

	if ( PlayerReplicationInfo.Team != None )
		PlayerReplicationInfo.Team.RemoveFromTeam(self);
	PlayerReplicationInfo.Team = None;
	PlayerReplicationInfo.Score = 0;
	PlayerReplicationInfo.Deaths = 0;
	PlayerReplicationInfo.GoalsScored = 0;
	PlayerReplicationInfo.Kills = 0;
	ServerSpectate();
	BroadcastLocalizedMessage(Level.Game.GameMessageClass, 14, PlayerReplicationInfo);

	ClientBecameSpectator();
}

function ClientBecameSpectator()
{
	UpdateURL("SpectatorOnly", "1", true);
}

//spectating player wants to become active and join the game
function BecomeActivePlayer()
{
	if (Role < ROLE_Authority)
		return;

	if ( !Level.Game.AllowBecomeActivePlayer(self) )
		return;

	bBehindView = false;
	FixFOV();
	ServerViewSelf();
	PlayerReplicationInfo.bOnlySpectator = false;
	Level.Game.NumSpectators--;
	Level.Game.NumPlayers++;
	PlayerReplicationInfo.Reset();
	BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, PlayerReplicationInfo);
	if (Level.Game.bTeamGame)
		Level.Game.ChangeTeam(self, Level.Game.PickTeam(int(GetURLOption("Team")), None), false);
	if (!Level.Game.bDelayedStart)
    {
		// start match, or let player enter, immediately
		Level.Game.bRestartLevel = false;  // let player spawn once in levels that must be restarted after every death
		if (Level.Game.bWaitingToStartMatch)
			Level.Game.StartMatch();
		else
			Level.Game.RestartPlayer(PlayerController(Owner));
		Level.Game.bRestartLevel = Level.Game.Default.bRestartLevel;
    }
    else
        GotoState('PlayerWaiting');

    ClientBecameActivePlayer();
}

function ClientBecameActivePlayer()
{
	UpdateURL("SpectatorOnly","",true);
}


// called by menuselectprofile when a profile is selected.
function ProfileCallback()
{
	GotoState('PlayerWaiting');
}

//replicated to server
function ServerChooseTeam()
{
	PlayerReplicationInfo.bIsSpectator = false;
	GotoState('ChooseTeam');
}

state Spectating extends BaseSpectating
{
	ignores SwitchWeapon, RestartLevel, ClientRestart, Suicide,
	 ThrowWeapon, NotifyPhysicsVolumeChange, NotifyHeadVolumeChange;

	exec function AltFire(optional float F);

	exec function Fire( optional float F )
	{
    	if ( bFrozen )
		{
			if ( (TimerRate <= 0.0) || (TimerRate > 1.0) )
				bFrozen = false;
			return;
		}

		ServerViewNextPlayer();
	}

	// Return to spectator's own camera. - This is usually mapped to the second fire button
	exec function ThrowGrenade( optional float F )
	{
		bBehindView = false;
		ServerViewSelf();
	}

    function Timer()
    {
    	bFrozen = false;
    }

	function BeginState()
	{
		if ( Pawn != None )
		{
			SetLocation(Pawn.Location);
			UnPossess();
		}
		bCollideWorld = true;
		CameraDist = Default.CameraDist;
	}

	function EndState()
	{
		PlayerReplicationInfo.bIsSpectator = false;
		bCollideWorld = false;
	}
}

state ChooseTeam extends BaseSpectating
{
	ignores SeePlayer, HearNoise, NotifyBump, TakeDamage, PhysicsVolumeChange, NextWeapon, PrevWeapon, SwitchToBestWeapon;

	function ServerRestartPlayer();
	exec function Jump( optional float F );
	exec function Suicide();
	exec function AltFire(optional float F);
	exec function ThrowGrenade(optional float F);

	function bool CanRestartPlayer()
	{
		return false;
	}

	function ChangeTeam( int N, optional bool bSwitch )
	{
		// 255 implies autoselect
		if (N == 255)
		{
			N = Level.Game.PickTeam( N,None );
		}

		if (N < 0)
		{
			if (Level.Game.AllowBecomeSpectator( self ))
				BecomeSpectator();
			else
				ClientOpenMenu( GetTeamMenuClass(), False, "Full="$N );
		}
		// check to see if it's ok to join team first.
		else if (Level.Game.PickTeam( N,None ) == N)
		{
//			Log("Team selected.  Waiting to start");
			Level.Game.ChangeTeam(self, N, true);

         // Find a start spot.
			StartSpot = Level.Game.FindPlayerStart( self, byte(N) );

			GotoState('PlayerWaiting');

			Fire();
		}
		else
		{
			ClientOpenMenu( GetTeamMenuClass(), False, "Full="$N );
		}
	}

	exec function Fire(optional float F)
	{
		//ClientOpenMenu( GetTeamMenuClass() );
	}


	function EndState()
	{
		SetTimer(0.0, false);
	}

	function Timer()
	{
		// if it takes a while to connect this might be too fast.
		if (Player != None && Player.Console != None)
		{
			ClientOpenMenu( GetTeamMenuClass() );
			SetTimer(0.0, false);
		}
	}

	function BeginState()
	{
		// O_o  implied if spectating, then we should put up the menu.
		PlayerReplicationInfo.Team = None;

		SetTimer(0.5, true);
	}
}


state ChooseProfile extends BaseSpectating
{
	ignores SeePlayer, HearNoise, NotifyBump, TakeDamage, PhysicsVolumeChange, NextWeapon, PrevWeapon, SwitchToBestWeapon;

	function ServerRestartPlayer();
	exec function Jump( optional float F );
	exec function Suicide();
	exec function AltFire(optional float F);
	exec function ThrowGrenade(optional float F);

	function bool CanRestartPlayer()
	{
		return false;
	}

	function ProfileCallback()
	{
		if (GameReplicationInfo.bTeamGame)
			GotoState('ChooseTeam');
		else
			GotoState('PlayerWaiting');
	}

	exec function Fire(optional float F)
	{
		//ClientOpenMenu( GetTeamMenuClass() );
	}


	function EndState()
	{
		SetTimer(0.0, false);
	}

	function Timer()
	{
		// if it takes a while to connect this might be too fast.
		if (Player != None && Player.Console != None)
		{
			ClientOpenMenu( GetProfileMenuClass() );
			SetTimer(0.0, false);
		}
	}

	function BeginState()
	{
		SetTimer(0.5, true);
	}
}


auto state PlayerWaiting extends BaseSpectating
{
ignores SeePlayer, HearNoise, NotifyBump, TakeDamage, PhysicsVolumeChange, NextWeapon, PrevWeapon, SwitchToBestWeapon;

	exec function Jump( optional float F );
	exec function Suicide();

	function ChangeTeam( int N, optional bool bSwitch )
	{
        Level.Game.ChangeTeam(self, N, true, bSwitch);
	}

    function ServerRestartPlayer()
	{
		if ( Level.TimeSeconds < WaitDelay )
			return;
		if ( Level.NetMode == NM_Client )
			return;
		if ( Level.Game.bWaitingToStartMatch )
			PlayerReplicationInfo.bReadyToPlay = true;
		else
			Level.Game.RestartPlayer(self);
	}

	exec function Fire(optional float F)
	{
        LoadPlayers();
		ServerReStartPlayer();
	}

	exec function AltFire(optional float F)
	{
        Fire(F);
	}

	exec function ThrowGrenade(optional float F)
	{
		Fire(F);
	}

	function EndState()
	{
		if ( Pawn != None )
			Pawn.SetMesh();
		if ( PlayerReplicationInfo != None )
			PlayerReplicationInfo.SetWaitingPlayer(false);
		bCollideWorld = false;
	}

	function BeginState()
	{
		CameraDist = Default.CameraDist;
		if ( PlayerReplicationInfo != None )
			PlayerReplicationInfo.SetWaitingPlayer(true);
		bCollideWorld = true;
	}
}

state WaitingForPawn extends BaseSpectating
{
ignores SeePlayer, HearNoise, KilledBy, SwitchWeapon;

	exec function AltFire( optional float F );
	exec function ThrowGrenade(optional float F);

	exec function Fire( optional float F )
	{
		AskForPawn();
	}

	function LongClientAdjustPosition
	(
		float TimeStamp,
		name newState,
		EPhysics newPhysics,
		float NewLocX,
		float NewLocY,
		float NewLocZ,
		float NewVelX,
		float NewVelY,
		float NewVelZ,
		Actor NewBase,
		float NewFloorX,
		float NewFloorY,
		float NewFloorZ
	)
	{
		if ( newState == 'GameEnded' )
			GotoState(newState);
	}

	function PlayerTick(float DeltaTime)
	{
		Global.PlayerTick(DeltaTime);

		if ( Pawn != None )
		{
			Pawn.Controller = self;
			Pawn.bUpdateEyeHeight = true;
			ClientRestart(Pawn);
		}
		else if ( (TimerRate <= 0.0) || (TimerRate > 1.0) )
		{
			SetTimer(0.2,true);
			AskForPawn();
		}
	}

	function Timer()
	{
		AskForPawn();
	}

	function BeginState()
	{
		SetTimer(0.2, true);
        AskForPawn();
	}

	function EndState()
	{
		bBehindView = false;
		SetTimer(0.0, false);
	}
}

state GameEnded
{
ignores SeePlayer, HearNoise, KilledBy, NotifyBump, HitWall, NotifyHeadVolumeChange, NotifyPhysicsVolumeChange, Falling, TakeDamage, Suicide;

	function ServerReStartPlayer();
	exec function ThrowWeapon();

	function bool IsSpectating()
	{
		return true;
	}

	function ServerReStartGame()
	{
		Level.Game.RestartGame();
	}

	exec function Fire( optional float F )
	{
		if ( Role < ROLE_Authority)
			return;
		if ( !bFrozen )
			ServerReStartGame();
		else if ( TimerRate <= 0 )
			SetTimer(1.5, false);
	}

	exec function AltFire( optional float F )
	{
		Fire(F);
	}

	exec function ThrowGrenade(optional float F)
	{
		Fire(F);
	}

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;
		local Rotator ViewRotation;

		GetAxes(Rotation,X,Y,Z);
		// Update view rotation.

		if ( !bFixedCamera )
		{
			ViewRotation = Rotation;
			ViewRotation.Yaw += 32.0 * DeltaTime * aTurn;
			ViewRotation.Pitch += 32.0 * DeltaTime * aLookUp;
			ViewRotation.Pitch = ViewRotation.Pitch & 65535;
			If ((ViewRotation.Pitch > 18000) && (ViewRotation.Pitch < 49152))
			{
				If (aLookUp > 0)
					ViewRotation.Pitch = 18000;
				else
					ViewRotation.Pitch = 49152;
			}
			SetRotation(ViewRotation);
		}
		else if ( ViewTarget != None )
			SetRotation(ViewTarget.Rotation);

		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);

		if ( Role < ROLE_Authority ) // then save this move and replicate it
			ReplicateMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
		else
			ProcessMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
		bPressedJump = false;
	}

	function ServerMove
	(
		float TimeStamp,
		vector InAccel,
		vector ClientLoc,
		bool NewbRun,
		bool NewbDuck,
		bool NewbJumpStatus,
        bool NewbDoubleJump,
		eDoubleClickDir DoubleClickMove,
		byte ClientRoll,
		int View,
		optional byte OldTimeDelta,
		optional int OldAccel
	)
	{
        Global.ServerMove(TimeStamp, InAccel, ClientLoc, NewbRun, NewbDuck, NewbJumpStatus,NewbDoubleJump,
							DoubleClickMove, ClientRoll, (32767 & (Rotation.Pitch/2)) * 32768 + (32767 & (Rotation.Yaw/2)) );

	}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot, ViewRotation;
		local int tries, besttry;
		local float bestdist, newdist;
		local int startYaw;
		local actor ViewActor;

		ViewRotation = Rotation;
		ViewRotation.Pitch = 56000;
		tries = 0;
		besttry = 0;
		bestdist = 0.0;
		startYaw = ViewRotation.Yaw;

		for (tries=0; tries<16; tries++)
		{
			cameraLoc = ViewTarget.Location;
			SetRotation(ViewRotation);
			PlayerCalcView(ViewActor, cameraLoc, cameraRot);
			newdist = VSize(cameraLoc - ViewTarget.Location);
			if (newdist > bestdist)
			{
				bestdist = newdist;
				besttry = tries;
			}
			ViewRotation.Yaw += 4096;
		}

		ViewRotation.Yaw = startYaw + besttry * 4096;
		SetRotation(ViewRotation);
	}

	function Timer()
	{
		bFrozen = false;
	}

	function LongClientAdjustPosition
	(
		float TimeStamp,
		name newState,
		EPhysics newPhysics,
		float NewLocX,
		float NewLocY,
		float NewLocZ,
		float NewVelX,
		float NewVelY,
		float NewVelZ,
		Actor NewBase,
		float NewFloorX,
		float NewFloorY,
		float NewFloorZ
	)
	{
	}

	function BeginState()
	{
		local Pawn P;

		EndZoom();
		ResetFOV();
        FOVAngle = DesiredFOV;
		if( Pawn != None && Pawn.Weapon != None )
		{
			Pawn.Weapon.SetWeapFOV(FOVAngle);
		}
		bFire = 0;
		bAltFire = 0;
		if ( Pawn != None )
		{
			Pawn.Velocity = vect(0,0,0);
			Pawn.SetPhysics(PHYS_None);
			Pawn.AmbientSound = None;
			Pawn.SimAnim[0].RateScale = 0;
			Pawn.bPhysicsAnimUpdate = false;
			Pawn.StopAnimating();
            Pawn.SetCollision(true,false,false);
            StopFiring();
 			Pawn.bIgnoreForces = true;
		}
		myHUD.bShowScores = true;
		bFrozen = true;
		if ( !bFixedCamera )
		{
			FindGoodView();
			bBehindView = true;
		}
        SetTimer(5, false);
		ForEach DynamicActors(class'Pawn', P)
		{
			if ( P.Role == ROLE_Authority )
				P.RemoteRole = ROLE_DumbProxy;
			P.SetCollision(true,false,false);
			P.AmbientSound = None;
			P.Velocity = vect(0,0,0);
			P.SetPhysics(PHYS_None);
            P.bPhysicsAnimUpdate = false;
            P.StopAnimating();
            P.bIgnoreForces = true;
		}
	}

Begin:
}

state CtrlIncapacitated
{
	ignores SeePlayer, HearNoise, KilledBy, SwitchWeapon,
		NextWeapon, PrevWeapon, ForceReload, FOV, Use;

	exec function Fire( optional float F )
	{
		if ( bFrozen )
		{
			if ( (TimerRate <= 0.0) || (TimerRate > 1.0) )
				bFrozen = false;
			return;
		}
	}

	exec function AltFire( optional float F )
	{
		Fire(F);
	}

	exec function ThrowGrenade(optional float F)
	{
		Fire(F);
	}

	function ServerMove
	(
		float TimeStamp,
		vector Accel,
		vector ClientLoc,
		bool NewbRun,
		bool NewbDuck,
		bool NewbJumpStatus,
        bool NewbDoubleJump,
		eDoubleClickDir DoubleClickMove,
		byte ClientRoll,
		int View,
		optional byte OldTimeDelta,
		optional int OldAccel
	)
	{
		Global.ServerMove(
					TimeStamp,
					Accel,
					ClientLoc,
					false,
					false,
					false,
                    false,
					DoubleClickMove,
					ClientRoll,
					View);
	}

	function PlayerMove(float DeltaTime)
	{
		local rotator ViewRotation;

		if ( !bFrozen )
		{
			if ( bPressedJump )
			{
				Fire(0);
				bPressedJump = false;
			}

			ViewRotation = RelativeRotation;

			ViewRotation.Yaw += 4.0 * DeltaTime * aTurn;
			ViewRotation.Pitch += 4.0 * DeltaTime * aLookUp;
			ViewRotation.Yaw = Clamp( ViewRotation.Yaw, -8000, 8000 );
			ViewRotation.Pitch = Clamp( ViewRotation.Pitch, -8000, 8000 );
			SetRelativeRotation( ViewRotation );
		}
        else if ( (TimerRate <= 0.0) || (TimerRate > 1.0) )
			bFrozen = false;

		ViewShake(DeltaTime);
		ViewFlash(DeltaTime);
	}

	function BeginState()
	{
		StopFiring();
		EndZoom();
		FOVAngle = DesiredFOV;
		Target = None;
		TargetType = TGT_Default;
		TargetMarker = None;
		Pawn.bUpdateEyeHeight = false;
		Enemy = None;
		//bBehindView = true;
		bJumpStatus = false;
		bPressedJump = false;
        bBlockCloseCamera = true;
		bValidBehindCamera = false;
		//FindGoodView();
		CleanOutSavedMoves();
		Pawn.AttachToBone( self, 'DeathCamera' );
		SetTimer(2.0,false);
		SavedVisionMode = Pawn.CurrentUserVisionMode;
		GotoVisionMode( 0 );
	}

	function Timer()
	{
		if( Pawn != None && Pawn.Weapon != None )
			Pawn.Disarm();
	}

	function EndState()
	{
		local Rotator Rot;

		bBlockCloseCamera = false;
		CleanOutSavedMoves();
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);
		Rot = Rotation;
		Rot.Pitch = 0;
		SetRotation(Rot);
		bBehindView = false;
		bPressedJump = false;
		myHUD.bShowScores = false;
		Pawn.SwitchToLastWeapon();
		Pawn.bUpdateEyeHeight = true;
		SetBase(None);
		GotoVisionMode( SavedVisionMode );
	}

	function rotator GetViewRotation()
	{
		return Pawn.GetBoneRotation('DeathCamera') + RelativeRotation;
	}

	function CalcFirstPersonView( out vector CameraLocation, out rotator CameraRotation )
	{
		CameraLocation = Pawn.GetBoneLocation('DeathCamera');
		CameraRotation = Pawn.GetBoneRotation('DeathCamera') + RelativeRotation;
	}
}

state Dead extends CtrlIncapacitated
{
ignores QuickSave, SeePlayer, HearNoise, KilledBy, SwitchWeapon, NextWeapon, PrevWeapon;

	//gdr Need to continue to send empty movements to have the server update our ping.
	//    Makes sense in some strange Unreal way doesn't it?
	function PlayerMove(float DeltaTime)
	{
        if ( Role < ROLE_Authority ) // then save this move and replicate it
            ReplicateMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
        else
            ProcessMove(DeltaTime, vect(0,0,0), DCLICK_None, rot(0,0,0));
	}

	function bool IsDead()
	{
		return true;
	}

	function ServerReStartPlayer()
	{
		if ( Level.NetMode == NM_Client )
			return;

		Level.Game.RestartPlayer(self);
	}

	function PawnDied(Pawn P)
	{
		if ( Level.NetMode != NM_Client )
			warn(self$" PawnDied while dead");
	}

	exec function Fire( optional float F )
	{
		if ( bFrozen )
		{
			if ( (TimerRate <= 0.0) || (TimerRate > 1.0) )
				bFrozen = false;
			return;
		}
		//** SBD - Now that we have a death pause menu, we
		//		   don't want to do this anymore in single player
		if ( Level.NetMode != NM_Standalone )
		{
			LoadPlayers();
			ServerReStartPlayer();
		}
	}

	exec function AltFire( optional float F )
	{
		Fire(F);
	}

	exec function ThrowGrenade(optional float F)
	{
		Fire(F);
	}

	function Timer()
	{
		if ( Player != None &&
			 Player.Console != None &&
			 Player.Console.CurMenu == None )
		{
			ShowMenu();
		}

		if (!bFrozen)
			return;

		bFrozen = false;
		bPressedJump = false;
	}

	function FindGoodView()
	{
		local vector cameraLoc;
		local rotator cameraRot, ViewRotation;
		local int tries, besttry;
		local float bestdist, newdist;
		local int startYaw;
		local actor ViewActor;

		////log("Find good death scene view");
		ViewRotation = Rotation;
		ViewRotation.Pitch = 56000;
		tries = 0;
		besttry = 0;
		bestdist = 0.0;
		startYaw = ViewRotation.Yaw;

		for (tries=0; tries<16; tries++)
		{
			cameraLoc = ViewTarget.Location;
			SetRotation(ViewRotation);
			PlayerCalcView(ViewActor, cameraLoc, cameraRot);
			newdist = VSize(cameraLoc - ViewTarget.Location);
			if (newdist > bestdist)
			{
				bestdist = newdist;
				besttry = tries;
			}
			ViewRotation.Yaw += 4096;
		}

		ViewRotation.Yaw = startYaw + besttry * 4096;
		SetRotation(ViewRotation);
	}

	function BeginState()
	{
		local float VisorFadeTime;
		local color FlashColor;
		local color FadeColor;

		//if ( (Pawn != None) && (Pawn.Controller == self) )
		//	Pawn.Controller = None;

		EndZoom();
		FOVAngle = DesiredFOV;
		if( Pawn != None && Pawn.Weapon != None )
		{
			Pawn.Weapon.SetWeapFOV(FOVAngle);
		}

		//Pawn = None;
		Enemy = None;
		//bBehindView = true;
		bFrozen = true;
		bJumpStatus = false;
		bPressedJump = false;
        //bBlockCloseCamera = true;
		//bValidBehindCamera = false;
		//FindGoodView();
        //SetTimer(1.0, false);
		StopForceFeedback();
		ClientPlayForceFeedback("Damage");  // jdf
		CleanOutSavedMoves();

		VisorFadeTime = 5;

		FlashColor.R = 180;
		FlashColor.G = 180;
		FlashColor.B = 180;

		FadeColor.R = 255;
		FadeColor.G = 255;
		FadeColor.B = 255;

		StartScreenFade( VisorFadeTime, FadeColor );

		BlendInAddColor( FlashColor, VisorFadeTime, 10, 0  );
		AnimateBloomFilter( 255, VisorFadeTime, 10, 0 );
		AnimateBlur( 200, VisorFadeTime, 10, 0 );

		SetTimer( VisorFadeTime, false );

		if (( Level.NetMode == NM_Standalone ) && (Pawn != None))
		{
			SavedVisionMode = Pawn.CurrentUserVisionMode;
			GotoVisionMode( 0 );
		}
	}

	function EndState()
	{
		bBlockCloseCamera = false;
		CleanOutSavedMoves();
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);
//        if ( !PlayerReplicationInfo.bOutOfLives )
		bBehindView = false;
		bPressedJump = false;
		myHUD.bShowScores = false;

		if ( Level.NetMode == NM_Standalone )
			GotoVisionMode( SavedVisionMode );
	}
Begin:
    Sleep(3.0);
    myHUD.bShowScores = true;
}

state Briefing
{
	ignores QuickSave, ToggleHeadLamp, ForceReload;

	function BeginState()
	{
		Pawn.Velocity = vect(0,0,0);
		Pawn.Acceleration = vect(0,0,0);
		SavedVisionMode = Pawn.CurrentUserVisionMode;
		GotoVisionMode( 0 );
		bBriefing = true;
		bOkToSwitchWeapon = false;
		Pawn.bWantsToCrouch = false;
	}

	function PlayerMove(float DeltaTime)
	{
		// Rotation only.
		UpdateRotation(DeltaTime, 1);
	}


	function EndState()
	{
		bBriefing = false;
		bOkToSwitchWeapon = true;
		if ( Pawn != None )
		{
			if (Pawn.Weapon == None)
				Pawn.SwitchToLastWeapon();
			else
				Pawn.Weapon.BringUp(None);
		}

		GotoVisionMode( SavedVisionMode );
	}

}

//------------------------------------------------------------------------------
// Control options
function ChangeStairLook( bool B )
{
	bLookUpStairs = B;
	if ( bLookUpStairs )
		bAlwaysMouseLook = false;
}

function ChangeAlwaysMouseLook(Bool B)
{
	bAlwaysMouseLook = B;
	if ( bAlwaysMouseLook )
		bLookUpStairs = false;
}

simulated event ClientSetBadCDKey( bool bSet )
{
	log("ClientSetBadCDKey"@bBadCDKey@bSet);
	bBadCDKey = bSet;
	PropagateBadCDKey();
}

simulated event ClientSetMissingContent( bool bSet, string ContentName )
{
	log("ClientSetMissingContent"@bMissingContent@bSet@ContentName);
	bMissingContent = bSet;
	MissingContentName = ContentName;
	PropagateMissingContent();
}

// Replace with good code

event ClientOpenMenu (string Menu, optional bool bDisconnect,optional string Msg1, optional string Msg2)
{
	/* NO_GUI_UI
	if (Player.GUIController == None)
	{
	*/
		ClientOpenXMenu(Menu, bDisconnect, Msg1, Msg2);
		return;
	/* NO_GUI_UI
	}

	Player.GUIController.OpenMenu(Menu, Msg1, Msg2);
	*/

	if (bDisconnect)
		ConsoleCommand("Disconnect");
}

event ClientOpenXMenu (string Menu, optional bool bDisconnect,optional string Msg1, optional string Msg2)
{
    local class<Menu> MenuClass;

    MenuClass = class<Menu>( DynamicLoadObject( Menu, class'Class' ) );

    if( MenuClass == None )
    {
        log( "Could not load menu! ["$Menu$"]", 'Error' );
        return;
    }
    MenuOpen( MenuClass, "RESET "$Msg1$" "$Msg2 );

	if (bDisconnect)
		ConsoleCommand("Disconnect");
}

event ClientShowSingularMenu ( string Menu, string Args )
{
	local class<Menu> MenuClass;

    MenuClass = class<Menu>( DynamicLoadObject( Menu, class'Class' ) );

    if( MenuClass == None )
    {
        log( "Could not load menu! ["$Menu$"]", 'Error' );
        return;
    }

    if ( Player.Console.CurMenu.Class == MenuClass )
    {
		// We're already showing one
		return;
    }

	StopForceFeedback();  // jdf - no way to pause feedback

	/* // Pause if not already
	if(Level.Pauser == None)
		SetPause(true);
    */

    if ( Player.Console.CurMenu == None )
    {
 log("PC.C.PauseLevelMusic()");
		Player.Console.PauseLevelMusic();
    }

    Player.Console.MenuCall( MenuClass, Args );
}

event ClientCloseMenu(optional bool bCloseAll, optional bool bCancel)
{
	/* NO_GUI_UI
	if (Player.GUIController == None)
	{*/
		ClientCloseXMenu(bCloseAll, bCancel);
		return;
	/* NO_GUI_UI
	}

	if (bCloseAll)
		Player.GUIController.CloseAll(bCancel);
	else
		Player.GUIController.CloseMenu(bCancel);
	*/
}

event ClientCloseXMenu(optional bool bCloseAll, optional bool bCancel)
{
	MenuClose();
}

function bool CanRestartPlayer()
{
    return !PlayerReplicationInfo.bOnlySpectator;
}

// ali 05-04-04 Xbox Live Voice Chat

simulated event ServerChangeVoiceChatter( PlayerController Player, XboxAddr xbAddr, int Handle, int VoiceChannels, bool Add )
{
	if( (Level.NetMode == NM_DedicatedServer) || (Level.NetMode == NM_ListenServer) )
	{
		Level.Game.ChangeVoiceChatter( Player, xbAddr, Handle, VoiceChannels, Add );
	}
}

simulated event ServerChangeHasVoice( bool bVoiceOn )
{
	if (PlayerReplicationInfo != None)
		PlayerReplicationInfo.bHasVoice = bVoiceOn;
}

simulated event ServerGetVoiceChatters( PlayerController Player )
{
	local int i;

	if( (Level.NetMode == NM_DedicatedServer) || (Level.NetMode == NM_ListenServer) )
	{
		for( i=0; i<Level.Game.VoiceChatters.Length; i++ )
		{
			if( Player != Level.Game.VoiceChatters[i].Controller )
			{
				Player.ClientChangeVoiceChatter( Level.Game.VoiceChatters[i].xbAddr, Level.Game.VoiceChatters[i].Handle, Level.Game.VoiceChatters[i].VoiceChannels, true );
			}
		}
	}
}


// update on/off according to teams
function UpdateVoiceChatters()
{
	local int Index;
	local PlayerController PC;

	if( Level.NetMode != NM_Standalone )
	{
		for( Index=0; Index<Level.Game.VoiceChatters.Length; Index++ )
		{
			PC = PlayerController(Level.Game.VoiceChatters[Index].Controller);

			if (PC != self)
			{
				if( PC != None &&
					PC.PlayerReplicationInfo != None &&
					PC.PlayerReplicationInfo.Team == self.PlayerReplicationInfo.Team )
				{
					Log(" Turning ON voice for"@PC.GamerTag);
					PC.ClientChangeVoiceChatter( Level.Game.VoiceChatters[Index].xbAddr, Level.Game.VoiceChatters[Index].Handle, Level.Game.VoiceChatters[Index].VoiceChannels, true );
				}
				else
				{
					Log(" Turning OFF voice for"@PC.GamerTag);
					PC.ClientChangeVoiceChatter( Level.Game.VoiceChatters[Index].xbAddr, Level.Game.VoiceChatters[Index].Handle, 0, false );
				}
			}
		}
	}
}
simulated function ClientChangeVoiceChatter( XboxAddr xbAddr, int Handle, int VoiceChannels, bool Add )
{
	ChangeVoiceChatter( xbAddr, Handle, VoiceChannels, Add );
}

simulated function ClientLeaveVoiceChat()
{
	LeaveVoiceChat();
}

native final function LeaveVoiceChat();
native final function ChangeVoiceChatter( XboxAddr xbAddr, int Handle, int VoiceChannels, bool Add );

//UTrace
function ServerUTrace()
{
	if( Level.NetMode != NM_Standalone )
		return;

	UTrace();
}

exec function UTrace()
{
	// If they're running with "-log", be sure to turn it off
	ConsoleCommand("HideLog");
	if( Role!=ROLE_Authority )
		ServerUTrace();
	SetUTracing( !IsUTracing() );
	log("UTracing changed to "$IsUTracing()$" at "$Level.TimeSeconds);
}


//--------------------- Demo recording stuff

// Called on the client during client-side demo recording
simulated event StartClientDemoRec()
{
	// Here we replicate functions which the demo never saw.
	DemoClientSetHUD( MyHud.Class, MyHud.ScoreBoard.Class );

	// tell server to replicate more stuff to me
	bClientDemo = true;
	ServerSetClientDemo();
}

function ServerSetClientDemo()
{
	bClientDemo = true;
}

// Called on the playback client during client-side demo playback
simulated function DemoClientSetHUD(class<HUD> newHUDClass, class<Scoreboard> newScoringClass )
{
	if( MyHUD == None )
		ClientSetHUD( newHUDClass, newScoringClass );
}

simulated event MenuOpen (class<Menu> MenuClass, optional String Args)
{
    if (Player == None)
    {
        log ("PlayerController::MenuOpen: can't open menu without a player", 'Error');
        return;
    }

    if (Player.Console == None)
    {
        log ("PlayerController::MenuOpen: can't open menu without a console", 'Error');
        return;
    }

    Player.Console.MenuOpen (MenuClass, Args);
}

simulated event MenuClose()
{
    if (Player == None)
    {
        log ("PlayerController::MenuClose: can't close menu without a player", 'Error');
        return;
    }

    if (Player.Console == None)
    {
        log ("PlayerController::MenuClose: can't close menu without a console", 'Error');
        return;
    }

	// Refresh the number of existing saves
	if (Level.NetMode == NM_StandAlone)
		CurrentProfileNumSaves();

    Player.Console.MenuClose();
}

exec function PlayM(INT movieNum)
{
   /*if(movieNum == 0)
   {*/
      //Plays the movie directly to the frame buffer at the size of
      //the movie regaurdless of screen resolution.
      //Upper-left corner of movie is at 50, 50.
      myHud.PlayMovieDirect("xbox_trailer_av1400.xmv", 50, 50, true, false);
   /*}
   else if(movieNum == 1)
   {
      //Plays the movie scaled to 1/2 the size of the screen in the center.
      myHud.PlayMovieScaled(Texture'Movies.Movie1', 0.25, 0.25, 0.75,
                                                   0.75, false, false);
   }
   else if(movieNum == 2)
   {
      //Same as above but the movie is drawn using "Translucent" draw style.
      myHud.PlayMovieScaled(Texture'Movies.Movie1', 0.25, 0.25, 0.75,
                                             0.75, false, false, true);
   }
   else if(movieNum == 3)
   {
      //Plays the movie full screen with a sound played at
      //the same time to match the video.
      myHud.PlayMovieScaled(Texture'Movies.Movie1', 0, 0, 1, 1, false,
                                                                false);
      ViewTarget.PlaySound(Sound'CL_Sounds.SimpleTest1');
   }*/
}

//=============================================================================
// Status drawing.

simulated function BlendInAddColor( color NewColor, float InTime, float SustainTime, float OutTime )
{
	local color Black;
	Black.R = 0;
	Black.G = 0;
	Black.B = 0;

	AddStages[0].BaseColor = ColorAdd;
	AddStages[0].TotalTime = InTime;
	AddStages[0].RemainingTime = InTime;
	AddStages[0].GoalColor = NewColor;

	AddStages[1].BaseColor = NewColor;
	AddStages[1].TotalTime = SustainTime;
	AddStages[1].RemainingTime = SustainTime;
	AddStages[1].GoalColor = NewColor;

	AddStages[2].BaseColor = NewColor;
	AddStages[2].TotalTime = OutTime;
	AddStages[2].RemainingTime = OutTime;
	AddStages[2].GoalColor = Black;

	CurrentAddStage = CBS_In;
}

simulated function BlendInMultColor( color NewColor, float InTime, float SustainTime, float OutTime )
{
	local color White;
	White.R = 255;
	White.G = 255;
	White.B = 255;

	MultStages[0].BaseColor = ColorMultiply;
	MultStages[0].TotalTime = InTime;
	MultStages[0].RemainingTime = InTime;
	MultStages[0].GoalColor = NewColor;

	MultStages[1].BaseColor = NewColor;
	MultStages[1].TotalTime = SustainTime;
	MultStages[1].RemainingTime = SustainTime;
	MultStages[1].GoalColor = NewColor;

	MultStages[2].BaseColor = NewColor;
	MultStages[2].TotalTime = OutTime;
	MultStages[2].RemainingTime = OutTime;
	MultStages[2].GoalColor = White;

	CurrentMultStage = CBS_In;
}

simulated function AnimateBlur( int NewBlurLevel, float InTime, float SustainTime, float OutTime )
{
	BlurStages[0].BaseColor.R = Blur;
	BlurStages[0].TotalTime = InTime;
	BlurStages[0].RemainingTime = InTime;
	BlurStages[0].GoalColor.R = NewBlurLevel;

	BlurStages[1].BaseColor.R = NewBlurLevel;
	BlurStages[1].TotalTime = SustainTime;
	BlurStages[1].RemainingTime = SustainTime;
	BlurStages[1].GoalColor.R = NewBlurLevel;

	BlurStages[2].BaseColor.R = NewBlurLevel;
	BlurStages[2].TotalTime = OutTime;
	BlurStages[2].RemainingTime = OutTime;
	BlurStages[2].GoalColor.R = 0;

	CurrentBlurStage = CBS_In;
}

simulated function AnimateBloomFilter( int NewBloomFilter, float InTime, float SustainTime, float OutTime )
{
	BloomStages[0].BaseColor.R = Bloom;
	BloomStages[0].TotalTime = InTime;
	BloomStages[0].RemainingTime = InTime;
	BloomStages[0].GoalColor.R = NewBloomFilter;

	BloomStages[1].BaseColor.R = NewBloomFilter;
	BloomStages[1].TotalTime = SustainTime;
	BloomStages[1].RemainingTime = SustainTime;
	BloomStages[1].GoalColor.R = NewBloomFilter;

	BloomStages[2].BaseColor.R = NewBloomFilter;
	BloomStages[2].TotalTime = OutTime;
	BloomStages[2].RemainingTime = OutTime;
	BloomStages[2].GoalColor.R = 0;

	CurrentBloomStage = CBS_In;
}

exec function PauseM()
{
   myHud.PauseMovie(!myHud.IsMoviePaused());
}

exec function StopM()
{
   myHud.StopMovie();
}

simulated event PotentialSubtitledSoundPlayed(string SoundName, float Duration, int Priority)
{
}

simulated event bool InTacticalVisionMode()
{
	return False;
}

simulated event CheckpointSaveStarted()
{
}

simulated function bool CanEnableHints()
{
	return (bKeepHintMenusAwfulHack == false && Level != None && Level.bLevelContainsHints);
}


defaultproperties
{
     bAlwaysMouseLook=True
     CloneAccessories(0)=-1
     CloneAccessories(1)=-1
     CloneAccessories(2)=-1
     CloneAccessories(3)=-1
     CloneAccessories(4)=-1
     TrandoAccessories(0)=-1
     TrandoAccessories(1)=-1
     TrandoAccessories(2)=-1
     TrandoAccessories(3)=-1
     TrandoAccessories(4)=-1
     VoiceIndex(0)=1
     VoiceIndex(1)=1
     bVisor=True
     VisorModeDefault=2
     TacticalModeIntensity=0.2
     bGlobalRumbleActive=True
     bRumbleActive=True
     bAutoPullManeuvers=True
     bOkToSwitchWeapon=True
     bZeroRoll=True
     bDynamicNetSpeed=True
     bKeepHintMenusAwfulHack=True
     AnnouncerLevel=2
     AnnouncerVolume=4
     MaxResponseTime=0.125
     OrthoZoom=40000
     CameraDist=9
     DesiredFOV=85
     DefaultFOV=85
     FlashScale=(X=1,Y=1,Z=1)
     ColorMultiply=(B=255,G=255,R=255,A=255)
     MaxTimeMargin=0.35
     ProgressTimeOut=8
     NoPauseMessage="Game is not pauseable"
     ViewingFrom="Now viewing from"
     OwnCamera="Now viewing from own camera"
     QuickSaveName="QuickSave"
     AutoSaveName="AutoSave"
     NoQuickOrAutoSave="THERE IS NO QUICKSAVE OR AUTOSAVE TO LOAD."
     NetSplitID=-1
     bIsGuest=True
     LocalMessageClass=Class'Engine.LocalMessage'
     ShakeTimeElapsed=-1
     EnemyTurnSpeed=45000
     CheatClass=Class'Engine.CheatManager'
     InputClass=Class'Engine.PlayerInput'
     SpectateSpeed=600
     DynamicPingThreshold=400
     bEnablePickupForceFeedback=True
     bEnableWeaponForceFeedback=True
     bEnableDamageForceFeedback=True
     bEnableGUIForceFeedback=True
     bForceFeedbackSupported=True
     TeamBeaconMaxDist=5000
     TeamBeaconPlayerInfoMaxDist=1200
     TeamBeaconTeamColors(0)=(B=186,G=93,R=93,A=255)
     TeamBeaconTeamColors(1)=(B=72,G=75,R=240,A=255)
     LastSpeedHackLog=-100
     DifficultyDamageModifier=1
     DamageModifierEasy=0.25
     DamageModifierMedium=1
     DamageModifierHard=1.75
     FovAngle=85
     Handedness=1
     bIsPlayer=True
     bCanOpenDoors=True
     bCanDoSpecial=True
     bTravel=True
     NetPriority=3
}
