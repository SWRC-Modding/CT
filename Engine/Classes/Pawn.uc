//=============================================================================
// Pawn, the base class of all actors that can be controlled by players or AI.
//
// Pawns are the physical representations of players and creatures in a level.  
// Pawns have a mesh, collision, and physics.  Pawns can take damage, make sounds, 
// and hold weapons and other inventory.  In short, they are responsible for all 
// physical interaction between the player or AI and the world.
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Pawn extends Actor 
	abstract
	native
	dependsOn(PawnAudioTable)	
	placeable
	config(user)
	nativereplication
	hidecategories(Force,Sound);

#exec Texture Import File=Textures\Pawn.pcx Name=S_Pawn Mips=Off MASKED=1

// Determines which movement animations to play
enum EPawnMovementStyle
{
	MS_None,
	MS_Walk,
	MS_WalkWounded,
	MS_Run,
	MS_RunWounded,
	MS_Crouch,
	MS_Ladder,
	MS_Swim,
	MS_Fly,
};

const LayerAnimChannel = 3;
const DamageAnimChannel = 4;

// The state the AI starts in
enum EStartState
{
	SS_Idle,
	SS_Wander,
	SS_Perched,
	SS_PerchedCeiling,
	SS_Shutdown,
	SS_Asleep,
	SS_StationaryAttack,	
};

// The Alert state determines which idle animation to play
enum EIdleState
{
	AS_Shutdown,
	AS_Relaxed,
	AS_Alert,
	AS_Berserk,	
	AS_PerchedCeiling,
	AS_PerchedWall,
	AS_Possessed,
	AS_StandTurret,
	AS_SitTurret,
	AS_BackToWall,
};

// Used for characters with bHealthAffectsGameplay=true
enum EHealthLevel
{
	HL_Red,
	HL_Orange,
	HL_Yellow,
	HL_Green,
};

enum EPatrolMode
{
    PM_Loop,
	PM_PingPong,
	PM_Random,
	PM_Once,
};

struct PatrolPoint
{
	var() Actor		Node;
    var() float		PauseTime;
	var() name		Animation;
	var() int		NumAnimLoops;
	var() name		Event;
	var() bool		RunToNode;
	var() bool		ShootWhileMoving;
	var() bool		OrientToNode;
	var() bool		MakeInvulnerable;
	var() float		PatrolPriorityOverride;
};

struct DamageMultiplier
{
	var() name		BoneName;
	var() float		Multiplier;	
};

struct AccessoryInfo
{
	var bool				HasBeenUsed;
	var() bool				IsRequired;	
	var() class<Accessory>	AccessoryClass;
	var() name				BoneName;
};

struct DismembermentInfo
{
	var() name				Bone;
	var() float				SpawnProbability;
	var() class<Accessory>	Part;
	var() class<Emitter>	DismembermentEffectClass;
	var() bool				DisablePartialDismemberment;
};

struct FinishingMove
{
	var() name			EnemyClass;
	var() name			Animation;
	var() EDirection	DirectionFromEnemy;
	var() vector		OffsetFromEnemy;
};

struct ImpactAnim
{
	var() name			BoneName;
	var() name			Animation;	
};

///////////////////////////////////////////////////////////////////////////////
// FLAGS
///////////////////////////////////////////////////////////////////////////////

// Movement State flags
var bool		bIsWalking;				// currently walking (can't jump, affects animations)
var bool		bWantsToCrouch;			// if true crouched (physics will automatically reduce collision height to CrouchHeight)
var const bool	bIsCrouched;			// set by physics to specify that pawn is currently crouched
var const bool	bTryToUncrouch;			// when auto-crouch during movement, continually try to uncrouch
var bool		bJustLanded;			// used by eyeheight adjustment
var bool		bWarping;				// Set when travelling through warpzone (so shouldn't telefrag)
var bool		bUpAndOut;				// used by swimming 
var bool		bCrawler;				// crawling - pitch and roll based on surface pawn is on
var	bool		bRollToDesired;			// Update roll when turning to desired rotation (normally false)
var bool		bPhysicsAnimUpdate;		// Use physics to drive movement animations
var bool		bWasCrouched;			// Stores the old crouch state (used in UnPawn::UpdateMovementAnimation())
var bool		bWasWalking;			// Stores the old walk state (used in UnPawn::UpdateMovementAnimation())
var bool		bWasOnGround;			// Used to determine when to Jump
var bool		bPlayedDeath;			// Determines if we have played our death animation or gone into ragdoll yet
var bool		bCannotDie;				// if the character gets incapacitated rather than dying
var bool		bIncapacitated;			// currently incapacitated
var bool		bFallOnDeath;			// Enter PHYS_Falling when killed
var bool		bHasFullFlyAnims;		// This character has the full suite of directional flying animations
var bool		bDisallowPainAnims;		
var bool		bIncapacitatedOnTurret;	// The character was incapacitated while on a turret
var bool		bIgnoreFriendlyFire;	// Whether anybody should care if we're a victim of friendly fire

// Movement Capabilities
var bool		bCanJump;				// can the pawn jump
var	bool		bCanLedgeFall;			// can the pawn fall/walk off ledges
var bool		bCanCrouch;				// if true, this pawn is capable of crouching
var	bool 		bCanWalk;				// can walk
var	bool 		bCanRun;				// can run
var	bool		bCanSwim;				// can swim
var	bool		bCanFly;				// can use fly path nodes (not currently used effectively)
var	bool		bCanClimbLadders;		// can use ladders
var	bool		bCanStrafe;				// can strafe
var bool		bCanDodge;				// can do a sideways dodge move
var bool		bCanSideStep;			// can step to the side
var bool		bCanDive;				// can dive forward
var bool		bCanPerch;				// can attack to walls
var bool		bCanLean;				// can lean around corners
var bool		bMustStopToTurn;		// cannot turn while moving
var bool		bDisableFireAnims;		// prevent weapon fire anims from playing
var bool		bSaveMyCorpse;			// save this corpse for a short while
// Ledge flags
var	bool		bAvoidLedges;			// don't get too close to ledges
var	bool		bStopAtLedges;			// if bAvoidLedges and bStopAtLedges, Pawn doesn't try to walk along the edge at all
var	bool		bCanWalkOffLedges;		// Can still fall off ledges, even when walking (for Player Controlled pawns)
var bool		bDismembered;			// This pawn was dismembered when it died
// Jump Flags
var	bool		bNoJumpAdjust;			// set to tell controller not to modify velocity of a jump/fall	

// Collision and Physics Flags
var	bool		bIgnoreForces;			// if true, not affected by external forces
var const bool	bNoVelocityUpdate;		// used by C++ physics
var bool		bClientCollision;		// used on clients when temporarily turning off collision
var bool		bCollideAfterDeath;		// actor retains collision after being killed

// Networking flags
var const bool	bSimulateGravity;		// simulate gravity for this pawn on network clients when predicting position (true if pawn is walking or falling)
var const bool	bSimGravityDisabled;	// used on network clients
var	bool		bCachedRelevant;		// network relevancy caching flag
var	bool		bUseCompressedPosition;	// use compressed position in networking - true unless want to replicate roll, or very high velocities

// AI Related Flags
var(AI) bool		bIsSquadLeader;			// only used as startup property
var		bool		HealthAffectsGameplay;	// weather the health state affects animations, accuracy, and AI
var(AI)	bool		bDebugAI;				// whether we log AI info to logfile
var(AI) bool		bDontPossess;			// if true, Pawn won't be possessed at game start
var(AI) bool		bCanBeSquadMember;		// if this character can join a squad
var(AI) bool		bTossWeaponOnDeath;		// Hey, toss your weapon when you're dead!
var		bool		bDecelerateOnArrival;	// whether we should decelerate to zero speed on arrival
var		bool		bUseAnalogSpeed;		// use analog desired speed to determine running, walking
var		bool		bUseSpeedScaler;		// use the Analog Speed as a scale factor instead of an absolute speed
var		bool		bBeingRevivedByPlayer;	// whether this character is being revived by the player
var		bool		bPerceivedAsThreat;		// if this object is threatening, regardless of TeamIndex
var		bool		bNeverMeleeMe;			// AIs should not try to melee attack this Pawn (could move up to Actor)
var		bool		bSquadShouldFallBack;

// Hearing flags
var(AI) static bool	bLOSHearing;			// can hear sounds from line-of-sight sources (which are close enough to hear) bLOSHearing=true is like UT/Unreal hearing										
var(AI) static bool	bSameZoneHearing;		// can hear any sound in same zone (if close enough to hear)
var(AI) static bool	bAdjacentZoneHearing;	// can hear any sound in adjacent zone (if close enough to hear)
var(AI) static bool	bMuffledHearing;		// can hear sounds through walls (but muffled - sound distance increased to double plus 4x the distance through walls
var(AI) static bool	bAroundCornerHearing;	// Hear sounds around one corner (slightly more expensive, and bLOSHearing must also be true)
//var		bool	bAttackImmediateThreatsOnly;	// Attack only those who are visible or shooting at us
var(AI)	bool		bOnPathFailNoCollision;
var(AI)	bool		bOnPathFailTeleport;
//var(AI)	bool		bOnPathFailCheatWhenSeen;
var(AI) static bool	bCanPerformSquadMarkers;


// HUD flags
var		bool	bUpdateEyeheight;		// if true, UpdateEyeheight will get called every tick
var		bool	bUpdatingDisplay;		// to avoid infinite recursion through inventory setdisplay
var     bool    bHideRegularHUD;
var		bool	bDidDamageThisFrame;	// If true, the damage hit indicator will flash around the reticle
var		bool	bSetDamageThisFrame;	// If true, we've set bDidDamageThisFrame this frame

// Pickup, Inventory, and Powerup flags
var		globalconfig bool bWeaponBob;	// Should the weapon bob?
var		bool	bCanPickupInventory;	// if true, will pickup inventory when touching pickup actors
var		bool	bCanSpecialPickup;      // if trye, will pickup bSpecialPickup's
var()	bool	bNoDefaultInventory;	// don't spawn default inventory for this guy
var		bool	bAutoActivate;			// Automatically activate picked up powerups

// Visual AI Debugging Flags
var		bool	bShowFOV;				//draws the actor's FOV
var		bool	bShowHearing;			//Draws green line to the MoveTarget or, if that is None, to the Destination
var		bool	bShowPathing;			//draws information about the actor's desired pathing
var		bool	bShowEnemy;				//Draws a red line to the Enemy, and an orange one to the Target
var		bool	bShowFocus;				//Draws a blue line to the Focus or, if that is None, to the FocalPoint
var		bool	bShowMoveDest;			//Draws green line to the MoveTarget or, if that is None, to the Destination
var		bool	bShowStateTint;			//Draws a orange line to the Target

// Misc Flags
var		bool	bCanBeBaseForPawns;		// allows other pawns to attach to this pawn
var		bool	bDirectHitWall;			// always call pawn hitwall directly (no controller notifyhitwall)
var		bool	bSoakDebug;				// use less verbose version of debug display
var		bool	bIsTrandoshan;			// Used in multiplayer

// Turn focus flags.
var		bool	bEnableTurn;			// enable auto-turning code.
										// bRotateToDesired enables exact rotation.
var		bool	bEnableAim;				// enable upper body auto-aiming code.
var		bool	bEnableHead;			// enable head look
var		bool	bHas180Turns;			// Does this character have 180 degree turn anims
var		bool	bWasHit;
var		bool	bWasHitWasSent;
///////////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////////

// Shields and Health
var		travel float							Health;						// Counter for Health
var()	static float							MaxHealth;					// Health: 100 = normal maximum
var		float									Shields;					// Counter for Shields
var()	static float							MaxShields;					// Max/Starting Shields
var()	float									ShieldRechargeRate;
var()	static float							ShieldRechargeDelay;
var		EHealthLevel							HealthLevel;
var		float									NextShieldRechargeTime;
var()	array<DamageMultiplier>					DamageMultipliers;			// Array of BoneName/Float damage multiplier pairs
var()	static class<DamageVariance>			ShieldDamageVariance;		// This variance is active when shields are on
var()	float									ArmorFactor;				// Reduces damage by 1-factor
var()	float									BleedOutTime;				// How long we can be incapacitated before dying
var()	float									MinImpactDamage;			// Minimum amount of damage required to play an impact anim
var()	float									MinImpactInterval;			// Minimum amount of time between impact anims
var		float									LastImpactTime;				// Last time we played an impact animation
var()	float									MinStaggerDamage;			// Minimum amount of damage required to play a stagger anim
var()	float									MinStaggerInterval;			// Minimum amount of time between stagger anims
var		float									LastStaggerTime;			// Last time we played a twitch/impact animation
var()	array<ImpactAnim>						ImpactAnims;				// List of Bone/Anim pairs for impact anims

var(AI)	static class<Weapon>					BestGrenadeAgainst;			// The best type of grenade to use against this character


// Weapons and Inventory
var		Weapon				PendingWeapon;			// Will become weapon once current weapon is put down
var		travel Weapon		Weapon;					// The pawn's current weapon.
var		travel Weapon		OldWeapon;				// Player's last weapon that was last active - still in the inventory - not tossed
var		Weapon				CurrentTossableWeapon;	// Player's current weapon that can be tossed - don't travel because it'll get set in GiveTo()
var		travel Weapon		CurrentGrenade;			// The currently active grenade
var		travel Powerups		SelectedItem;			// currently selected inventory item
var()	autoload string		RequiredEquipment[16];	// allow L.D. to modify for single player
var		string				ActionString;			// used by the HUD to display what the AI is currently doing
var		Flashlight			Flashlight;
var()	float				DropGrenadePercentage;	// percentage an AI will toss its grenades when it dies
var		class<Projectile>	ReturnGrenadeClass;		// Class of grenade to throw back

// Accessories
var()	array<AccessoryInfo>	AvailableAccessories;
var()	int						MaxAccessories;
var		array<Accessory>		Accessories;

// Death and Dismemberment
var() class<Emitter>				DeathEffect;

var() array<DismembermentInfo>		DismembermentParts;
var() float							DismembermentThreshold;
var() class<DeathEffectContainer>	DeathEffectContainerClass;

var() Material						BurntMaterial;

var() array<FinishingMove>			FinishingMoves;
var() array<Info.MeshSetLinkup>		MeshSets;

// Visor FX
var		Pawn				EnemyKilledThisFrame;		// Pawn that was killed this frame
var()	array<Material>		VisorBloodSplatters;
var		int					CurrentUserVisionMode;		// Current vision mode user has selected
var()	VisionMode			VisionPawnUser[3];			// Array of vision modes user can choose
var()	VisionMode			VisionPawnHit;
var()	VisionMode			VisionPawnIncap;
var()	VisionMode			VisionPawnScavDrill;
var()	Color				ShieldColor;				// Color when pawns shield is hit

// Perception
var(AI) float				HearingThreshold;		// max distance at which a makenoise(1.0) loudness sound can be heard
var(AI)	float				SightRadius;			// Maximum seeing distance.
var(AI)	float				PeripheralVision;		// Cosine of limits of peripheral vision.
var(AI) float				AutoDetectRadius;		// Radius within enemies can be detected even if not sighted
var		float		   		BaseEyeHeight; 			// Base eye height above collision center.
var		float	        	EyeHeight;     			// Current eye height, adjusted for bobbing and stairs.
var		float				OldZ;					// Old Z Location - used for eyeheight smoothing

