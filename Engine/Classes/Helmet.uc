class Helmet extends Object
	native
	collapsecategories;

// Used to specify constants for a vertex shader. 
 struct SplatInfo
{
	var Material	SplatDecal;		// Splat material
	var Vector		SplatPosition;	// Splat position where z is scale
	var float		SplatRotation;	// Splat rotation
};

// Frame Effects class
var FrameFX		FrameFX;

// User defined variables
var() byte		BlurNormal;
var() byte		BlurHit;

// HUD Visor FX variables
var array<SplatInfo>	Splatters;			// Array of current splatters
var	transient bool		Dirty;				// Indicates whether a wipe is needed
var	transient float		NextWipeTime;		// Time the helmet got dirt/damage on it
var transient float		WipeProgress;		// Wipe progress from 0 to 1

var()		  float		WipeTime;			// Time it takes for wiper to complete
var()		  float		BloodWipeDelay;		// Delay before wiping blood
var()		  float		HitWipeDelay;		// Delay before wiping hit

var	transient float		TimeSinceVisionChange;
var transient VisionMode LastVisionMode;
var transient bool		bFadeOut;			// Busy fading out from old vision mode

var		  Light		InteriorHealthLight;	// Lights up the interior of the helmet
var		  Light		InteriorShieldLight;	// Lights up the interior of the helmet

// State Tracking variables for the HUD
//		These variables track the players last state to track changes
//		in the players condition.

var const byte	BlurCurrent;
var const int	LastHealthLevel;
var const float LastHealth;
var const float	HealthFlashTime;
var const float	HealthFlashDuration;
var const float	LastShield;
var const float	ShieldFlashTime;
var const float	ShieldFlashDuration;
var const int	LastStance;
var const float	StanceGlowTime;
var const int	LastCount;
var const int	LastTargetType;
var	const Sound	StaticSound;
var	const PlayerReplicationInfo pPrevTarget;
var const float TargetHitBlipTime;
var const float TargetHitBlipDuration;

var		  Weapon LastWeapon;
var		  Weapon LastLastWeapon;
var		  float	TimeWeaponChanged;
var()	  float AmmoSpinupTime;

var()	  float HitLowHealth;
var()	  float HitLargeDamage;
var		  float HitLastTime;

var		  float ShieldInitStart;
var		  float ShieldInitEnd;
var localized String ShieldInitText;
var		  float HealthInitStart;
var		  float HealthInitEnd;
var localized String HealthInitText;
var		  float SquadInitStart;
var		  float SquadInitEnd;
var localized String SquadInitText;
var	const float InitTime;

var		  float PromptFadeInTime;

var localized String CancelManeuverText;
var localized String EngageTargetHealthText;

var	localized String IncapacitatedText;
var localized String SquadUnavailableText;
var localized String HealthCriticalText;
var localized String FullHealthText;

var localized String TacticalVisionModeText;
var localized String LowLightVisionModeText;

var() localized String IncapMenuMaintainText;
var() localized String IncapMenuRecallText;
var() localized String IncapMenuReloadPCText;
var() localized String IncapMenuReloadXboxText;
var() int IncapMenuSelectedItem;

var() localized String	Checkpoint;
var() localized String	XboxCheckpointAddition;
var() float				ElapsedCheckpointSaveTime;
var() float				CheckpointSaveTime;

var() String GenericPromptText;	// NOTE that this is supplied from elsewhere, and thus is localized elsewhere
var() Array<String> GenericPromptButtonFuncs;
var() float GenericPromptTime;
var() bool	GenericPromptFlash;

var() localized String DemolitionReadyPromptText;
var() Array<String> DemolitionReadyPromptButtonFuncs;
var() localized String MountTurretPromptText;
var() Array<String> MountTurretPromptButtonFuncs;
var() localized String SquadOccupiedText;
var() localized String SquadIncapacitatedText;
var() localized String ReviveSquadmatePromptText;
var() Array<String> ReviveSquadmatePromptButtonFuncs;
var() localized String PickupPromptText;
var() Array<String> PickupPromptButtonFuncs;
var() localized String RemoveScavPromptText;
var() Array<String> RemoveScavPromptButtonFuncs;

var() localized String SearchAndDestroy;
var() localized String FormUp;
var() localized String SecurePosition;
var() localized String Cancel;

// Subtitles
struct SubtitleGroup
{
	var String GroupName;
	var int NumEntries;
};
var() array<SubtitleGroup> SubtitleGroupInfo;