// AI
var		Controller					Controller;				// Current controller, generally same actor as Owner
var		static class<AIController>	ControllerClass;		// default class to use when pawn is controlled by AI (can be modified by an AIScript)
var(AI) EStartState					StartState;				// which state should you start in
var(AI) EPatrolMode					PatrolMode;				// Way in which to traverse the list of patrol points
var 	byte						Visibility;				// How visible is the pawn? 0=invisible, 128=normal, 255=highly visible
var		byte						SquadHUDSlot;			// The slot for this guy's HUD icon
var(AI) float						PatrolPriority;			// Priority for Patroling
var(AI)	array<PatrolPoint>			PatrolRoute;			// List of points to walk when on patrol
var(AI) name						AIScriptTag;			// tag of AIScript which should be associated with this pawn
var(AI)	int							TeamIndex;				// the current team index for this pawn
var		int							TargetedByCount;		// the number of controllers who have us their Enemy
var		Squad						Squad;					// Which squad are you a part of
var(AI) static float				Accuracy;				// How accurate you are (from 0 to 1.0)
var		Vector						MovementOffsetDir;		// A world-coordinate offset to use when moving to nav points
var		float						AnalogSpeed;			// A field for a desired analog speed (btwn running and walking)
var		float						MaximumDesiredSpeed;	// The maximum desired speed the Pawn should go (allows smooth deceleration)
var		vector						TakeHitLocation;		// location of last hit (for playing hit/death anims)
var		class<DamageType>			HitDamageType;			// damage type of last hit (for playing hit/death anims)
var		class<DamageType>			LastHitDamageType;		// Last damage type
var		Pawn						LastHitBy;				// Who damaged me last
var		float						LastHitLocalTime;		// Time hit on the local machine, not replicated!
var		Vector						LastHitLocation;		// World position of last shield hit 
var		name						LastHitBone;

var()	float						MaxDeathLoopTime;
var		float						DeathLoopStartTime;
var		static float				GunHeight;				// Height of gun barrel when standing
var		float						LastDodgeTime;			
var		vector						PreDodgeLocation;
var(Events)	name					FirstDamagedEvent;		// Event to play when first damaged

// Pathing
var		NavigationPoint		Anchor;					// current nearest path;
var		ReachSpec			CurrentLink;			// the last link we were following
var		vector				CurrentLinkDir;
var	const NavigationPoint	LastAnchor;				// recent nearest path
var		float				FindAnchorFailedTime;	// last time a FindPath() attempt failed to find an anchor.
var		float				LastValidAnchorTime;	// last time a valid anchor was found
var		float				DestinationOffset;		// used to vary destination over NavigationPoints
//var		float				NextPathRadius;			// radius of next path in route
var		const float			AvgPhysicsTime;			// Physics updating time monitoring (for AI monitoring reaching destinations)
//var		vector				SerpentineDir;			// serpentine direction
//var		float				SerpentineDist;			// how far to go before strafing again
//var		float				SerpentineTime;			// how long to stay straight before strafing again
var			vector				AvoidanceDir;			// the direction we last chose for avoidance
var			float				NextAvoidanceTime;		// the next time we can choose a new avoidance direction

// Player info.
var	string					OwnerName;				// Name of owning player (for save games, coop)
var PlayerReplicationInfo	PlayerReplicationInfo;
var name					LandMovementState;		// PlayerControllerState to use when moving on land or air
var name					WaterMovementState;		// PlayerControllerState to use when moving in water

// Physics and Collision
var	vector	Floor;					// Normal of floor pawn is standing on (only used by PHYS_Spider and PHYS_Walking)
var static float	CrouchHeight;			// CollisionHeight when crouching
var static float	CrouchRadius;			// CollisionRadius when crouching
var static float	PerchRadius;			// The Radius and Half-height of actors when clinging to walls

// Movement
var() float			NextFootstepTime;		// The next time we should generate a footstep stimulus
var static float	StrafeDistance;			// How far you can strafe right or left
var static float	StepDistance;			// How far you can step right or left
var static float	DodgeDistance;			// How far you can dodge right or left
var static float	LeapDistance;			// how far you can leap forward
var static float	LeanDistance;			// how far you can lean to the side
var static int		FlyingRollMult;			// Tweak to make flying look more dramatic

// Movement Speeds
var float			GroundSpeed;				// The maximum ground speed.
var float			WaterSpeed;					// The maximum swimming speed.
var float			AirSpeed;					// The maximum flying speed.
var float			LadderSpeed;				// Ladder climbing speed
var float			MaxFallSpeed;				// max speed pawn can land without taking damage (also limits what paths AI can use)
var static float	AccelRate;					// max acceleration rate
var float			JumpZ;      				// vertical acceleration w/ jump
var float			AirControl;					// amount of AirControl available to the pawn
var static float	WalkSpeedRatio;				// ratio of walking:running speed
var static float	CrouchSpeedRatio;			// ratio of crouched:running speed
var static float	WoundedSpeedRatio;			// ratio of wounded:normal speed
var static float	BackSpeedRatio;				// ratio of back:forward speed
var static float	SideSpeedRatio;				// ratio of side:forward speed
var static float	AimSpeed;					// how fast the upper body tracks to it's target
var static float	DecelRate;					// how fast we slow down
var static float	MinCorrectedSpeed;			// minimum speed for throttling
var static float	MaxAnalogOverdriveRatio;	// maximum overdrive speed multiplier when using an analog throttle

var static EStimulusType	RunStimulus;		// the type of stimulus running generates
var static EStimulusType	WalkStimulus;		// the type of stimulus walking or crouching generates

// Animation Variables
var EPhysics				OldPhysics;					// used in Physics based animation
var EPawnMovementStyle		CurrentMovementStyle;		// determines which movement animations to play
var EIdleState				CurrentIdleState;			// determines which breathe and run to play
var EIdleState				OldIdleState;				// the previous alert state
var float					OldRotYaw;					// used for determining if pawn is turning
var vector					OldAcceleration;			// used in Physics based animation
var name					MovementAnim;				// forward anim (back/left/right variants deduced)
var name					TurnLeftAnim;				// turning anims when standing in place (scaled by turn speed)
var name					TurnRightAnim;				// turning anims when standing in place (scaled by turn speed)
var name					BreatheWeaponAnim;			// weapon-specific breathe animation
var float					MovementBlendStartTime;		// used for delaying the start of run blending
var() static float			MaxAimYaw;					// in degrees, how far you can aim your upper body
var() static float			MaxAimPitch;				// in degress, units, how far you can aim your upper body
var() static float			MaxLookYaw;					// in degress, units, how far you can turn your head
var() static float			MaxLookPitch;				// in degress, units, how far you can turn your head

// Sound
var		class<PawnAudioTable>	AudioTable;
var		float					EndTalkTime;				// Time that the current/last voice line will finish playing
var()	float					MinYelpTime;				// minimum time between hurt sounds
var		float					LastYelpTime;

var()	float					MinShieldDamageSoundInterval;
var		float					LastShieldDamageSoundTime;
var()	sound					ShieldDamageSound;			// Sound to play when shield takes damage
var		PawnAudioTable.EPawnAudioEvent	  MostRecentCue;
var static PawnAudioTable.EPawnAudioEvent EnemySpottedCue;
var static PawnAudioTable.EPawnAudioEvent KilledEnemyCue;
var static PawnAudioTable.EPawnAudioEvent AllyDownCue;
var static PawnAudioTable.EPawnAudioEvent DownedByCue;
var static PawnAudioTable.EPawnAudioEvent AssistingAllyCue;

//var float	SoundDampening;								// Automatically dampens all sounds played by this pawn
var sound	Land;

// Ragdoll impact sounds.
var(Karma)		array<sound>	RagImpactSounds;
var(Karma)		float			RagImpactSoundInterval;
var(Karma)		float			RagImpactVolume;
var transient	float			RagLastSoundTime;

//KARMA stuff
var(Karma) static float RagdollLifeSpan;	// MAXIMUM time the ragdoll will be around. De-res's early if it comes to rest.
var(Karma) string		RagdollOverride;	// Name of the Ragdoll asset to use when entering ragdoll

// Networking
var		name							AnimAction;			// use for replicating anims 
var		float							LastStartTime;		// the last spawn time, only used in PlayerController::Suicide()
var		transient CompressedPosition	PawnPosition;		// Used if bUseCompressed position is true
var		float							NetRelevancyTime;
var		playerController				LastRealViewer;
var		actor							LastViewer;
var     PlayerStart						LastStartSpot;		// used to avoid spawn camping in MP

var const float LastLocTime;		// used to force periodic location replication, even when pawn isn't moving

// Water
var		float		SplashTime;				// time of last splash


// View bob (seems like all this belongs in the PlayerController...)
var		globalconfig float	Bob;
var		float				LandBob;
var		float				AppliedBob;
var		float				BobTime;
var		vector				WalkBob;

// Misc
var		const float		UncrouchTime;		// when auto-crouch during movement, continually try to uncrouch once this decrements to zero
var		vector			TearOffMomentum;	// momentum to apply when torn off (bTearOff == true)
var		Projector		Shadow;				// Shadow projector
var		PhysicsVolume	HeadVolume;			// physics volume of head
var		float	        SpawnTime;			// Added back in
var		float			DamageScaling;

// Travel
//var		travel SquadTravelInfo	SquadInfo[3];	// keep track of the squad's health for travelling purposes

replication
{
// NOTE: THIS CLASS IS NATIVE REPLICATED; MAKING CHANGES HERE FOR VARIABLES WILL HAVE NO EFFECT

	// Variables the server should send to the client.
	reliable if( bNetDirty && (Role==ROLE_Authority) )
        bSimulateGravity, bIsCrouched, bIsWalking, bIncapacitated, PlayerReplicationInfo, AnimAction, HitDamageType, TakeHitLocation, HealthLevel;
	reliable if( bNetDirty && (Role==ROLE_Authority) )
		bWasHit;
	reliable if( bTearOff && bNetDirty && (Role==ROLE_Authority) )
		TearOffMomentum;	
	reliable if( bNetDirty && bNetOwner && Role==ROLE_Authority )
         Controller,SelectedItem, GroundSpeed, WaterSpeed, AirSpeed, AccelRate, JumpZ, AirControl, /*CurrentTossableWeapon, */CurrentGrenade, LastHitBy;
	reliable if( bNetDirty && Role==ROLE_Authority )
         Health, Shields, TeamIndex;
    unreliable if ( !bNetOwner && bNetDirty &&Role==ROLE_Authority )
		PawnPosition;
	reliable if ( Role==ROLE_Authority )
		PlayHurtCue, SwitchToLastWeapon;
	unreliable if ( bNetDirty && Role==ROLE_Authority )
		bDidDamageThisFrame;

	// replicated functions sent to server by owning client
	reliable if( Role<ROLE_Authority )
		ServerChangedWeapon;

}

native event SetAnchor(NavigationPoint Point);
native function bool ReachedDestination(Actor Goal);
native function ForceCrouch();
//native final function PlayCue(PawnAudioTable.EPawnAudioEvent Cue, optional bool NonPositional);
native final function PlayOwnedCue(PawnAudioTable.EPawnAudioEvent Cue, optional float MinTriggerInterval);
native final function Sound GetSoundFromCue(PawnAudioTable.EPawnAudioEvent Cue, optional float MinTriggerInterval);

native final function PlayJumpSoundOnCurrentMaterial();
native event StopTalking();

native function bool GetConfigValue(string section, string key, out string value, optional string file );

// TeamFunctions
native simulated function int GetTeamIndex();
native event SetTeamIndex(int index);
native simulated function TeamInfo.ETeamRelationship GetRelationTowards(Pawn Other);

native simulated function EHealthLevel CalcHealthLevel(float HealthRatio);

function HookupReviveMarker();
function RemoveReviveMarker();

// Travel
event SaveSquadTravelInfo()
{
}


simulated function PlayerController GetRecentController()
{
	return PlayerController(Controller);
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	if ( (Controller == None) || Controller.bIsPlayer )
		Destroy();
	else
		Super.Reset();
}

function StopFire( optional float F )
{
    if( Weapon!=None )
        Weapon.StopFire(F);
}

function Fire( optional float F )
{
    if( Weapon!=None )
        Weapon.Fire(F);
}

simulated function String GetHumanReadableName()
{
	if ( PlayerReplicationInfo != None )
		return PlayerReplicationInfo.GetPlayerName();
	else
		return GetItemName(string(class));
}

function PlayTeleportEffect(bool bOut, bool bSound)
{
	//MakeNoise(1.0); //Add a stimulus?
}

function NotifyTargeted( PlayerController Player )
{
	Super.NotifyTargeted( Player );

	if( Controller != None )
	{
		if( Player.Pawn.Flashlight != None && Player.Pawn.Flashlight.bIsOn )
			Player.Pawn.PostStimulusToIndividual( ST_Flashlight, Controller );
		else
			Player.Pawn.PostStimulusToIndividual( ST_Targeted, Controller );
	}
}

/* PossessedBy()
 Pawn is possessed by Controller
*/
function PossessedBy(Controller C)
{
	Controller = C;
	NetPriority = 3;

	if ( C.PlayerReplicationInfo != None )
	{
		PlayerReplicationInfo = C.PlayerReplicationInfo;
		OwnerName = PlayerReplicationInfo.PlayerName;
	}
	if ( C.IsA('PlayerController') )
	{
		if ( Level.NetMode != NM_Standalone )
			RemoteRole = ROLE_AutonomousProxy;
		BecomeViewTarget();
	}
	else
		RemoteRole = Default.RemoteRole;

	SetOwner(Controller);	// for network replication
	Eyeheight = BaseEyeHeight;
	ChangeAnimation();
}

function UnPossessed()
{		
	PlayerReplicationInfo = None;
	SetOwner(None);
	Controller = None;
}

function BecomeViewTarget()
{
	bUpdateEyeHeight = true;
}

function DropToGround()
{
	bCollideWorld = true;
	bInterpolating = false;
	if ( Health > 0 )
	{
		SetCollision(true,true,true);
		SetPhysics(PHYS_Falling);
		AmbientSound = None;
		if ( IsHumanControlled() )
			Controller.GotoState(LandMovementState);
	}
}

event SetWalking(bool bNewIsWalking)
{
	// If we are wounded we can only walk
	//if (UseWoundedAnims())
	//	bNewIsWalking = true;	

	if ( bNewIsWalking != bIsWalking )
	{
		bIsWalking = !bCanRun || bNewIsWalking;
		ChangeAnimation();
	}
}

function bool CanSplash()
{
	if ( (Level.TimeSeconds - SplashTime > 0.25)
		&& ((Physics == PHYS_Falling) || (Physics == PHYS_Flying))
		&& (Abs(Velocity.Z) > 100) )
	{
		SplashTime = Level.TimeSeconds;
		return true;
	}
	return false;
}