struct SubtitleEntry
{
	var String SoundID;
	var String Text;
};
var() array<SubtitleEntry> SubtitleEntries;
//var private native const int SubtitleEntries[5];	// TMap<FString, FString>!

var() String SubtitleText;	// NOTE that this is supplied from elsewhere, and thus is localized elsewhere
var() float SubtitleDisplayTime;
var() int SubtitlePriority;

// Objective tracker
var		  Actor ObjectiveActor;
var		  Vector ObjectivePosition;

var const float LastSniperZoomRechargeCenterOffset;

var const float	FadeTime;
var const Color FadeColor;
var	const float	ElapsedFadeTime;

var		  float SquadCommandPromptDelay;
var		  float ElapsedSquadCommandPromptTime;
var		  float SquadCommandPromptFadeInTime;

var		  int	SelectedSquadCommand;
var		  float ElapsedSelectedSquadCommandTime;
var		  float	SelectedSquadCommandPulseTime;


var		  bool  bDraw;				// Do we draw ANYTHING?
var const bool	bWiping;			// Is the visor being wiped?
var const bool	PlayingShieldChargeSound;
var const bool	PlayingHealthChargeSound;
var const bool  PlayingHealSound;
var		  bool	bShowObjectiveTracker;
var		  bool  bBracketVisibleObjective;
var const bool	bDoingFade;
var		  bool  bDoInitSequence;
var		  bool  bShowSquadCommandPrompt;

var		  bool	bBlinkSquad;
var		  bool	bBlinkGrenade;
var		  bool	bBlinkHealth;
var		  bool  bBlinkShields;
var		  bool	bBlinkSquadCommandPrompt;

var		  bool  bInTacticalVisionMode;

var		  bool	bCheckpointSave;

var transient const bool bSubtitleTextLastFrame;
var transient const bool bGenericPromptTextLastFrame;
var transient const bool bCurrentObjectiveTextLastFrame;
var transient const bool bMarkerTextLastFrame;
var transient const bool bActivateTextLastFrame;
var transient const bool bDetonateTextLastFrame;
var transient const bool bPickupTextLastFrame;
var transient const bool bTurretTextLastFrame;
var transient const bool bReviveTextLastFrame;
var transient const bool bPickupPromptTextLastFrame;
var transient const bool bRemoveScavPromptTextLastFrame;

var		  float SquadBlinkTime;
var		  float ElapsedSquadBlinkTime;

var		  float GrenadeBlinkTime;
var		  float ElapsedGrenadeBlinkTime;

var		  float HealthBlinkTime;
var		  float ElapsedHealthBlinkTime;

var		  float ShieldsBlinkTime;
var		  float ElapsedShieldsBlinkTime;

var		  float SquadCommandPromptBlinkTime;
var		  float ElapsedSquadCommandPromptBlinkTime;

var		  float VisorModeTextTime;
var		  float ElapsedVisorModeSwitchTime;

var const Array<Actor> LightArray;	// Interior helmet lights

var		  float VisCheckFrequency;



struct CachedMarkerVisInfo
{
	var Actor	mpActor;
	var float	mfElapsedVisCheckTime;
	var float	mfScreenX;
	var float	mfScreenY;
	var float	mfScreenXSize;
	var float	mfScreenYSize;
	var bool	mbVisible;
	var bool	mbUsedThisFrame;
	var bool	mbTargeted;
};

var const CachedMarkerVisInfo mCachedMarkerVisInfo[20];

struct CachedObjectiveVisInfo
{
	var float	mfElapsedVisCheckTime;
	var bool	mbVisible;
};

var		  CachedObjectiveVisInfo mCachedObjectiveVisInfo;

var transient const String LastGrenade;
var transient const float LastTime;

var const transient String LastSubtitleText;
var const float fSubtitleStartTime;

var const transient String LastGenericPromptText;
var const float fGenericPromptStartTime;

var const transient String LastCurrentObjectiveText;
var const float fCurrentObjectiveStartTime;

var const transient String LastPickupText;
var const float fPickupStartTime;

var const transient String LastPickupPromptText;
var const float fPickupPromptStartTime;

var const transient String LastRemoveScavPromptText;
var const float fRemoveScavPromptStartTime;

var const transient String LastMarkerText;
var const float fMarkerStartTime;

var const transient String LastActivateText;
var const float fActivateStartTime;

var const transient String LastDetonateText;
var const float fDetonateStartTime;

var const transient String LastTurretText;
var const float fTurretStartTime;

var const transient String LastReviveText;
var const float fReviveStartTime;

var(Assets) Texture	LargeVisorIcons;
var(Assets) Texture	CloneVisorIcons;
var(Assets) Texture	MPVisorIcons;
var(Assets) Texture	TexAmmo;
var(Assets) Texture	TexClip;
var(Assets) Texture	TexHit;
var(Assets) Texture	MarkerArrows;
var(Assets) Texture	TexAButton;
var(Assets) Texture	TexBButton;
var(Assets) Texture	TexXButton;
var(Assets) Texture	TexYButton;
var(Assets) Texture	TexBlackButton;
var(Assets) Texture	TexWhiteButton;
var(Assets) Texture	TexDPadMonoButton;
var(Assets) Texture	TexCompassGraphic;
var(Assets) Texture	TexTargetBlip;
var(Assets) Texture	TexObjectiveRing;
var(Assets) Texture	TexDPadBlue;
var(Assets) Texture TexSniperRecharge;
var(Assets) Texture TexSniperNonZoomReticle;
var(Assets) Texture TexTextLine01;
var(Assets) Texture TexText01;
var(Assets) Texture TexMarkerArrows;
var(Assets) Texture TexHealRing;
var(Assets) Texture TexHealIndicator;
var(Assets) Texture TexUseRing;
var(Assets) Texture TexUseIndicator;
var(Assets) Texture TexTrandoRollReticle;
var(Assets) Texture TexTacticalLine;
var(Assets) Texture TexTacticalGraphic;
var(Assets) Texture TexBowcasterClip;
var(Assets) Texture TexBowcasterRecharge;

var(Assets) Texture EventTimer;

var(Assets) Texture IncapMenuButton;

var(Assets) HardwareShader HSBloodSplatter;
var(Assets) HardwareShader HSHitEffect;
var(Assets) HardwareShader HSWiper;

var(Assets) font			Orbit8Font;
var(Assets)	font			Orbit15Font;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)


exec function ShowActorObjectiveTracker( Actor a )
{
	if ( None == a )
		return;

	ObjectiveActor = a;
	bShowObjectiveTracker = true;
	// Reset this so it will check
	mCachedObjectiveVisInfo.mfElapsedVisCheckTime = VisCheckFrequency;
}

exec function ShowPositionObjectiveTracker( Vector position )
{
	ObjectiveActor = None;
	ObjectivePosition = position;
	bShowObjectiveTracker = true;
	// Reset this so it will check
	mCachedObjectiveVisInfo.mfElapsedVisCheckTime = VisCheckFrequency;
}

exec function HideObjectiveTracker()
{
	ObjectiveActor = None;
	bShowObjectiveTracker = false;
}    

// Make these native so the lookup will be as fast as it can be
simulated native function LoadSubtitles();
simulated native function PotentialSubtitledSoundPlayed(String SoundName, float Duration, int Priority);

simulated function CheckpointSaveStarted()
{
	bCheckpointSave = True;
	ElapsedCheckpointSaveTime = 0.0;
}

simulated native function ShowSquadCommandPrompt( bool bShow );
simulated function SquadCommandSelected(int command)
{
	if ( bShowSquadCommandPrompt )
	{
		SelectedSquadCommand = command;
		ElapsedSelectedSquadCommandTime = 0.0;
	}
}