function bool ShouldTossWeapon(Weapon NextWeapon)
{
	local int NumAllowedInInvGrp, NumCurrentlyInInvGrp, NextWeaponInvGrp;
	local Inventory CurrentInv;

	if ( IsPlayerPawn() )
	{
		if ( Level.NetMode != NM_Standalone && Health <= 0 )
			return bTossWeaponOnDeath;
		if ( NextWeapon != None && NextWeapon.WeaponType != WT_Thrown )
		{
			NextWeaponInvGrp = Level.GRI.WeaponInventoryGroup(NextWeapon);
			NumAllowedInInvGrp = Level.Game.GetNumAllowedInGroup(NextWeapon);
			CurrentInv = Inventory;

			while ( CurrentInv != None )
			{
				if (CurrentInv.IsA('Weapon'))
				{
					if (Level.GRI.WeaponInventoryGroup(Weapon(CurrentInv)) == NextWeaponInvGrp)
						NumCurrentlyInInvGrp++;
				}
				CurrentInv = CurrentInv.Inventory;
			}
			if (NumCurrentlyInInvGrp >= NumAllowedInInvGrp)
				return true;
		}
		return false;
	}
	// NPC died - toss weapon
	else if (Health <= 0)
		return bTossWeaponOnDeath;
}

/* DisplayDebug()
list important actor variable on canvas.  Also show the pawn's controller and weapon info
*/
simulated function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	local string T;
	Super.DisplayDebug(Canvas, YL, YPos);

	Canvas.SetDrawColor(255,255,255);

	Canvas.DrawText("Animation Action "$AnimAction$" Health "$Health);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	T = "Floor "$Floor$" Crouched "$bIsCrouched$" Try to uncrouch "$UncrouchTime;
	Canvas.DrawText(T);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("EyeHeight "$Eyeheight$" BaseEyeHeight "$BaseEyeHeight$" Physics Anim "$bPhysicsAnimUpdate);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	if ( Controller == None )
	{
		Canvas.SetDrawColor(255,0,0);
		Canvas.DrawText("NO CONTROLLER");
		YPos += YL;
		Canvas.SetPos(4,YPos);
	}
	else
		Controller.DisplayDebug(Canvas,YL,YPos);

	if ( Weapon == None )
	{
		Canvas.SetDrawColor(0,255,0);
		Canvas.DrawText("NO WEAPON");
		YPos += YL;
		Canvas.SetPos(4,YPos);
	}
	else
		Weapon.DisplayDebug(Canvas,YL,YPos);
}
		 		
//
// Compute offset for drawing an inventory item.
//
simulated function vector CalcDrawOffset(inventory Inv)
{
	local vector DrawOffset;

	if ( Controller == None )
		return (Inv.PlayerViewOffset >> Rotation) + BaseEyeHeight * vect(0,0,1);

	DrawOffset = ((0.9/Weapon.DisplayFOV * 100 * ModifiedPlayerViewOffset(Inv)) >> GetViewRotation() ); 
	if ( !IsLocallyControlled() )
		DrawOffset.Z += BaseEyeHeight;
	else
	{	
		DrawOffset.Z += EyeHeight;
        if( bWeaponBob )
			DrawOffset += WeaponBob(Inv.BobDamping);
	}
	return DrawOffset;
}

function vector ModifiedPlayerViewOffset(inventory Inv)
{
	return Inv.PlayerViewOffset;
}

function vector WeaponBob(float BobDamping)
{
	Local Vector WBob;

	WBob = BobDamping * WalkBob;
	WBob.Z = (0.45 + 0.55 * BobDamping) * WalkBob.Z;
	return WBob;
}

function CheckBob(float DeltaTime, vector Y)
{
	local float Speed2D;

    if( !bWeaponBob )
    {
		BobTime = 0;
		WalkBob = Vect(0,0,0);
        return;
    }
	Bob = FClamp(Bob, -0.01, 0.01);
	if (Physics == PHYS_Walking )
	{
		Speed2D = VSize(Velocity);
		if ( Speed2D < 10 )
			BobTime += 0.2 * DeltaTime;
		else
			BobTime += DeltaTime * (0.3 + 0.7 * Speed2D/GroundSpeed);
		WalkBob = Y * Bob * Speed2D * sin(8 * BobTime);
		AppliedBob = AppliedBob * (1 - FMin(1, 16 * deltatime));
		WalkBob.Z = AppliedBob;
		if ( Speed2D > 10 )
			WalkBob.Z = WalkBob.Z + 0.75 * Bob * Speed2D * sin(16 * BobTime);
		if ( LandBob > 0.01 )
		{
			AppliedBob += FMin(1, 16 * deltatime) * LandBob;
			LandBob *= (1 - 8*Deltatime);
		}
	}
	else if ( Physics == PHYS_Swimming )
	{
		Speed2D = Sqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y);
		WalkBob = Y * Bob *  0.5 * Speed2D * sin(4.0 * Level.TimeSeconds);
		WalkBob.Z = Bob * 1.5 * Speed2D * sin(8.0 * Level.TimeSeconds);
	}
	else
	{
		BobTime = 0;
		WalkBob = WalkBob * (1 - FMin(1, 8 * deltatime));
	}
}
	
//***************************************
// Interface to Pawn's Controller

// return true if controlled by a Player (AI or human)
simulated function bool IsPlayerPawn()
{
	return ( (Controller != None) && Controller.bIsPlayer );
}

// return true if was controlled by a Player (AI or human)
simulated function bool WasPlayerPawn()
{
	return IsA('PlayerCommando');
}

// return true if controlled by a real live human
simulated function bool IsHumanControlled()
{
	return ( PlayerController(Controller) != None );
}

simulated function rotator GetViewRotation()
{
	if ( Controller == None )
		return Rotation;
	return Controller.GetViewRotation();
}

simulated function SetViewRotation(rotator NewRotation )
{
	//It is illegal to set rotations on non-player-controllers
	if ( PlayerController(Controller) != None )
		Controller.SetRotation(NewRotation);
}

final function bool InGodMode()
{
	return ( (Controller != None) && Controller.bGodMode );
}

function bool NearMoveTarget()
{
	if ( (Controller == None) || (Controller.MoveTarget == None) )
		return false;

	return ReachedDestination(Controller.MoveTarget);
}

simulated final function bool PressingFire()
{
	return ( (Controller != None) && (Controller.bFire != 0) && !Controller.IsInState('GameEnded') );
}

simulated final function bool PressingAltFire()
{
	return ( (Controller != None) && (Controller.bAltFire != 0) );
}

function Actor GetMoveTarget()
{	
	if ( Controller == None )
		return None;

	return Controller.MoveTarget;
}

function SetMoveTarget(Actor NewTarget )
{
	if ( Controller != None )
		Controller.MoveTarget = NewTarget;
}

function bool LineOfSightTo(actor Other)
{
	return ( (Controller != None) && Controller.LineOfSightTo(Other) );
} 

simulated final function rotator AdjustAim(Ammunition FiredAmmunition, vector ProjStart, float AimError, optional out int ShouldFire)
{
	if ( Controller == None )
		return Rotation;

	return Controller.AdjustAim(FiredAmmunition, ProjStart, AimError, ShouldFire);
}

function Actor ShootSpecial(Actor A)
{
	if ( !Controller.bCanDoSpecial || (Weapon == None) )
		return None;

	Controller.FireWeaponAt(A);
	Controller.bFire = 0;
	return A;
}

function HandlePickup(Pickup pick)
{	
	if ( Controller != None )
		Controller.HandlePickup(pick);
}

function ReceiveLocalizedMessage( class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	if ( PlayerController(Controller) != None )
		PlayerController(Controller).ReceiveLocalizedMessage( Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );
}

event ClientMessage( coerce string S, optional Name Type )
{
	if ( PlayerController(Controller) != None )
		PlayerController(Controller).ClientMessage( S, Type );
}

function Trigger( actor Other, pawn EventInstigator )
{
	// If we have the same tag as the controller, there's no reason to trigger it directly
	if ( Controller != None && Tag != Controller.Tag ) 
		Controller.Trigger(Other, EventInstigator);
}

function ToggleFlashlight()
{
	if( Flashlight == None )
		return;
	SetFlashlight( !Flashlight.bIsOn );
}

function SetFlashlight(bool bOn)
{
	local PlayerController Player;

	if( Flashlight == None )
		return;

    if( !bOn )
	{
		Flashlight.TurnOff();
		if( Controller.IsA('PlayerController') )
		{
			Player = PlayerController(Controller);

			if( Player.myHud.FlashlightProjector != None )
			{
				Player.myHud.FlashlightProjector.ProjTexture = None;
				Player.myHud.FlashlightProjector.Disable('Tick');
				// NathanM: Enable the following line when the projector is used on actors
				//PlayerController(Controller).myHud.FlashlightProjector.SetCollision(false,false,false);
			}

			if( Player.PlayerSpotLight != None )
			{
				Player.PlayerSpotLight.LightType = LT_None;
				Player.PlayerSpotLight.bLightPriorityOverride = false;
			}
			PlaySound( Player.myHud.FlashlightOff );
		}
	}
	else
	{
		Flashlight.TurnOn();
		if( Controller.IsA('PlayerController') )
		{
			Player = PlayerController(Controller);

			// ------- SPAWN SPOTLIGHT ---------
			if( Player.PlayerSpotLight == None )
			{
				// Create a flashlight for the player to carry around
				Player.PlayerSpotLight = Spawn( class'DynamicLight' ); //, self );
				// Turn off by default
				Player.PlayerSpotLight.LightType = LT_None;
			}

			// ------- VERTEX SPOTLIGHT ---------
			// Used on non-BSP geometry
			Player.PlayerSpotLight.LightType = LT_Steady;
			Player.PlayerSpotLight.LightHue = 0;
			Player.PlayerSpotLight.LightBrightness = 0;  // This is faded to max in playercontroller::PlayerTick
			Player.PlayerSpotLight.LightSaturation = 255;
			Player.PlayerSpotLight.LightRadius = 4000;
			Player.PlayerSpotLight.LightRadiusInner = 120;
			Player.PlayerSpotLight.LightCone = 0;
			Player.PlayerSpotLight.bDynamicLight = true;
			Player.PlayerSpotLight.bLightPriorityOverride = true; // cg: make it outpower all other lights. 
			PlaySound( Player.myHud.FlashlightOn );
			

			// ------- PROJECTED TEXTURE ---------
			// Used on BSP until we implement dynamic spots on BSP
			// Make sure player is not affected by the flashlight

			if( Player.myHud.FlashlightProjector != None )
			{
				if( Player.myHud.FlashlightProjector.IsActorExcluded( self ) == false )
				{
					Player.myHud.FlashlightProjector.ExcludeActor( self );
				}
				//Player.myHud.FlashlightProjector.Enable('Tick');

				// NathanM: Enable the following line if/when the projector is used on actors
				//Player.myHud.FlashlightProjector.SetCollision(true,false,false);

				Player.myHud.FlashlightProjector.ProjTexture = Player.myHud.FlashlightTexture;
			}
		}
	}
}

//***************************************

function GiveWeapon(string aClassName )
{
	local class<Weapon> WeaponClass;
	local Weapon NewWeapon;

	WeaponClass = class<Weapon>(DynamicLoadObject(aClassName, class'Class'));

	if( FindInventoryType(WeaponClass) != None )
		return;
	
	newWeapon = Spawn(WeaponClass);

	if( newWeapon != None )
		newWeapon.GiveTo(self);
}

function Disarm()
{
	if (Weapon != None)
	{
		if (Weapon.bSaveAsLastWeapon)
			OldWeapon = Weapon;
		Weapon.PutDown(None);
	}
}

function SetDisplayProperties(ERenderStyle NewStyle, Material NewTexture, bool bLighting )
{
	Style = NewStyle;
	Texture = NewTexture;
	bUnlit = bLighting;
	if ( Weapon != None )
		Weapon.SetDisplayProperties(Style, Texture, bUnlit);

	if ( !bUpdatingDisplay && (Inventory != None) )
	{
		bUpdatingDisplay = true;
		Inventory.SetOwnerDisplay();
	}
	bUpdatingDisplay = false;
}

function SetDefaultDisplayProperties()
{
	Style = Default.Style;
	texture = Default.Texture;
	bUnlit = Default.bUnlit;
	if ( Weapon != None )
		Weapon.SetDefaultDisplayProperties();

	if ( !bUpdatingDisplay && (Inventory != None) )
	{
		bUpdatingDisplay = true;
		Inventory.SetOwnerDisplay();
	}
	bUpdatingDisplay = false;
}

function FinishedInterpolation()
{
	DropToGround();
}

function JumpOutOfWater(vector jumpDir)
{
	Falling();
	Velocity = jumpDir * WaterSpeed;
	Acceleration = jumpDir * AccelRate;
	velocity.Z = FMax(380,JumpZ); //set here so physics uses this for remainder of tick
	bUpAndOut = true;
}


// Modify velocity called by physics before applying new velocity for this tick.
// Velocity,Acceleration, etc. have been updated by the physics, but location hasn't
// REVIST: This appears to be useless
simulated event ModifyVelocity(float DeltaTime, vector OldVelocity);

event FellOutOfWorld(eKillZType KillType)
{
	if ( Role < ROLE_Authority )
		return;
	if ( (Controller != None) && Controller.AvoidCertainDeath() )
		return;
	Health = -1;

	if( KillType == KILLZ_Lava)
		Died( None, class'Falling', Location );
	else if(KillType == KILLZ_Suicide)
		Died( None, class'Falling', Location );
	else
	{
	SetPhysics(PHYS_None);
		Died( None, class'Falling', Location );
	}
}

/* ShouldCrouch()
Controller is requesting that pawn crouch
*/
function ShouldCrouch(bool Crouch)
{
	bWantsToCrouch = Crouch;
}

// Stub events called when physics actually allows crouch to begin or end
// use these for changing the animation (if script controlled)
event EndCrouch(float HeightAdjust)
{
	EyeHeight -= HeightAdjust;
	OldZ += HeightAdjust;
	BaseEyeHeight = Default.BaseEyeHeight;
}

event StartCrouch(float HeightAdjust)
{
	EyeHeight += HeightAdjust;
	OldZ -= HeightAdjust;
	BaseEyeHeight = FMin(0.8 * CrouchHeight, CrouchHeight - 10);
}

function AddVelocity( vector NewVelocity)
{
	if ( bIgnoreForces || (NewVelocity == vect(0,0,0)) )
		return;
	if ( (Physics == PHYS_Walking)
		|| (((Physics == PHYS_Ladder) || (Physics == PHYS_Spider)) && (NewVelocity.Z > Default.JumpZ)) )
		SetPhysics(PHYS_Falling);
	if ( (Velocity.Z > 380) && (NewVelocity.Z > 0) )
		NewVelocity.Z *= 0.5;
	Velocity += NewVelocity;
}

function KilledBy( pawn EventInstigator, optional class<DamageType> damageType )
{
	local Controller Killer;

	if (damageType == None)
		damageType = class'Suicided';

	Health = 0;
	if ( EventInstigator != None )
		Killer = EventInstigator.Controller;
	Died( Killer, damageType, Location );
}