cpptext
{
	#define MAX_CACHED_MARKERS 20
	// --------- UObject interface --------- 

	void Destroy();

	// --------- UHelmet interface --------- 

	// HUD Draw functions
	void DrawHUD( UViewport* Viewport, FRenderInterface* RI );
	void SetupVisionMode( UViewport* Viewport );
	void DrawTextInfo( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY );
	bool DrawPlayerTargetTextInfo( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, APawn* pPawn );
	bool DrawObjectiveText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY );
	bool DrawIncapOrDeathText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY );
	bool DrawStanceText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY );	
	bool DrawSubtitleText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawGenericPromptText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawCurrentObjectiveText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawPickupText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawPickupPromptText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawRemoveScavPromptText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawMarkerText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawActivateText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawDetonateText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawTurretText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	bool DrawReviveText( UViewport* Viewport, APlayerController* Owner, float InvScaleX, float InvScaleY, bool bDrewLastFrame );
	void DrawPromptText( UViewport* Viewport, float InvScaleX, float InvScaleY, const FColor& color, const FString& Text, const TArrayNoInit<FString>* const pPromptButtonFuncs );
	void DrawHelmet( UViewport* Viewport, FRenderInterface* RI );
	void DrawSniperZoom( UViewport* Viewport, APlayerController* Owner, float Time, float ElapsedTime, float InvScaleX, float InvScaleY, int Brightness );
	void DrawWookieeBowcasterZoom( UViewport* Viewport, APlayerController* Owner, float Time, float ElapsedTime, float InvScaleX, float InvScaleY, int Brightness );
	void DrawTacticalVisionMode( UViewport* Viewport, APlayerController* Owner, float Time, float ElapsedTime, float InvScaleX, float InvScaleY, bool bStarted );
	void DrawTimerRing( UViewport* pViewport, int ScreenX, int ScreenY, FColor EmptyColor, FColor FilledColor, float fRatioComplete, float fScale );
	bool GetMarkerIconUVs(ASquadMarker* pSquadMarker, UMaterial* pMaterial, float& U1, float& V1, float& U2, float& V2, int HUDIconX = -1, int HUDIconY = -1);

	void DrawIncap( float ElapsedTime, UViewport* Viewport, float InvScaleX, float InvScaleY, bool bIncapLastFrame );
	void DoIncapMenu( UViewport* Viewport, float InvScaleX, float InvScaleY, bool bIncapLastFrame );
	
	void DrawCheckpointSaveText( UViewport* Viewport, APlayerController* Owner, float ElapsedTime, float InvScaleX, float InvScaleY );
	
	// HUD Visor FX
	void DoHelmetFX(UViewport* Viewport,FRenderInterface* RI);
	void DrawHelmetFX(UViewport* Viewport,FRenderInterface* RI);
	void AddSplat( UMaterial* Decal, FVector Position, float Rotation, float WipeDelay, float Time );
	void DeleteSplatters();

	void StartScreenFade(float Time, const FColor& color);

	bool ShowSquadCommandPrompt(bool bShow);

	void ActorDestroyed(AActor* pActor);
	
	AActor* GetTargetedIconActor();

	void LoadSubtitles( const FString& LevelName );
	void LoadLevelSpecificSubtitles( const FString& LevelCompareName, 
									 int NumSubtitleGroups, 
									 int LastSharedIdx,
									 int& LastIdx );
	
	void PlaySound( AActor* Hud, USound* Sound, INT Flags = 0  );
	void StopSound( AActor* HUD, USound* Sound, FLOAT FadeOut = 0.f  );

	UTexture* GetKeyIcon( int key );

	void UpdateWeaponAmmo( UViewport* Viewport );
	
	int GetSquadPawnHUDPosition( APawn* pSquadPawn );

}