function TakeFallingDamage()
{
	local float Shake, EffectiveSpeed;

	if (Velocity.Z < -0.5 * MaxFallSpeed)
	{
		if ( Role == ROLE_Authority )
		{
		    //MakeNoise(1.0); REVISIT: Put in a stimulus
		    if (Velocity.Z < -1 * MaxFallSpeed)
		    {
			    EffectiveSpeed = Velocity.Z;
			    if ( TouchingWaterVolume() )
					EffectiveSpeed = FMin(0, EffectiveSpeed + 100);
			    if ( EffectiveSpeed < -1 * MaxFallSpeed )
				    TakeDamage(-100 * (EffectiveSpeed + MaxFallSpeed)/MaxFallSpeed, None, Location, vect(0,0,0), class'Falling');
		    }
		}
		if ( Controller != None )
		{
			Shake = FMin(1, -1 * Velocity.Z/MaxFallSpeed);
			//Controller.ShakeView(0.175 + 0.1 * Shake, 850 * Shake, Shake * vect(0,0,1.5), 120000, vect(0,0,10), 1);
		}
	}
	else if (Velocity.Z < -1.4 * JumpZ)
	{
		//MakeNoise(0.5); //REVISIT: Put in a stimulus
	}
}

function ClientReStart()
{
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
	BaseEyeHeight = Default.BaseEyeHeight;
	EyeHeight = BaseEyeHeight;
	PlayWaiting();
}

function ClientSetLocation( vector NewLocation, rotator NewRotation )
{
	if ( Controller != None )
		Controller.ClientSetLocation(NewLocation, NewRotation);
}

function ClientSetRotation( rotator NewRotation )
{
	if ( Controller != None )
		Controller.ClientSetRotation(NewRotation);
}

function ClientDying(class<DamageType> DamageType, vector HitLocation)
{
	if ( Controller != None )
		Controller.ClientDying(DamageType, HitLocation);
}

//=============================================================================
// Inventory related functions.

// check before throwing
simulated function bool CanThrowWeapon(Weapon WeaponToThrow)
{
    return ( (WeaponToThrow != None) && ((Level.Game == None) || !Level.Game.bWeaponStay) );
}

// toss out current weapon
event TossWeapon(Weapon WeaponToThrow, Vector TossVel)
{
	local Vector X,Y,Z, TossOrigin;
	local rotator TossOrientation;

	WeaponToThrow.Velocity = TossVel;
	GetAxes(Rotation,X,Y,Z);
	WeaponToThrow.PickupAmmoCount = WeaponToThrow.AmmoType.AmmoAmount;
	if ( (Controller != None) && (Controller.IsA('PlayerController')) )
	{
		TossOrigin = Location;
		WeaponToThrow.DropFrom(TossOrigin + 0.8 * CollisionRadius * X - 0.5 * CollisionRadius * Y, TossOrientation);
	}
	else
	{
		TossOrigin = GetBoneLocation(GetWeaponBoneFor(WeaponToThrow));
		TossOrientation = GetBoneRotation(GetWeaponBoneFor(WeaponToThrow));
		WeaponToThrow.DropFrom(TossOrigin, TossOrientation);
	}
	
}	

exec function SwitchToLastWeapon()
{
	if (Health <= 0)
		return;

	if ( OldWeapon != None )
	{
		PendingWeapon = OldWeapon;
		if( Weapon != None && !Weapon.IsInState('DownWeapon') )
			Weapon.PutDown(PendingWeapon);
		else
		{
			ChangedWeapon();
			Weapon.bChangeWeapon = false;
		}
	}
	else if (PlayerController(Controller) != None)
		PlayerController(Controller).SwitchWeapon(1);
}

// The player/bot wants to select next item
exec function NextItem()
{
	if (SelectedItem==None) {
		SelectedItem = Inventory.SelectNext();
		Return;
	}
	if (SelectedItem.Inventory!=None)
		SelectedItem = SelectedItem.Inventory.SelectNext(); 
	else
		SelectedItem = Inventory.SelectNext();

	if ( SelectedItem == None )
		SelectedItem = Inventory.SelectNext();
}

// FindInventoryType()
// returns the inventory item of the requested class
// if it exists in this pawn's inventory 

event Inventory FindInventoryType( class DesiredClass )
{
	local Inventory Inv;
	local int Count;

	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )   
	{
		if ( Inv.class == DesiredClass )
			return Inv;
		Count++;
		if ( Count > 1000 )
			return None;
	}
	return None;
} 

// Add Item to this pawn's inventory. 
// Returns true if successfully added, false if not.
function bool AddInventory( inventory NewItem )
{
	// Skip if already in the inventory.
	local inventory Inv;
	local actor Last;

	Last = self;
	
	// The item should not have been destroyed if we get here.
	if (NewItem ==None )
		log("tried to add none inventory to "$self);

	for( Inv=Inventory; Inv!=None; Inv=Inv.Inventory )
	{
		if( Inv == NewItem )
			return false;
		Last = Inv;
	}

	// Add to back of inventory chain (so minimizes net replication effect).
	NewItem.SetOwner(Self);
	NewItem.Inventory = None;
	Last.Inventory = NewItem;

	if ( Controller != None )
		Controller.NotifyAddInventory(NewItem);
	return true;
}

// Remove Item from this pawn's inventory, if it exists.
function DeleteInventory( inventory Item )
{
	// If this item is in our inventory chain, unlink it.
	local actor Link;
	local int Count;

	if ( Item == Weapon )
		Weapon = None;
	if ( Item == SelectedItem )
		SelectedItem = None;
	for( Link = Self; Link!=None; Link=Link.Inventory )
	{
		if( Link.Inventory == Item )
		{
			Link.Inventory = Item.Inventory;
			Item.Inventory = None;
			break;
		}
		if ( Level.NetMode == NM_Client )
		{
		Count++;
		if ( Count > 1000 )
			break;
	}
	}
	Item.SetOwner(None);

	if (Item.IsA('Weapon'))
		DeleteInventory(Weapon(Item).AmmoType);
}

// Just changed to pendingWeapon
event ChangedWeapon()
{
	local Weapon LastWeapon;

	LastWeapon = Weapon;

	// Reset FOV
	if( Controller != None && Controller.IsA('PlayerController') )
	{
		PlayerController(Controller).EndZoom();
	}
	
	if (Weapon == PendingWeapon)
	{
		if ( Weapon == None )
		{
			Controller.SwitchToBestWeapon();
			return;
		}
		/*
		// TODO: ClientIsInState is a NOOP .. will always retun false
		else if ( Weapon.ClientIsInState('DownWeapon') ) 
			Weapon.ClientGotoState('Idle');
		PendingWeapon = None;
		*/
		ServerChangedWeapon(LastWeapon, Weapon);
		return;
	}

	// jH: removing these lines so we can disarm the player
	//if ( PendingWeapon == None )
	//	PendingWeapon = Weapon;
	//Log("PendingWeapon "$PendingWeapon$" Weapon "$Weapon);
		
	Weapon = PendingWeapon;
	if ( (Weapon != None) && (Level.NetMode == NM_Client) )
		Weapon.BringUp(LastWeapon);
	PendingWeapon = None;
	if (Weapon != None)
		Weapon.Instigator = self;
	ServerChangedWeapon(LastWeapon, Weapon);

	if ( Controller != None )
		Controller.ChangedWeapon();
}

function ServerChangedWeapon(Weapon LastWeapon, Weapon W)
{
	if ( LastWeapon != None )
	{
		LastWeapon.SetDefaultDisplayProperties();
		LastWeapon.DetachFromPawn(self);		
	}
	Weapon = W;
	if ( Weapon == None )
		return;

	if ( Weapon != None )
		Weapon.AttachToPawn(self);

	Weapon.SetRelativeLocation(Weapon.Default.RelativeLocation);
	Weapon.SetRelativeRotation(Weapon.Default.RelativeRotation);
	if ( LastWeapon == Weapon )
	{
		// TODO: ClientIsInState is a NOOP .. will always retun false
		if ( Weapon.ClientIsInState('DownWeapon') ) 
			Weapon.BringUp(LastWeapon);
		Inventory.OwnerEvent('ChangedWeapon'); // tell inventory that weapon changed (in case any effect was being applied)
		return;
	}
	Inventory.OwnerEvent('ChangedWeapon'); // tell inventory that weapon changed (in case any effect was being applied)
	PlayWeaponSwitch(W);
	Weapon.BringUp(LastWeapon);
}

//==============
// Encroachment
event bool EncroachingOn( actor Other )
{
	if ( Other.bWorldGeometry )
		return true;
		
	if ( ((Controller == None) || !Controller.bIsPlayer || bWarping) && (Pawn(Other) != None) )
		return true;
		
	return false;
}

event EncroachedBy( actor Other )
{
}

function GibbedBy(actor Other)
{
	if ( Role < ROLE_Authority )
		return;

	if (!bCannotDie)
		Died(None, class'Gibbed', Location);
}

//Base change - if new base is pawn or decoration, damage based on relative mass and old velocity
// Also, non-players will jump off pawns immediately
function JumpOffPawn()
{
	Velocity += (100 + CollisionRadius) * VRand();
	Velocity.Z = 200 + CollisionHeight;
	SetPhysics(PHYS_Falling);
	bNoJumpAdjust = true;
	Controller.SetFall();
}

singular event BaseChange()
{
	if ( bInterpolating )
		return;

	if ( (base == None) && (Physics == PHYS_None) )
		SetPhysics(PHYS_Falling);
	// Pawns can only set base to non-pawns, or pawns which specifically allow it.
	// Otherwise we do some damage and jump off.
	else if ( Pawn(Base) != None )
	{	
		if ( !Pawn(Base).bCanBeBaseForPawns )
			JumpOffPawn();		
	}
}

event UpdateEyeHeight( float DeltaTime )
{
	local float smooth, MaxEyeHeight;
	local float OldEyeHeight;
	local Actor HitActor;
	local vector HitLocation,HitNormal;

	if (Controller == None )
	{
		EyeHeight = 0;
		return;
	}
	if ( bTearOff )
	{
		EyeHeight = 0;
		bUpdateEyeHeight = false;
		return;
	}
	HitActor = trace(HitLocation,HitNormal,Location + (CollisionHeight + MAXSTEPHEIGHT + 14) * vect(0,0,1),
					Location + CollisionHeight * vect(0,0,1),true);
	if ( HitActor == None )
		MaxEyeHeight = CollisionHeight + MAXSTEPHEIGHT;
	else
		MaxEyeHeight = HitLocation.Z - Location.Z - 14;

	// smooth up/down stairs
	smooth = FMin(1.0, 10.0 * DeltaTime/Level.TimeDilation);
	If( Controller.WantsSmoothedView() )
	{
		OldEyeHeight = EyeHeight;
		EyeHeight = FClamp((EyeHeight - Location.Z + OldZ) * (1 - smooth) + BaseEyeHeight * smooth,
							-0.5 * CollisionHeight, MaxEyeheight);
	}
	else
	{
		bJustLanded = false;
		EyeHeight = FMin(EyeHeight * ( 1 - smooth) + BaseEyeHeight * smooth, MaxEyeHeight);
	}
	Controller.AdjustView(DeltaTime);
}

/* EyePosition()
Called by PlayerController to determine camera position in first person view.  Returns
the offset from the Pawn's location at which to place the camera
*/
simulated event vector EyePosition()
{
	if (IsHumanControlled())
		return EyeHeight * vect(0,0,1) + WalkBob + PlayerController(Controller).ShakeOffset;
	else
		return EyeHeight * vect(0,0,1);
}

//=============================================================================

simulated event Destroyed()
{
	if ( Shadow != None )
	{
		Shadow.Destroy();
		Shadow = None;
	}
	if ( Flashlight != None )
		Flashlight.Destroy();
	if ( Controller != None )
		Controller.PawnDied(self);

	while ( Accessories.length > 0 )
	{
		if ( Accessories[0] != None && !Accessories[0].bDeleteMe )		
			Accessories[0].Destroy();
		else
			Accessories.Remove(0,1);
	}

	if ( Level.NetMode == NM_Client )
		return;

	while ( Inventory != None )
		Inventory.Destroy();

	Level.RemoveDeadBody( self );

	Weapon = None;
	Super.Destroyed();
}

//=============================================================================
//
// Called immediately before gameplay begins.
//
simulated event PreBeginPlay()
{
	local KarmaParamsSkel skelParams;
	//local int i;

	Super.PreBeginPlay();
	Instigator = self;
	DesiredRotation = Rotation;
	if ( bDeleteMe )
		return;

	if ( BaseEyeHeight == 0 )
		BaseEyeHeight = 0.8 * CollisionHeight;
	EyeHeight = BaseEyeHeight;

	/*
	for( i = 0; i < MeshSets.length; i++ )
	{
		LinkAnimToMesh( MeshSets[i].Set, MeshSets[i].Mesh );
	}
	*/

	// Use ragdoll "override" as base skeletal Karma spec.
	skelParams = KarmaParamsSkel(KParams);
	if( skelParams != none && skelParams.KSkeleton == "" )
		skelParams.KSkeleton = RagdollOverride;

}

event PostBeginPlay()
{
	local AIScript A;

	Super.PostBeginPlay();
	SplashTime = 0;
	SpawnTime = Level.TimeSeconds;
	EyeHeight = BaseEyeHeight;
	OldRotYaw = Rotation.Yaw;

	Health = MaxHealth;
	if (ShieldRechargeRate == 0.0)
	{
		//if we don't recharge shields, we don't need the Tick function
		NextShieldRechargeTime = 1e10;
		Disable('Tick');
	}
	SetShields( MaxShields );
	

	// automatically add controller to pawns which were placed in level
	// NOTE: pawns spawned during gameplay are not automatically possessed by a controller
	if ( Level.bStartup && (Health > 0) && !bDontPossess )
	{
		// check if I have an AI Script
		if ( AIScriptTag != '' )
		{			
			ForEach AllActors(class'AIScript',A,AIScriptTag)
				break;
			// let the AIScript spawn and init my controller
			if ( A != None )			
				A.SpawnControllerFor(self);			
		}
		if ( A == None ) //only do this work if we didn't find an AIScript controller
		{
			if ( (ControllerClass != None) && (Controller == None) )
				Controller = spawn(ControllerClass);
			if ( Controller != None )		
				Controller.Possess(self);
		}
	}
	
	if ( Level.bStartup && !bNoDefaultInventory )	
	{		
		AddDefaultInventory();		
	}  	

}

event PostLoadBeginPlay()
{
	local int i;

	if( bTearOff && Physics == PHYS_KarmaRagdoll )
	{
		Destroy();
		return;
	}

	for( i = 0; i < MeshSets.length; i++ )
	{
		LinkAnimToMesh( MeshSets[i].Set, MeshSets[i].Mesh );
	}
	Super.PostLoadBeginPlay();
}

// called after PostBeginPlay on net client
simulated event PostNetBeginPlay()
{
	if ( Role == ROLE_Authority )
		return;
	if ( Controller != None )
	{
		Controller.Pawn = self;
		if ( (PlayerController(Controller) != None)
			&& (PlayerController(Controller).ViewTarget == Controller) )
			PlayerController(Controller).SetViewTarget(self);
	} 

	if ( Role == ROLE_AutonomousProxy )
		bUpdateEyeHeight = true;

	if ( (PlayerReplicationInfo != None) 
		&& (PlayerReplicationInfo.Owner == None) )
		PlayerReplicationInfo.SetOwner(Controller);

	PlayWaiting();

	Super.PostNetBeginPlay();
}

simulated function SetMesh()
{
    if (Mesh != None)
        return;

	LinkMesh( default.mesh );
}

function Gasp();

function SetMovementPhysics()
{
	if (Physics == PHYS_Falling || Physics == PHYS_Flying)
		return;
	if ( CurrentPhysicsVolume.bWaterVolume )
		SetPhysics(PHYS_Swimming);
	else if ( Physics != PHYS_Walking )
		SetPhysics(PHYS_Walking);
}

simulated function TeamInfo GetTeam()
{
	return Level.GRI.GetTeam(GetTeamIndex());	
}

event DieAgain()
{
	Died(LastHitBy.Controller, LastHitDamageType, LastHitLocation, LastHitBone);
}

event BleedOut()
{
	Log("Bleed out: "$BleedOutTime);
	bIncapacitated=false;
	Died(None, class'DamageBleedOut', Location);
}

function Died(Controller Killer, class<DamageType> damageType, vector HitLocation, optional name BoneName )
{
    local Vector TossVel;
	local bool DamageKills;

	if ( bDeleteMe || Level.bLevelChange )
		return; // already destroyed, or level is being cleaned up

	bIncapacitated = false;
	bValidEnemy = false;
	CurrentLink = None;

	//lets the squad marker know the attached guy is dead
	if (SquadMarker != None)
		SquadMarker.MemberKilled(self, Killer, damageType);

	DamageKills = false;
	if (IsHumanControlled())
		DamageKills = damageType.default.bKillsNotIncapacitatesPlayer;
	else
		DamageKills = damageType.default.bKillsNotIncapacitates;

	//Log("DamageType: "$damageType$" DamageKills "$DamageKills);
	if (bCannotDie && !DamageKills)
	{
		if (Controller != None && Controller.Machine != None && Controller.Machine.IsA('Turret'))
		{			
			bIncapacitatedOnTurret = true;
			bValidEnemy=true;
			return;
		}
		Health = 0;
		OnIncapacitated(Killer, damageType, HitLocation);
		return;
	}

	bIncapacitatedOnTurret=false;
	// mutator hook to prevent deaths
	// WARNING - don't prevent bot suicides - they suicide when really needed
	if ( Level.Game.PreventDeath(self, Killer, damageType, HitLocation) )
	{
		Health = max(Health, 1); //mutator should set this higher
		return;
	}

	//PostStimulusToWorld(ST_DiedDefault); //moved to Landed
	PlayDeathCue(damageType);

	Health = Min(0, Health);

    if (Weapon != None)
    {
		if ( Controller != None )
			Controller.LastPawnWeapon = Weapon.Class;
        Weapon.HolderDied();
		if (ShouldTossWeapon(None))
		{
			TossVel = Vector(GetViewRotation());
			TossVel = TossVel * ((Velocity Dot TossVel) + 500) + Vect(0,0,200);
			TossWeapon(Weapon, TossVel);

			if( CurrentGrenade != None && FRand() <= DropGrenadePercentage )
				TossWeapon(CurrentGrenade, TossVel);
		}
    }

	if( Killer != None )
	{
		Killer.Killed(self, damageType);
	}

	if ( Controller != None ) 
	{   
		Controller.WasKilledBy(Killer, damageType);
		Level.Game.Killed(Killer, Controller, self, damageType);
	}
	else
		Level.Game.Killed(Killer, Controller(Owner), self, damageType);

	if ( Killer != None )
		TriggerEvent(Event, self, Killer.Pawn);
	else
		TriggerEvent(Event, self, None);

	if ( IsHumanControlled() )
		PlayerController(Controller).ForceDeathUpdate();
	else if (Squad != None)
	{
		Squad.RemoveFromSquad(self);
	}

	if( DeathEffectContainerClass != None && !DamageType.default.NoDeathEffect )
		DeathEffectContainerClass.static.DoDeathEffect( self, DamageType, 0, HitLocation, TearOffMomentum, BoneName );
	else
		PlayDying(DamageType, HitLocation, BoneName);

	if ( Level.Game.bGameEnded )
		return;
	if ( !bPhysicsAnimUpdate && !IsLocallyControlled() )
		ClientDying(DamageType, HitLocation);
}

event Falling()
{
	//SetPhysics(PHYS_Falling); //Note - physics changes type to PHYS_Falling by default
	if ( Controller != None )
		Controller.SetFall();
}

event HitWall(vector HitNormal, actor Wall, EMaterialType KindOfMaterial);

event Landed(vector HitNormal)
{
	LandBob = FMin(50, 0.055 * Velocity.Z); 
	TakeFallingDamage();
	if ( Health > 0 )
		PlayLanded(Velocity.Z);
	bJustLanded = true;
}

event HeadVolumeChange(PhysicsVolume newHeadVolume)
{
	if ( (Level.NetMode == NM_Client) || (Controller == None) )
		return;
}

function bool TouchingWaterVolume()
{
	local PhysicsVolume V;

	ForEach TouchingActors(class'PhysicsVolume',V)
		if ( V.bWaterVolume )
			return true;
			
	return false;
}

//Pain timer just expired.
//Check what zone I'm in (and which parts are)
//based on that cause damage, and reset BreathTime

function bool IsInPain()
{
	local PhysicsVolume V;

	ForEach TouchingActors(class'PhysicsVolume',V)
		if ( V.bPainCausing && (V.DamagePerSec > 0) )
			return true;
	return false;
}		

function bool CheckWaterJump(out vector WallNormal)
{
	local actor HitActor;
	local vector HitLocation, HitNormal, checkpoint, start, checkNorm, Extent;

	checkpoint = vector(Rotation);
	checkpoint.Z = 0.0;
	checkNorm = Normal(checkpoint);
	checkPoint = Location + CollisionRadius * checkNorm;
	Extent = CollisionRadius * vect(1,1,0);
	Extent.Z = CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, checkpoint, Location, true, Extent);
	if ( (HitActor != None) && (Pawn(HitActor) == None) )
	{
		WallNormal = -1 * HitNormal;
		start = Location;
		start.Z += 1.1 * MAXSTEPHEIGHT;
		checkPoint = start + 2 * CollisionRadius * checkNorm;
		HitActor = Trace(HitLocation, HitNormal, checkpoint, start, true);
		if (HitActor == None)
			return true;
	}

	return false;
}

function bool Dodge(eDoubleClickDir DoubleClickMove)
{
	return false;
}

//Player Jumped
function bool DoJump( bool bUpdating )
{
	if ( !bIsCrouched && !bWantsToCrouch && ((Physics == PHYS_Walking) || (Physics == PHYS_Ladder) || (Physics == PHYS_Spider)) )
	{
		if ( Role == ROLE_Authority )
		{
			//REVISIT: NathanM: Add Jump Stimulus Here
		}
		PlayOwnedCue(PAE_JumpGrunt);
		if ( Physics == PHYS_Spider )
			Velocity = JumpZ * Floor;
		else if ( Physics == PHYS_Ladder )
			Velocity.Z = 0;
		else if ( bIsWalking )
			Velocity.Z = Default.JumpZ;
		else
			Velocity.Z = JumpZ;
		if ( (Base != None) && !Base.bWorldGeometry )
			Velocity.Z += Base.Velocity.Z; 
		PlayJumpSoundOnCurrentMaterial();
		SetPhysics(PHYS_Falling);
        return true;
	}
    return false;
}

/* PlayMoverHitSound()
Mover Hit me, play appropriate sound if any
*/
function PlayMoverHitSound();

function PlayDyingSound();


/* 
Pawn was killed - detach any controller, and die
*/

// blow up into little pieces (implemented in subclass)		
// Pawn was killed - detach any controller, and die
simulated function ChunkUp( Rotator HitRotation, class<DamageType> D ) 
{
	if (bCannotDie)
		return;

	if ( (Level.NetMode != NM_Client) && (Controller != None) )
	{
		if ( Controller.bIsPlayer )
			Controller.PawnDied(self);
		else
			Controller.Destroy();
	}

	bTearOff = true;
	HitDamageType = class'Gibbed'; // make sure clients gib also
	if ( (Level.NetMode == NM_DedicatedServer) || (Level.NetMode == NM_ListenServer) )
		GotoState('TimingOut');
	if ( Level.NetMode == NM_DedicatedServer ) 
		return;
	if ( class'GameInfo'.static.UseLowGore() )
	{
		Destroy();
		return;
	}
	SpawnGibs(HitRotation,D);

	if ( Level.NetMode != NM_ListenServer )
		Destroy();
}

// spawn gibs (local, not replicated)
simulated function SpawnGibs(Rotator HitRotation, class<DamageType> D);

//=============================================================================
// Animation interface for controllers

simulated event SetAnimAction(name NewAction)
{
	AnimAction = NewAction;
	PlayAnim(AnimAction);
}

function String GetDebugName()
{
	if ( PlayerReplicationInfo != None )
		return PlayerReplicationInfo.PlayerName;
	else
		return GetItemName(string(self));
}

simulated function name SelectFireAnim()
{
	if (Level.NetMode == NM_Standalone)
	{
		if (bIsCrouched)
			return 'CrouchFire';
		if (UseWoundedAnims())
			return 'WoundedFire';
		return 'Fire';
	}

	if (Weapon != None)
		return WeaponAttachment(Weapon.ThirdPersonActor).MPPawnFireAnim;

	//log("ERROR!! SelectFireAnim(): pawn has no weapon: "$self);
	return 'DC17mFire'; //error condition! fall back to something reasonable
}

simulated function PawnPlayFiring(name animName, bool bLoop)
{
	if ( bDisableFireAnims )
		return;

	//if we are a dedicated server or this pawn is locally controlled (1st person POV), no need to play a fire anim on me
	if (Level.NetMode == NM_DedicatedServer || IsLocallyControlled())
		return;

	if (animName == '')
		animName = SelectFireAnim();

	if ( bLoop )
		LoopAnim( animName, 'fire', , , 3 );
	else
		PlayAnim( animName, 'fire', , , 3 );
}

simulated event PawnStopFiring()
{
	LoopAnim( '', 'fire', , , 3 );
}

function PlayWeaponSwitch(Weapon NewWeapon);


// REVISIT: simulated?
function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType)
{
	local vector X,Y,Z,Dir;

	if (Damage <= 0)
		return;

	GetAxes(Rotation,X,Y,Z);
	Dir = Normal(HitLoc - Location);	
	PlayAnim('hit');	
}

//=============================================================================
// Pawn internal animation functions

//play the standard breathe on pawn whole body, and if need to, play a weapon specific breathe on upper body
simulated function PlayBreathe(optional name animName)
{
	LoopAnim('ActionBreathe');
	if (Level.NetMode == NM_Standalone && !Level.IsSplitScreen())
		return;
	
	if (animName != '')
		BreatheWeaponAnim = animName;

	if (BreatheWeaponAnim != '' && HasAnim(BreatheWeaponAnim))
		LoopAnim( BreatheWeaponAnim, 'spine2', , , 2 );
}

simulated event ChangeAnimation()
{
	if ( (Controller != None) && Controller.bControlAnimations )
		return;	
	if (bIncapacitated || Health < 0) // REVISIT: this might be handled better a different way
		return;

	// player animation - set up new idle and moving animations
	PlayWaiting();
	PlayMoving();
}

simulated event AnimEnd(int Channel)
{
	// REVISIT: Cut this entirely?
	//if ( Channel == 0 )
	//	PlayWaiting();
}

// Animation group checks (usually implemented in subclass)

function bool CannotJumpNow()
{
	return false;
}

simulated event EndJump()
{
}

simulated event PlayJump()
{
	//PlayOwnedCue(PAE_JumpGrunt);
	//PlayOwnedSound(JumpSound, SLOT_Talk, 1.0, true );
	//BaseEyeHeight =  0.7 * Default.BaseEyeHeight;
	//PlayAnim('Jump');
}

simulated event PlayFalling()
{
	if( HasAnim( 'FallBreathe' ) )
		LoopAnim( 'FallBreathe' );
	else
		PlayBreathe();
}

simulated function PlayMoving()
{
	local EPawnMovementStyle OldMovementStyle;
	
	OldMovementStyle = CurrentMovementStyle;
	CurrentMovementStyle = UpdateMovementStyle();

	if (CurrentMovementStyle != OldMovementStyle || OldIdleState != CurrentIdleState ){		
		ChangeWeaponAnimation();
		ChangeMovementAnimation();
	}
	OldIdleState = CurrentIdleState;
	ChangeWeaponAnimation();
}

simulated function ChangeMovementAnimation()
{
	switch (CurrentMovementStyle)
	{
	case MS_None:
		return;
	case MS_Crouch:
		AnimateCrouchWalking();
		return;
	case MS_Walk:
		AnimateWalking();
		return;
	case MS_WalkWounded:
		AnimateWoundedWalking();
		return;
	case MS_Run:	
		AnimateRunning();
		return;
	case MS_RunWounded:
		AnimateWoundedRunning();
		return;
	case MS_Swim:
		AnimateSwimming();
		return;
	case MS_Fly:
		AnimateFlying();
		return;
	default:
	}
}

simulated function bool CanChangeAnims()
{
	return ( Physics != PHYS_Falling && Physics != PHYS_Flying && Physics != PHYS_RootMotion && !IsManningTurret() && !IsShutdown() );
}

simulated function bool IsManningTurret()
{
	return ( CurrentIdleState == AS_StandTurret || CurrentIdleState == AS_SitTurret );
}

simulated function bool IsShutdown()
{
	return ( CurrentIdleState == AS_Shutdown || CurrentIdleState == AS_PerchedWall || CurrentIdleState == AS_PerchedCeiling );
}

simulated function RelaxedStanding()
{
	if( HasAnim('RelaxBreathe' ) )
		LoopAnim('RelaxBreathe');
	else
		PlayBreathe();
}

simulated function AnimateStanding()
{
	if (Physics == PHYS_Flying)
	{
		if( HasAnim('FlyBreathe') )
			LoopAnim('FlyBreathe');
		else
			PlayBreathe();
	}
	else
	{
		switch(CurrentIdleState)
		{
		case AS_Shutdown:			
			if( HasAnim('ShutdownBreathe' ) )
				LoopAnim('ShutdownBreathe');
			else if( HasAnim('RelaxBreathe') )
				LoopAnim('RelaxBreathe');
			else
				PlayBreathe();
			break;			
		case AS_Relaxed:
			if (UseWoundedAnims())
				LoopAnim('WoundedBreathe');
			else
				RelaxedStanding();
			break;
		case AS_Alert:
			if (UseWoundedAnims())
				LoopAnim('WoundedBreathe');
			else if( HasAnim('ActionBreathe' ) )
				PlayBreathe();
			else if( HasAnim('RelaxBreathe') )
				LoopAnim('RelaxBreathe');
			break;			
		case AS_Berserk:			
			LoopAnim('BerserkBreathe');			
			break;								
		case AS_PerchedWall:
			LoopAnim('WallBreathe');			
			break;		
		case AS_PerchedCeiling:					
			LoopAnim('CeilingBreathe');
			break;
		case AS_Possessed:
			LoopAnim('ScavHeadBreathe');			
			break;
		case AS_StandTurret:			
			LoopAnim('TurretStandBreathe');			
			break;
		case AS_SitTurret:				
			LoopAnim('TurretSitBreathe');						
			break;
		case AS_BackToWall:
			LoopAnim('WallStandBreathe');
			break;
		}
	}
}