defaultproperties
{
     BlurHit=120
     WipeTime=2.2
     BloodWipeDelay=1.5
     HitWipeDelay=1.5
     LastHealthLevel=3
     HealthFlashDuration=0.5
     ShieldFlashDuration=0.5
     TargetHitBlipDuration=0.5
     AmmoSpinupTime=0.5
     HitLowHealth=20
     HitLargeDamage=100
     ShieldInitStart=3.25
     ShieldInitEnd=5.25
     ShieldInitText="CHARGING SHIELDS"
     HealthInitStart=1
     HealthInitEnd=3
     HealthInitText="SCANNING HEALTH"
     SquadInitStart=5.5
     SquadInitEnd=7.5
     SquadInitText="INIT SQUAD LINK"
     PromptFadeInTime=0.5
     CancelManeuverText="CANCEL MANEUVER"
     EngageTargetHealthText="TARGET HEALTH"
     IncapacitatedText="- INCAPACITATED -"
     SquadUnavailableText="- SQUAD UNAVAILABLE -"
     HealthCriticalText="HEALTH CRITICAL"
     FullHealthText="YOU ARE AT FULL HEALTH"
     TacticalVisionModeText="TACTICAL MODE"
     LowLightVisionModeText="LOW LIGHT MODE"
     IncapMenuMaintainText="MAINTAIN CURRENT ORDERS"
     IncapMenuRecallText="RECALL AND REVIVE"
     IncapMenuReloadPCText="RELOAD LAST SAVE GAME"
     IncapMenuReloadXboxText="RELOAD LAST AUTOSAVE"
     Checkpoint="SAVING CHECKPOINT"
     XboxCheckpointAddition="PLEASE DON'T TURN OFF YOUR XBOX CONSOLE"
     CheckpointSaveTime=3
     DemolitionReadyPromptText="PRESS @ TO DETONATE"
     DemolitionReadyPromptButtonFuncs(0)="Use | onrelease StopUse"
     MountTurretPromptText="PRESS @ TO MOUNT TURRET"
     MountTurretPromptButtonFuncs(0)="Use | onrelease StopUse"
     SquadOccupiedText="RECALL SQUAD BEFORE ENGAGING MANEUVER"
     SquadIncapacitatedText="REVIVE SQUAD BEFORE ENGAGING MANEUVER"
     ReviveSquadmatePromptText="PRESS AND HOLD @ TO REVIVE SQUADMATE"
     ReviveSquadmatePromptButtonFuncs(0)="Use | onrelease StopUse"
     PickupPromptText="PRESS @ TO PICKUP "
     PickupPromptButtonFuncs(0)="Use | onrelease StopUse"
     RemoveScavPromptText="PRESS @ TO REMOVE SCAVENGER DROID"
     RemoveScavPromptButtonFuncs(0)="Fire | onrelease StopFire"
     SearchAndDestroy="SEARCH AND DESTROY"
     FormUp="FORM UP"
     SecurePosition="SECURE AREA"
     Cancel="CANCEL MANEUVER"
     SquadCommandPromptDelay=1
     SquadCommandPromptFadeInTime=0.5
     SelectedSquadCommand=-1
     SelectedSquadCommandPulseTime=1
     bDraw=True
     bBracketVisibleObjective=True
     bDoInitSequence=True
     VisorModeTextTime=3
     VisCheckFrequency=0.25
     LargeVisorIcons=Texture'HUDTextures.Icons.LargeVisorIcons'
     CloneVisorIcons=Texture'HUDTextures.Icons.CloneVisorIcons'
     MPVisorIcons=Texture'HUDTextures.Icons.MPVisorIcons'
     TexAmmo=Texture'HUDTextures.Icons.HUD_Ammo'
     TexClip=Texture'HUDTextures.Icons.HUD_ClipFrame'
     TexHit=Texture'HUDTextures.Icons.HUD_Hit2'
     MarkerArrows=Texture'HUDTextures.Reticles.MarkerArrows'
     TexAButton=Texture'HUDTextures.Icons.AButton'
     TexBButton=Texture'HUDTextures.Icons.BButton'
     TexXButton=Texture'HUDTextures.Icons.XButton'
     TexYButton=Texture'HUDTextures.Icons.Ybutton'
     TexBlackButton=Texture'HUDTextures.Icons.Blackbutton'
     TexWhiteButton=Texture'HUDTextures.Icons.Whitebutton'
     TexDPadMonoButton=Texture'HUDTextures.Icons.Dpad_mono'
     TexCompassGraphic=Texture'HUDTextures.Icons.CompassGraphic'
     TexTargetBlip=Texture'HUDTextures.Reticles.HUD_Use'
     TexObjectiveRing=Texture'HUDTextures.Icons.ObjectiveRing'
     TexDPadBlue=Texture'HUDTextures.Icons.Dpad_blue'
     TexSniperRecharge=Texture'HUDTextures.Icons.SniperRecharge'
     TexSniperNonZoomReticle=Texture'HUDTextures.Reticles.HUD_WeaponSniperNonZoom'
     TexTextLine01=Texture'HUDTextures.Icons.TextLine01'
     TexText01=Texture'HUDTextures.Icons.Text01'
     TexMarkerArrows=Texture'HUDTextures.Reticles.MarkerArrows'
     TexHealRing=Texture'HUDTextures.Reticles.HUD_ReviveOutside'
     TexHealIndicator=Texture'HUDTextures.Reticles.HUD_ReviveInside'
     TexUseRing=Texture'HUDTextures.Reticles.HUD_UsePanelOutside'
     TexUseIndicator=Texture'HUDTextures.Reticles.HUD_UsePanelInside'
     TexTrandoRollReticle=Texture'HUDTextures.Reticles.HUD_TurretTrandoRoll'
     TexTacticalLine=Texture'HUDTextures.Icons.TacticalLine'
     TexTacticalGraphic=Texture'HUDTextures.Icons.TacticalGraphic'
     TexBowcasterClip=Texture'HUDTextures.Icons.HUD_BowcasterClip'
     TexBowcasterRecharge=Texture'HUDTextures.Icons.HUD_BowcasterRecharge'
     EventTimer=Texture'HUDTextures.Reticles.HUD_EventTimer'
     IncapMenuButton=Texture'HUDTextures.Icons.SniperRecharge'
     HSBloodSplatter=HardwareShader'FrameFX.VisorFX.BloodSplatter'
     HSHitEffect=HardwareShader'FrameFX.VisorFX.HitDamage'
     HSWiper=HardwareShader'FrameFX.VisorFX.Wiper'
     Orbit8Font=Font'OrbitFonts.OrbitBold8'
     Orbit15Font=Font'OrbitFonts.OrbitBold15'
}