// AnimateCrouchWalking(): crouching and walking
simulated function AnimateCrouchWalking()
{
	TurnLeftAnim = 'CrouchTurnLeft';
	TurnRightAnim = 'CrouchTurnRight';
	MovementAnim = 'CrouchForward';
}

simulated function AnimateWoundedWalking()
{
	TurnLeftAnim = 'WoundedTurnLeft';
	TurnRightAnim = 'WoundedTurnRight';
	MovementAnim = 'WoundedWalkForward';
}

simulated function AnimateWoundedRunning()
{
	TurnLeftAnim = 'WoundedTurnLeft';
	TurnRightAnim = 'WoundedTurnRight';
	MovementAnim = 'WoundedRunForward';
}

simulated function AnimateWalking()
{
	TurnLeftAnim = 'TurnLeft';
	TurnRightAnim = 'TurnRight';

	MovementAnim = 'WalkForward';
}

// AnimateFlying()
//flying - not used in Warfare, so don't need real animation
simulated function AnimateFlying()
{
	if( Physics == PHYS_Flying && bCanFly )
		MovementAnim = 'FlyForward';
	else
		MovementAnim = 'ActionBreathe';
}

simulated function AnimateCrouching()
{
	LoopAnim('CrouchBreathe');
}
	
simulated function AnimateRunning()
{
	if( CurrentIdleState == AS_Berserk )
	{
		MovementAnim = 'BerserkRunForward';
		TurnLeftAnim = 'TurnLeft';
		TurnRightAnim = 'TurnRight';
	}
	else
	{
		MovementAnim = 'RunForward';
		TurnLeftAnim = 'TurnLeft';
		TurnRightAnim = 'TurnRight';
	}
}

/* AnimateSwimming()
Moving through water - check acceleration for direction
*/
simulated function AnimateSwimming()
{
}

/* AnimateTreading()
Still in water
*/
simulated function AnimateTreading()
{
}


simulated function EPawnMovementStyle UpdateMovementStyle()
{
	if ( (Physics == PHYS_None) || ((Controller != None) && Controller.bPreparingMove) )
	{
		return MS_None;
	}
	if (Physics == PHYS_Walking)
	{
		if (bIsCrouched)
		{
			return MS_Crouch;
		}
		else if (bIsWalking)
		{
			if (UseWoundedAnims())
				return MS_WalkWounded;
			return MS_Walk;
		}
		else if (UseWoundedAnims())
			return MS_RunWounded;
		else
			return MS_Run;
	}
	//else if ((Physics == PHYS_Swimming) || ((Physics == PHYS_Falling) && IsPlayingSwimming() && TouchingWaterVolume()))
	//	return MS_Swim;
	else if ( Physics == PHYS_Ladder )
		return MS_Ladder;
	else if ( Physics == PHYS_Flying )
		return MS_Fly;
	else if (bIsCrouched)
		return MS_Crouch;
	else if (bIsWalking)
		return MS_Walk;
	else
		return MS_Run;
}

simulated function ChangeWeaponAnimation()
{	
	return;
}

simulated function PlayWaiting()
{
	if (bIncapacitated) //REVISIT: this might be handled better a different way
		return;
	
	if ( Physics == PHYS_Falling && VSizeSq( Velocity ) != 0 )	
		PlayFalling();
	else if ( bIsCrouched )
		AnimateCrouching();	
	else
		AnimateStanding();
}

function PlayLanded(float impactVel)
{	
	impactVel = impactVel/JumpZ;
	impactVel = 0.1 * impactVel * impactVel;
	BaseEyeHeight = Default.BaseEyeHeight;

	PostStimulusToWorld(ST_FootstepLoud);

	if ( impactVel > 0.17 )	
		PlayOwnedCue(PAE_LandGrunt);

	if ( (impactVel > 0.01) && !TouchingWaterVolume() )
		PlaySound(Land, SLOT_Interact, FClamp(4 * impactVel,0.5,5), false);
}

simulated event PlayDodging( bool bRight )
{
    if( bRight )
		PlayAnim('DodgeRight');
	else
		PlayAnim('DodgeLeft' );
}

simulated function PlayDyingAnim(class<DamageType> DamageType, vector HitLoc)
{
	if( !DamageType.default.DisallowDeathAnim )
	{
		StopAnimating( true );

		if( IsHumanControlled() )
			PlayAnim( 'PlayerDeath' );
		else if( CurrentIdleState == AS_StandTurret )
			PlayAnim( 'TurretStandDeath' );
		else if( CurrentIdleState == AS_SitTurret )
			PlayAnim( 'TurretSitDeath' );
		else if( Physics == PHYS_Flying )
			PlayAnim( 'FlyDeath' );
		else
		{
			if( DamageType.IsA( 'CTDamageElectricity' ) && HasAnim( 'ElectrocutionDeath' ) && HasAnim( 'OnFireDeath' ) && frand() > 0.5 )
			{
				PlayAnim( 'ElectrocutionDeath' );
				return;
			}
			else if( ( DamageType.IsA( 'CTDamageFire' ) || DamageType.IsA( 'CTDamageAcid' ) ) && HasAnim( 'OnFireDeath' ) && frand() > 0.5 )
			{
				PlayAnim( 'OnFireDeath' );
				return;
			}
			else
			{
				PlayAnim( 'Death' );
			}
		}
	}
}

simulated event PlayDying(class<DamageType> DamageType, vector HitLoc, optional name BoneName )
{
	local Emitter DeathEmitter;	

	AmbientSound = None;
    bCanTeleport = false; 
    bReplicateMovement = false;
    bTearOff = true;
    bPlayedDeath = true;
		
	HitDamageType = DamageType; // these are replicated to other clients
    TakeHitLocation = HitLoc;

	if( !bSaveMyCorpse )
		LifeSpan = RagdollLifeSpan;

	DeathLoopStartTime = Level.TimeSeconds;

	if( DeathEffect != None && !DamageType.default.NoDeathEffect )
	{
		DeathEmitter = Spawn( DeathEffect );
		DeathEmitter.SetBase( self );
	}

	GotoState('Dying');

	if( TryToRagdoll( DamageType, BoneName ) )
	{				
		if (GoRagDoll(DamageType, HitLoc))
			return;
	}

	// non-ragdoll death fallback
	if( ( Physics != PHYS_Falling && ( Physics != PHYS_Flying || HasAnim('FlyDeath') ) && CurrentIdleState != AS_PerchedWall && CurrentIdleState != AS_PerchedCeiling ) ||
		( Level.NetMode != NM_StandAlone || IsHumanControlled() ) )
	{
		BaseEyeHeight = Default.BaseEyeHeight;
		PlayDyingAnim(DamageType, HitLoc);
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);

		if( IsManningTurret() )
		{
			SetBase(None);
			SetPhysics(PHYS_RootMotion);
		}
		else if( bFallOnDeath )
			SetPhysics(PHYS_Falling);
		else
			SetPhysics(PHYS_None);		
	}
	else if( !bSaveMyCorpse )
	{
		CompletelyDismember(VSize(TearOffMomentum));
		Destroy();
	}
}

simulated function bool TryToRagdoll( class<DamageType> DamageType, name BoneName )
{
	return	Level.NetMode != NM_DedicatedServer &&
			( !IsHumanControlled() || !IsLocallyControlled() ) &&
			( Physics == PHYS_Falling || Physics == PHYS_Flying || frand() < DamageType.default.RagdollOnDeathProbability || !HasAnim('Death') );
}

simulated function PartiallyDismember( name BoneName, vector HitLocation, vector Momentum )
{
	local int i;
	local vector BoneLoc;
	local rotator BoneRot;
	local Accessory NewPart;
	local Emitter NewEmitter;	

	if( class'GameInfo'.static.UseLowGore() && TypeOfMaterial != MT_DroidMetal )
		return;

	if (BoneName == 'spine1' || BoneName == 'spine2' || BoneName == 'pelvis')
		bDismembered = true;

	for( i = 0; i < DismembermentParts.length; i++ )
	{
		if( DismembermentParts[i].DisablePartialDismemberment )
			return;

		if( DismembermentParts[i].Bone == BoneName )
		{
			BoneLoc = GetBoneLocation( BoneName );
			BoneRot = GetBoneRotation( BoneName );

			if( DismembermentParts[i].Part != None && Level.ActiveKarmaAccessories < Level.MaxKarmaAccessories )
			{
				NewPart = Spawn( DismembermentParts[i].Part, , , BoneLoc, BoneRot );
				if( NewPart != None )
					NewPart.DetachAccessory(HitLocation,Momentum);					
			}

			if( DismembermentParts[i].DismembermentEffectClass != None )
			{
				NewEmitter = Spawn( DismembermentParts[i].DismembermentEffectClass, , , BoneLoc, BoneRot );
				if( NewEmitter != None )
				{
					AttachToBone( NewEmitter, BoneName );
					NewEmitter.SetRelativeLocation( vect(0,0,0) );
					NewEmitter.SetRelativeRotation( rot(0,0,0) );
				}
			}

			break;
		}
	}

	SetBoneScale( BoneName, 0 );
}

simulated function CompletelyDismember( float MomentumMag )
{
	local int i;
	local Accessory NewPart;
	local vector DetachVector;

	if( class'GameInfo'.static.UseLowGore() && TypeOfMaterial != MT_DroidMetal )
		return;

	// Make sure that our sphyl goes away before we spawn a bunch of parts.
	KSetBlockKarma(false);

	bDismembered = true;

	for( i = 0; i < DismembermentParts.length; i++ )
	{
		if( Level.ActiveKarmaAccessories < Level.MaxKarmaAccessories && frand() < DismembermentParts[i].SpawnProbability )
		{			
			NewPart = Spawn( DismembermentParts[i].Part, , , GetBoneLocation( DismembermentParts[i].Bone ), GetBoneRotation( DismembermentParts[i].Bone ) );
			if( NewPart != None )
			{
				DetachVector = ( NewPart.Location - Location );
				if( VIsZero( DetachVector ) )
					DetachVector = vect(0,0,1);
				else
					DetachVector = Normal( DetachVector );

				DetachVector *= MomentumMag;
				NewPart.DetachAccessory(NewPart.Location, DetachVector + Velocity);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// DEATH AND INCAPACITATION
///////////////////////////////////////////////////////////////////////////////

// Called when in Ragdoll when we hit something over a certain threshold velocity
// Used to play impact sounds.
event KImpact(actor other, vector pos, vector impactVel, vector impactNorm, EMaterialType MaterialHit)
{
	local int numSounds, soundNum;
	numSounds = RagImpactSounds.Length;

	if(numSounds > 0 && Level.TimeSeconds > RagLastSoundTime + RagImpactSoundInterval)
	{
		soundNum = Rand(numSounds);
		PlaySound(RagImpactSounds[soundNum], SLOT_Pain, RagImpactVolume);
		RagLastSoundTime = Level.TimeSeconds;
	}
}

/* TimingOut - where gibbed pawns go to die (delay so they can get replicated)
*/
State TimingOut
{
ignores BaseChange, Landed, AnimEnd, Trigger, Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling;

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		return 0;
	}

	function BeginState()
	{
		SetPhysics(PHYS_None);
		SetCollision(false,false,false);
		LifeSpan = 1.0;
		if ( Controller != None )
		{
			if ( Controller.bIsPlayer )
				Controller.PawnDied(self);
			else
				Controller.Destroy();
		}
	}
}

State Dying
{
ignores Trigger, Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling;

	event ChangeAnimation() {}	
	function PlayWaiting() {}
	function PlayBreathe(optional name animName) {}
	function PlayWeaponSwitch(Weapon NewWeapon) {}
	function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType) {}
	simulated function PlayNextAnimation() {}

	event Revive()
	{
		//can't be revived when we're dead
	}

	function Timer()
	{
		if ( !bSaveMyCorpse && !PlayerCanSeeMe() )
			Destroy();
		else
			SetTimer( 1.5, false );
	}

	event AnimEnd(int Channel)
	{		
		if ( Channel != 0 )
			return;
		
		if( !bCollideAfterDeath )
			SetCollision(false, false, false);

		bProjTarget = false;		
	}

	simulated event AnimLoopEnd( float LoopStart )
	{
		if( (Level.TimeSeconds - LastHitLocalTime) < 0.2 && Level.TimeSeconds - DeathLoopStartTime < MaxDeathLoopTime )
		{
			PlayDyingAnim( HitDamageType, vect(0,0,0) );
		}
	}

	function Landed(vector HitNormal)
	{
		LandBob = FMin(50, 0.055 * Velocity.Z);
		if( Weapon != None )
			Weapon.ServerStopFire();
		if ( Level.NetMode == NM_DedicatedServer )
			return;
		if ( Shadow != None )
			Shadow.Destroy();
	}

	simulated function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		// Warning!! LastHitTime is not replicated as absolute times don't match on clients at all
		if( DamageType.IsA( HitDamageType.Name ) )
		{
			LastHitLocalTime = Level.TimeSeconds;			
		}

		if(bPlayedDeath && Physics == PHYS_KarmaRagdoll)
		{
			//LifeSpan += 2.0;
			//LifeSpan = Min( LifeSpan, RagdollLifeSpan );			
				
			if( damageType.default.KApplyImpulseToRoot )
			{
				KAddImpulse( Momentum, HitLocation, 'root');
			}
			else
			{
				KAddImpulse( Momentum, HitLocation );
			}			
		}

		return Damage;
	}

	singular function BaseChange()
	{
		if( base == None && 
			Physics != PHYS_Falling && Physics != PHYS_Karma && Physics != PHYS_KarmaRagdoll )
			SetPhysics(PHYS_Falling);		        	
	}

	function BeginState()
	{
		if( !bCollideAfterDeath )
		{
			SetCollision(true,false,false);			

			if( bBlockKarma && Physics != PHYS_KarmaRagdoll )			
				KSetBlockKarma(false);		
		}

		bCollideWorld = true;

        if ( bTearOff && (Level.NetMode == NM_DedicatedServer) )
			LifeSpan = 1.0;
		else
			SetTimer(RagdollLifeSpan * 0.1, false);
        		
		if ( Controller != None )
		{
			if ( Controller.bIsPlayer )
				Controller.PawnDied(self);
			else
			{
				Controller.Destroy();
				Controller = None;
				SetOwner(None);
			}
		}

		Level.AddDeadBody( self );
	}

	function Died(Controller Killer, class<DamageType> damageType, vector HitLocation, optional name BoneName )
	{
	}

	event FellOutOfWorld(eKillZType KillType)
	{
		if(KillType == KILLZ_Lava || KillType == KILLZ_Suicide )
			return;

		Destroy();
	}

Begin:
	Sleep(0.15);
	PlayDyingSound();
	Sleep(1.5);
	PostStimulusToWorld(ST_DiedDefault);
}

event Revive()
{
	Health = 1;
	bCanTeleport = true; 
	GotoState('Reviving');
}

simulated event bool GoRagDoll(class<DamageType> DamageType, vector HitLocation)
{
	local vector shotDir;
	local float maxDim;	
	local KarmaParamsSkel skelParams;
	local vector TraceOffset;

	if( RagdollOverride != "" )
		KMakeRagdollAvailable();
	else
		return false;

	if( KIsRagdollAvailable() )
	{
        skelParams = KarmaParamsSkel(KParams);
		skelParams.KSkeleton = RagdollOverride;
		KParams = skelParams;
		
		// Stop animation playing.
		StopAnimating(false);
		
		// Set initial linear velocity for ragdoll.
		skelParams.KStartLinVel = Velocity;			
		    
		// Set up deferred shot-bone impulse
		shotDir = Normal(TearOffMomentum);
		maxDim = Max(CollisionRadius, CollisionHeight);

		TraceOffset = ( 2 * maxDim ) * shotDir;
					    
		skelParams.KShotStart = HitLocation - TraceOffset;
		skelParams.KShotEnd = HitLocation + TraceOffset;
		skelParams.KShotStrength = VSize(TearOffMomentum);
		if( DamageType != None && DamageType.default.KApplyImpulseToRoot )
			skelParams.bKApplyToRoot = true;		
		
		// Turn on Karma collision for ragdoll.		
		KSetBlockKarma(true);
		
		// Set physics mode to ragdoll. 
		// This doesn't actaully start it straight away, it's deferred to the next tick.
		SetPhysics(PHYS_KarmaRagdoll);

		return true;
	}

	return false;
}

//Client functionality
simulated event PlayIncapacitated(class<DamageType> DamageType, vector HitLocation)
{
	bCanTeleport = false; 
		
	HitDamageType = DamageType; // these are replicated to other clients
    TakeHitLocation = HitLocation;
	bIncapacitated = true;
	PlayDeathCue(DamageType);

	if (Role == ROLE_Authority)
		GotoState('Incapacitated');  //only use the state on the server

	if ( Level.NetMode != NM_DedicatedServer )
	{
		//if (GoRagDoll(DamageType, HitLocation))
		//	return;
	}
		
	// non-ragdoll death fallback
    BaseEyeHeight = Default.BaseEyeHeight;    
	bPlayedDeath = true;
}

function SetReviveDuration(class<DamageType> DamageType)
{
}

function OnIncapacitated(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{	
	//Log("Incapacitated");
	SetReviveDuration(DamageType);
	PlayIncapacitated(DamageType, HitLocation);

	if (Killer != None)
		Killer.Killed(self, damageType);

	if ( Killer != None )
		TriggerEvent(Event, self, Killer.Pawn);
	else
		TriggerEvent(Event, self, None);

	Controller.WasKilledBy(Killer, damageType);

}

function PlayReviveCue()
{
	if (Squad != None)
		Squad.AddVoiceLine(self, PAE_Revived, None, 4.0f, 2.0f);
	else
		PlayOwnedCue(PAE_Revived);
}

State Reviving
{
	ignores GibbedBy;
	event ChangeAnimation() {}	
	function PlayWeaponSwitch(Weapon NewWeapon) {}
	function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType) {}
	simulated function PlayNextAnimation() {}

	function bool CanChangeAnims()
	{
		return false;
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{	
		local bool DamageKills;
		DamageKills = false;
		if (IsHumanControlled())
			DamageKills = damageType.default.bKillsNotIncapacitatesPlayer;
		else
			DamageKills = damageType.default.bKillsNotIncapacitates;

		//if the damage is able to kill us, then kill us
		if (DamageKills)
		{
			bIncapacitated=false;
			Died(instigatedBy.Controller, damageType, hitlocation);
		}
		return 0;
	}

	simulated function BeginState()
	{
		Controller.BeginPawnRevive(self);
		SetCollision(true, true, true);
		bWantsToCrouch = false;
		//SetCollisionSize(default.CollisionRadius, default.CollisionHeight);
		Health = 1;
		bPlayedDeath = false;  //this does need to be set on client...
	}

	simulated event Tick(float DeltaTime)
	{
		Global.Tick(DeltaTime);
		Health += MaxHealth/12.0f * DeltaTime;
	}
	function EndState()
	{
		if (Anchor != None && Anchor.bBlocked)
		{
			Anchor = None;
			CurrentLink = None;
			LastValidAnchorTime = 0;
		}
		bIncapacitated = false;
		bValidEnemy = true;
		Health = MaxHealth/2;
		SetHealthLevel();		
		bCanTeleport = true;
		PlayReviveCue();
		Controller.PawnRevived(self); //does this need to be execute on client?
	}

Begin:
	if( IsHumanControlled() )
		PlayAnim('PlayerImmobileGetUp');
	else
		PlayAnim('ImmobileGetUp');
	FinishAnim();
	GotoState(''); //this might need to execute on client
}


//This should get partially unified with state Dying
//because they are very similar
State Incapacitated
{
	ignores GibbedBy, Trigger, Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling;

	function PlayWaiting(){}
	event ChangeAnimation() {}	
	function PlayWeaponSwitch(Weapon NewWeapon) {}
	function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType) {}
	simulated function PlayNextAnimation() {}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{	
		local bool DamageKills;

		DamageKills = false;
		if (IsHumanControlled())
			DamageKills = damageType.default.bKillsNotIncapacitatesPlayer;
		else
			DamageKills = damageType.default.bKillsNotIncapacitates;

		//if the damage is able to kill us, then kill us
		if (DamageKills)
		{
			bIncapacitated=false;
			Died(instigatedBy.Controller, damageType, hitlocation);
		}
		return 0;
	}
	function OnIncapacitated(Controller Killer, class<DamageType> damageType, vector HitLocation)
	{
	}

	function bool CanChangeAnims()
	{
		return false;
	}

	function EndState()
	{
		local int i;

		RemoveReviveMarker();

		bEnableAim = true;
		bEnableTurn = true;
		
		while( i < Attached.length )
		{
			if( Attached[i].IsA( 'StatusEffect' ) )
			{				
				Attached[i].Destroy();
				continue;
			}
			i++;
		}
		ShieldRechargeRate = default.ShieldRechargeRate;
		SetShields(MaxShields);
		Controller.bGodMode = false;
	}

	function PlayFallDown()
	{
		if (!bIncapacitatedOnTurret)
		{
			if( IsHumanControlled() )
				PlayAnim('PlayerWoundedFallDown');
			else
				PlayAnim('WoundedFallDown');
		}
	}
	function Timer()
	{	
		Log("Incapacitated timer calling BleedOut");
		BleedOut();
	}

Begin:
	if ( Controller != None )		
		Controller.PawnIncapacitated(self);		
	Health = 0;	
	ShieldRechargeRate = 0;
	SetShields(0);
	Controller.bGodMode = true;
	bEnableAim = false;
	bEnableTurn = false;	
	Velocity = vect(0,0,0); //set our velocity to zero	
	Acceleration = vect(0,0,0);
	Weapon.ServerStopFire();	
	HookupReviveMarker();

	// NathanM: This check is a hack that needs to be before the death anim
	// to prevent the SBD from getting stuck on guys that've jump died from getting bumped into
	// I only moved the set collision call in this case for fear of accidentally introducing other, scary bugs
	if( LastHitDamageType.IsA('CTDamageSpiderBump') )
		SetCollision(true,false,false);

	if( !LastHitDamageType.default.DisallowDeathAnim )
	{
		PlayFallDown();	
		FinishAnim();
	}
	bIncapacitatedOnTurret=false;
	bWantsToCrouch = true;
	PostStimulusToWorld(ST_DiedDefault);

	if( !LastHitDamageType.IsA('CTDamageSpiderBump') )
		SetCollision(true,false,false);

	if( IsHumanControlled() )
		LoopAnim('PlayerImmobileBreathe');
	else
		LoopAnim('ImmobileBreathe');
	if (BleedOutTime > 0)
		SetTimer( BleedOutTime, false );
}

State Dispense
{
	simulated function BeginState()
	{
		PlayAnim('DispUnfold',,,,4);
	}

	function AnimEnd(int Channel)
	{
		if (Channel == 4)
		{			
			bCollideWorld = true;
			SetCollision(true, true, true);
			SetPhysics(PHYS_Walking);			
			bPhysicsAnimUpdate = true;
			bAlignBottom = true;
			TeamIndex = Default.TeamIndex;
			SetShields( Default.MaxShields );
			if( Controller != None )
			{
				Controller.Trigger( self, self );
			}
			GotoState('');
		}
	}
}

//
//	HEALTH AND SHIELDS
//	Recharging, taking damage, setting health level and color
//
simulated event Tick(float DeltaTime )	// Restore the charge
{
	if (Role != ROLE_Authority)
	{
		// If a client and this pawn was hit set its LastHitLocalTime and set bWasHit to false
		if ( bWasHit )
		{
			bWasHit = false;
			LastHitLocalTime = Level.TimeSeconds;
		}

		return;
	}

	// Reset Hit Functions
	if (( bWasHitWasSent == true ) && ( bWasHit == true ))
	{
		bWasHitWasSent = false;
		bWasHit = false;
	}

	if ( !bSetDamageThisFrame )
		bDidDamageThisFrame = false;

	bSetDamageThisFrame = false;

	super.Tick(DeltaTime);

	EnemyKilledThisFrame = None;

	if (Level.TimeSeconds > NextShieldRechargeTime && Shields < MaxShields)
		SetShields( fclamp(Shields + (ShieldRechargeRate * DeltaTime), 0, MaxShields) );

	// Shadows done here now so that options can be changed in real time
	if ( Level.NetMode != NM_DedicatedServer )
	{
		if( bActorShadows && Level.ShadowsEnabled )
		{
			if( Shadow == None )
			{
				Shadow = Spawn(class'ShadowProjector',self,'',Location);
				ShadowProjector(Shadow).ShadowActor = self;
				ShadowProjector(Shadow).LightDirection = Normal(vect(1,1,3));
				ShadowProjector(Shadow).LightDistance = 380;
				ShadowProjector(Shadow).MaxTraceDistance = 350;
			}
			if( ShadowProjector(Shadow).ShadowTexture == None )
				ShadowProjector(Shadow).InitShadow();
		}
		else if ( Shadow != None )
		{
			Shadow.Destroy();
		}
	}	
}

//change these to native events?
simulated function bool IsDead()
{
	return (Health <=0 && !bIncapacitated && !bIncapacitatedOnTurret);
}

simulated event bool IsDeadOrIncapacitated()
{
	return (HealthLevel == HL_Red);
}

//would use native events, but the auto-generate code doesn't make function const
simulated native function bool IsWounded(); 
simulated native function bool UseWoundedAnims();
simulated native function bool IsLocallyControlled(); // return true if controlled by local (not network) player

event SetShields( float NewShields )
{
	Shields = NewShields;

	if( Shields > 0 )
		TypeOfMaterial = MT_PersonalShield;
	else
		TypeOfMaterial = default.TypeOfMaterial;
}


simulated event SetHealthLevel()
{
	local float HealthRatio;
	local EHealthLevel OldHealthLevel;

	OldHealthLevel = HealthLevel;
	HealthRatio = Health/MaxHealth;

	HealthLevel = CalcHealthLevel(HealthRatio);
	
	if( HealthLevel != OldHealthLevel && HealthAffectsGameplay )
	{		
		ChangeAnimation();
	}	
}

const REDDELAY = 15.0;
function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	local int i;
	local float actualDamage;
	local bool bAlreadyDead;
	local bool bHurt;
	local float delay;
	local Controller Killer;
	local EHealthLevel OldHealth;
	local name ImpactAnim;


	if ( Role < ROLE_Authority )
		return 0;
	
	// apply bone multiplier regardless of shield state
	for( i = 0; i < DamageMultipliers.length; ++i )
	{
		// We have to check for '' because it's possible to have a bogus entry in the list
		if( DamageMultipliers[i].BoneName != '' && DamageMultipliers[i].BoneName == BoneName )
		{
			Damage *= DamageMultipliers[i].Multiplier;
			break;
		}
	}

	Damage *= 1.0 - ArmorFactor;

	// So the HUD of the instigator knows to flash the damage indicator
	if ( instigatedBy != None )
	{
		instigatedBy.bDidDamageThisFrame = true;
		instigatedBy.bSetDamageThisFrame = true;
	}

	// Check to make sure we're actually taking damage
	if( Shields > 0 && ShieldDamageVariance != None )
		actualDamage = CalculateDamageFrom(DamageType, Damage, ShieldDamageVariance);
	else
		actualDamage = CalculateDamageFrom(DamageType, Damage);

	// figure out actual damage reduced by any game rules.
	actualDamage = Level.Game.ReduceDamage(ActualDamage, self, instigatedBy, HitLocation, Momentum, DamageType);

	if (actualDamage <= 0)
		return 0;
	
	if ( damagetype == None )
	{
		if ( InstigatedBy != None )
			warn("No damagetype for damage by "$instigatedby$" with weapon "$InstigatedBy.Weapon);
		DamageType = class'DamageType';
	}

	LastHitDamageType = DamageType;

	if( FirstDamagedEvent != '' )
	{
		TriggerEvent( FirstDamagedEvent, self, self );
		FirstDamagedEvent = '';
	}	

	bWasHit = true;
	LastHitLocalTime = Level.TimeSeconds;
	if( instigatedBy != None && instigatedBy != self )
	{
		LastHitBy = instigatedBy;
		LastHitBone = BoneName;
		LastHitLocation = hitlocation - GetBoneLocation(LastHitBone);
		LastHitLocation = LastHitLocation >> GetBoneRotation(LastHitBone);

		// If pawn is not dying, (dying is handled elsewhere)
		if( !bPlayedDeath )
		{
			if( bHurt )
				// Show visor damage
				HitDamageType = DamageType;
			else
				HitDamageType = None;
		}
	}

	if( !damageType.default.bBypassesShields)
	{
		SetShields( Shields - actualDamage ); //apply damage to shields
		delay = ShieldRechargeDelay;		
		NextShieldRechargeTime = max(Level.TimeSeconds + ShieldRechargeDelay, NextShieldRechargeTime); //delay shield recharge any time the shields are hit by 1 second
		if( Shields < 0 )
		{
			actualDamage = -Shields;
			Shields = 0;			
		}
		else
		{
			Controller.NotifyTakeHit(instigatedBy, HitLocation, Damage, DamageType, Momentum);
			return 0;
		}
	}

	bAlreadyDead = (Health <= 0);

	if( Physics == PHYS_None && CanChangeAnims() )
	{
		SetMovementPhysics();
	}

	Health -= actualDamage;
	
	if( Shields > 0 && !DamageType.default.bBypassesShields )
	{
		if( Level.TimeSeconds - LastShieldDamageSoundTime >= MinShieldDamageSoundInterval )
		{
			LastShieldDamageSoundTime = Level.TimeSeconds;
			PlaySound( ShieldDamageSound );
		}
		bHurt = false;
	}
	else 
	{
		if( Level.TimeSeconds - LastYelpTime >= MinYelpTime )
		{
			LastYelpTime = Level.TimeSeconds;
			PlayHurtCue(damageType);
		}

		// If we took a bunch of damage (>10%), or we are really low health (<40%)
		if( actualDamage > MaxHealth * 0.1f || (actualDamage > 0.f && Health < MaxHealth * 0.4f) )
			// Show visor damage
			bHurt = true;
	}	

	// TimR: Added for testing low health states
	if( Health <= 0 && InGodMode() )
	{
		Health = 0.5;
	}

	OldHealth = HealthLevel;
	SetHealthLevel();
	
	if( !IsHumanControlled() )
	{
		if( ( HealthLevel == HL_Orange || HealthLevel == HL_Red ) && (OldHealth == HL_Yellow || OldHealth == HL_Green) )
		{
			PlayOwnedCue(PAE_DropToOrange);		
		}

		if( !bDisallowPainAnims )
		{
			if( ActualDamage >= MinStaggerDamage && Level.TimeSeconds - LastStaggerTime >= MinStaggerInterval && CanChangeAnims() )
			{
				Velocity = vect(0,0,0);
				Acceleration = vect(0,0,0);
				AIController(Controller).AddReflexAnimGoal('Hit');
				if( Weapon != None )
					Weapon.StopFire(0);

				LastStaggerTime = Level.TimeSeconds;
			}
			else if( BoneName != '' &&
					 MinImpactInterval != -1 &&
					 actualDamage >= MinImpactDamage && Level.TimeSeconds - LastImpactTime >= MinImpactInterval )
			{
				if( !IsShutdown() && !IsManningTurret() )
				{
					ImpactAnim = 'Impact';
					/*
					for( i = 0; i < ImpactAnims.length; i++ )
					{
						if( BoneName == ImpactAnims[i].BoneName )
						{
							ImpactAnim = ImpactAnims[i].Animation;
							break;
						}
					}
					*/
					PlayAnim( ImpactAnim, 'fire', , , 4 );
					LastImpactTime = Level.TimeSeconds;
				}
			}
		}
	}

	if ( HitLocation == vect(0,0,0) )
		HitLocation = Location;
	if ( bAlreadyDead )
	{		
		Warn(self$" took regular damage "$damagetype$" from "$instigatedby$" while already dead at "$Level.TimeSeconds);
		ChunkUp(Rotation, DamageType);
		return 0;
	}
	
	if ( Health <= 0 )
	{
		// pawn died
		if ( instigatedBy != None )
		{
			if (instigatedBy.Controller == None)
				Killer = instigatedBy.GetRecentController();
			else
				Killer = instigatedBy.Controller;
	
			instigatedBy.EnemyKilledThisFrame = self;
		}
	

		if ( bPhysicsAnimUpdate )		
			TearOffMomentum = momentum;					
		Died(Killer, damageType, HitLocation, BoneName);
	}
	else
	{
		if ( Controller != None )
			Controller.NotifyTakeHit(instigatedBy, HitLocation, actualDamage, DamageType, Momentum);
	}

	// NathanM: This is not correct in the case of shields...
	return ActualDamage;
}

//
// WEAPONS AND INVENTORY
//

event DoMeleeAttack( name AttackBone, vector BoneOffset )
{
	local Actor HitActor;	
	local vector VecToOther, BodyDir, HitLocation;
	local name NearestBone;

	BodyDir = vector( Rotation );
	BodyDir.Z = 0;

	if( AttackBone == '' )
		AttackBone = GetWeaponBoneFor( Weapon );

	HitLocation = GetBoneLocation( AttackBone );
	HitLocation += BoneOffset << GetBoneRotation( AttackBone ) ;

	ForEach VisibleCollidingActors( class'Actor', HitActor, Weapon.MeleeRange )
	{				
		if( abs( HitActor.Location.Z - Location.Z ) <= ( Max( HitActor.CollisionHeight, CollisionHeight ) * 1.5 ) )
		{
			VecToOther = HitActor.Location - Location;
			VecToOther.Z = 0;
			VecToOther = Normal( VecToOther );

			if( !HitActor.IsA('Accessory') && VecToOther Dot BodyDir > 0.8 )
			{
				HitActor.TakeDamage( Weapon.MeleeDamage, self, HitActor.Location, VecToOther * Weapon.MeleeDamageType.default.KDamageImpulse, Weapon.MeleeDamageType );

				// We are checking None and bDeleteMe here because it's possible that TakeDamage destroyed the actor we were just working over
				if( HitActor != None && !HitActor.bDeleteMe )
				{
					if( Weapon.MeleeHitEffectsByMaterial != None )
					{
						NearestBone = HitActor.FindNearestBone( HitLocation );
						Weapon.MeleeHitEffectsByMaterial.Static.SpawnEffectsFor( HitActor, HitActor.TypeOfMaterial, HitActor.GetBoneLocation( NearestBone ), -VecToOther );
					}

					if( Weapon.MeleeStatusEffect != None && HitActor.IsA('Pawn') )
						Weapon.MeleeStatusEffect.Static.AddStatusEffectTo( HitActor, self, Weapon.MeleeStatusEffectDuration, Weapon.MeleeStatusEffectDamagePerSec );
				}
			}
		}
	}	
}

function AddDefaultInventory()
{
	local int i;

	//CL: Add Default Equipment
	for ( i=0; i<16; i++ )
		if ( RequiredEquipment[i] != "" )	
			CreateInventory(RequiredEquipment[i]);			
	
	Level.Game.AddGameSpecificInventory(self);

	// HACK FIXME
	if ( inventory != None )
		inventory.OwnerEvent('LoadOut');
	
	Controller.ClientSwitchToBestWeapon();

    SpawnAccessories();
}

function Inventory CreateInventory(string InventoryClassName)
{
	local Inventory Inv;
	local class<Inventory> InventoryClass;

	InventoryClass = Level.Game.BaseMutator.GetInventoryClass(InventoryClassName);
	if( (InventoryClass!=None) && (FindInventoryType(InventoryClass)==None) )
	{
		Inv = Spawn(InventoryClass,self);		
		if( Inv != None )
		{			
			Inv.GiveTo(self);
			if ( Inv != None )
			Inv.PickupFunction(self);
			if ( Inv.IsA('Weapon') )
			{
				if ( Weapon(Inv).WeaponType == WT_Secondary && Weapon(Inv).bCanThrow )
					CurrentTossableWeapon = Weapon(Inv);
				if ( (CurrentGrenade == None) && (Weapon(Inv).WeaponType == WT_Thrown) )
				{
					CurrentGrenade = Weapon(Inv);
				}
			}
			return Inv;
		}
	}
	return None;
}

function SpawnAccessories()
{
	local int i;
	local int NumAvailableAccessories;	
	local bool StopSpawningAccessories;

	NumAvailableAccessories = AvailableAccessories.length;
	
	for( i = 0; i < AvailableAccessories.length && Accessories.length < MaxAccessories; ++i )
	{
		if( AvailableAccessories[i].IsRequired && IsBoneAvailable( AvailableAccessories[i].BoneName ) )
		{
			AddAccessory( i );
			AvailableAccessories[i].HasBeenUsed = true;
			--NumAvailableAccessories;
		}
	}

	while( Accessories.length < MaxAccessories && NumAvailableAccessories > 0 )
	{
		
		i = rand( AvailableAccessories.length );
		if( !AvailableAccessories[i].HasBeenUsed && IsBoneAvailable( AvailableAccessories[i].BoneName ) )
		{
			AddAccessory( i );
			AvailableAccessories[i].HasBeenUsed = true;
			--NumAvailableAccessories;
		}

		StopSpawningAccessories = true;
		for( i = 0; i < AvailableAccessories.length; ++i )
		{
            if( !AvailableAccessories[i].HasBeenUsed && IsBoneAvailable( AvailableAccessories[i].BoneName ) )
			{
				StopSpawningAccessories = false;
				break;
			}
		}

		if( StopSpawningAccessories )
			return;
	}
}

simulated function AddAccessory( int which )
{
	local AccessoryInfo newAccessoryInfo;	

	if (which < 0 || which >= AvailableAccessories.length)
		return;

	newAccessoryInfo = AvailableAccessories[which];

	Accessories.Insert(0,1);
	Accessories[0] = spawn( newAccessoryInfo.AccessoryClass, self );
	AttachToBone( Accessories[0], newAccessoryInfo.BoneName );
	Accessories[0].SetRelativeLocation( vect(0,0,0) );
	Accessories[0].SetRelativeRotation( rot(0,0,0) );	
}

function Detach( Actor Other )
{
	local int i;

	for( i = 0; i < Accessories.length; ++i )
	{
		if( Other == Accessories[i] )
		{			
			Accessories.Remove( i, 1 );
			break;
		}
	}
}

function bool IsBoneAvailable( name BoneName )
{
	local int i;

	for( i = 0; i < Accessories.Length; ++i )
	{
		if( Accessories[i].AttachmentBone == BoneName )		
			return false;		
	}

	return true;
}

simulated event rotator GetWeaponRotation()
{
	return GetBoneRotation(GetWeaponBoneFor(Weapon));
}

simulated function name GetWeaponBoneFor(Inventory I)
{
	return 'weaponAttach_R';
}

//
//	TEAM FUNCTIONS
//

//  REVISIT - we could move these functions up to Pawn or further up
//	hierarchy -- we may want to consider non-Pawns as enemies, allies, etc.
//	Make team functions simulated? shouldn't really be necessary
//  since the logic probably only matters on the server, but there will
//  likely be the info necessary to compute them on the client 


simulated function bool IsEnemy(Pawn Other)
{
	return (GetRelationTowards(Other) == TEAM_Enemy);
}

simulated function bool IsNotEnemy(Pawn Other)
{
	return (GetRelationTowards(Other) != TEAM_Enemy);
}

simulated function bool IsAlly(Pawn Other)
{
	return (GetRelationTowards(Other) == TEAM_Ally);
}

simulated function bool IsNotAlly(Pawn Other)
{
	return (GetRelationTowards(Other) != TEAM_Ally);
}

simulated function bool IsNeutral(Pawn Other)
{
	return (GetRelationTowards(Other) == TEAM_Neutral);
}

event function PlayDeathCue(class<DamageType> damageType)
{
	StopTalking();
	if( CurrentIdleState != AS_Shutdown )
		PlayOwnedCue(damageType.default.DeathAudioEvent);
}

event PlaySpottedCue(Actor Enemy)
{
	if (Enemy.IsA('Pawn'))
		PlayOwnedCue(Pawn(Enemy).EnemySpottedCue, 5.0);
}

simulated function PlayHurtCue(class<DamageType> damageType)
{
	local Sound hurtSound;
	hurtSound = GetSoundFromCue(damageType.default.HurtAudioEvent);
	if ( hurtSound != None && CurrentIdleState != AS_Shutdown )
		PlaySound(hurtSound);
}

simulated event String GetHudDescription()
{
	return "00";
}

simulated event String GetHudNickname()
{
	return "";
}


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AIScriptTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

function float GetDifficultyMod()
{
	return 1.0;
}

simulated function SetHudArmTexture(Weapon theWeapon);


defaultproperties
{
     bIsWalking=True
     bPhysicsAnimUpdate=True
     bFallOnDeath=True
     bCanCrouch=True
     bCanWalk=True
     bCanRun=True
     bCanStrafe=True
     bUseCompressedPosition=True
     bTossWeaponOnDeath=True
     bPerceivedAsThreat=True
     bSameZoneHearing=True
     bAdjacentZoneHearing=True
     bMuffledHearing=True
     bWeaponBob=True
     bEnableTurn=True
     bEnableAim=True
     MaxHealth=100
     ShieldRechargeDelay=1
     HealthLevel=HL_Green
     DamageMultipliers(0)=(BoneName="Head",Multiplier=2)
     MinImpactDamage=15
     MinImpactInterval=0.1
     MinStaggerDamage=100
     MinStaggerInterval=0.5
     DropGrenadePercentage=0.5
     MaxAccessories=2
     VisionPawnUser(0)=VisionModeNormal'FrameFX.VisionModes.VisionNormal'
     VisionPawnUser(1)=VisionModeEnhance'FrameFX.VisionModes.VisionEnhance'
     VisionPawnUser(2)=VisionModeSniper'FrameFX.VisionModes.VisionNight'
     VisionPawnHit=VisionModeHit'FrameFX.VisionModes.VisionHit'
     VisionPawnIncap=VisionModeDistort'FrameFX.VisionModes.VisionIncap'
     VisionPawnScavDrill=VisionModeScavDrill'FrameFX.VisionModes.VisionScavDrill'
     SightRadius=10000
     PeripheralVision=0.2
     AutoDetectRadius=200
     BaseEyeHeight=72
     EyeHeight=72
     ControllerClass=Class'Engine.AIController'
     Visibility=128
     PatrolPriority=0.3
     Accuracy=0.5
     MaximumDesiredSpeed=1000
     MaxDeathLoopTime=2
     LastValidAnchorTime=-0.25
     AvgPhysicsTime=0.1
     LandMovementState="PlayerWalking"
     WaterMovementState="PlayerSwimming"
     CrouchHeight=39
     CrouchRadius=40
     FlyingRollMult=1
     GroundSpeed=600
     WaterSpeed=300
     AirSpeed=600
     LadderSpeed=200
     MaxFallSpeed=1200
     AccelRate=2048
     AirControl=0.35
     WalkSpeedRatio=0.5
     CrouchSpeedRatio=0.5
     WoundedSpeedRatio=0.833
     BackSpeedRatio=0.6
     SideSpeedRatio=0.8
     AimSpeed=200
     DecelRate=600
     MinCorrectedSpeed=150
     MaxAnalogOverdriveRatio=1
     CurrentIdleState=AS_Relaxed
     MaxAimYaw=80
     MaxAimPitch=80
     MaxLookYaw=80
     MaxLookPitch=80
     MinYelpTime=0.5
     MinShieldDamageSoundInterval=0.1
     EnemySpottedCue=PAE_SpottedEnemy
     KilledEnemyCue=PAE_KilledExcited
     RagImpactSoundInterval=0.5
     RagImpactVolume=2.5
     RagdollLifeSpan=60
     Bob=0.008
     DamageScaling=1
     Physics=PHYS_Walking
     DrawType=DT_Mesh
     bReceiveDynamicShadows=False
     bCriticalAttachment=True
     bUpdateSimulatedPosition=True
     bUseHWOcclusionTests=True
     bTravel=True
     bCanBeDamaged=True
     bShouldBaseAtStartup=True
     bOwnerNoSee=True
     bCanTeleport=True
     bDisturbFluidSurface=True
     bCanBeAutoAimed=True
     bAlignBottom=True
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     bBlockKarma=True
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=2
     Texture=Texture'Engine.S_Pawn'
     CollisionRadius=40
     CollisionHeight=78
     Buoyancy=99
     RotationRate=(Yaw=20000,Roll=2048)
     Begin Object Class=KarmaParamsSkel Name=PawnKParams
         KConvulseSpacing=(Max=2.2)
         KLinearDamping=0.15
         KAngularDamping=0.05
         KBuoyancy=1
         KStartEnabled=True
         KVelDropBelowThreshold=50
         bHighDetailOnly=False
         KFriction=1.5
         KRestitution=0.1
         KImpactThreshold=400
         Name="PawnKParams"
     End Object
     KParams=KarmaParamsSkel'Engine.Pawn.PawnKParams'
     bNoRepMesh=True
     bDirectional=True
     bValidEnemy=True
}

