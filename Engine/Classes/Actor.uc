//=============================================================================
// Actor: The base class of all actors.
// Actor is the base class of all gameplay objects.  
// A large number of properties, behaviors and interfaces are implemented in Actor, including:
//
// -	Display 
// -	Animation
// -	Physics and world interaction
// -	Making sounds
// -	Networking properties
// -	Actor creation and destruction
// -	Triggering and timers
// -	Actor iterator functions
// -	Message broadcasting
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Actor extends Object
	abstract
	native
	nativereplication;

// Imported data (during full rebuild).
#exec Texture Import File=Textures\S_Actor.pcx Name=S_Actor Mips=Off MASKED=1
#exec Texture Import File=Textures\LockLocation.pcx Name=S_LockLocation Mips=Off MASKED=1
#exec Texture Import File=Textures\AutoAlignToTerrain.pcx Name=S_AutoAlignToTerrain Mips=Off MASKED=1

// cg hidden until finished. 
var(Lighting) BYTE	ShadowAmbient;

// Light modulation.
var(Lighting) enum ELightType
{
	LT_None,
	LT_Steady,
	LT_Pulse,
	LT_Blink,
	LT_Flicker,
	LT_Strobe,
	LT_BackdropLight,
	LT_SubtlePulse,
	LT_TexturePaletteOnce,
	LT_TexturePaletteLoop,
	LT_FadeOut,
	LT_Flame
} LightType;

// Spatial light effect to use.
var(Lighting) enum ELightEffect
{
	LE_None,
	LE_TorchWaver,
	LE_FireWaver,
	LE_WateryShimmer,
	LE_Searchlight,
	LE_SlowWave,
	LE_FastWave,
	LE_CloudCast,
	LE_StaticSpot,
	LE_Shock,
	LE_Disco,
	LE_Warp,
	LE_Spotlight,
	LE_NonIncidence,
	LE_Shell,
	LE_OmniBumpMap,
	LE_Interference,
	LE_Cylinder,
	LE_Rotor,
	LE_Sunlight,
	LE_QuadraticNonIncidence
} LightEffect;


//This is what is used to determine how to respond to a stimulus
const SG_MAX = 19; //make sure this stays in sync w/ number of StimulusGroup enums (it's needed to create a fixed-size array in UnrealScript)
enum EStimulusGroup
{
	SG_WeaponFire,				//someone fired a weapon
	SG_WeaponHit,				//a projectile hit something
	SG_CharacterNoise,			//used for footsteps
	SG_Stunned,					//when hit buy a stun weapon
	SG_Explosion,				//something blew up
	SG_GrenadeBounce,			//a grenade bounced
	SG_Died,					//a death scream
	SG_DoorBreach,				//someone performed a door breach
	SG_Reloading,				//The AIs weapon is out of ammo
	SG_Flashlight,				//when the AI sees the player (unused)
	SG_Damaged,					//we were damaged
	SG_EnemySpotted,			//Enemy spotted
	SG_Triggered,				//this actor was triggered
	SG_Bumped,					//this actor ran into someone	
	SG_Targeted,				//a projectile has been spawned that we might want to dodge
	SG_LostEnemy,				//we lost our quarry
	SG_KilledSomeone,			//we just killed someone
	SG_ExplosionLarge,			//a very large explosion
	SG_MineChirp,				//a chirp from a mine/trap
};



enum EStimulusType {
	ST_Invalid,					//the invalid stimulus type
	ST_WeaponFireDefault,		//for typical weapons
	ST_WeaponFireQuiet,			//for silenced weapons
	ST_WeaponHitDefault,		//a weapon hit a wall or person
	ST_CharacterNoiseDefault,	//unused
	ST_FootstepSoft,			//for walk, crouch footsteps
	ST_FootstepLoud,			//for run footsteps
	ST_Stunned,					//when hit buy a stun weapon
	ST_ExplosionDefault,		//for typical explosions
	ST_ExplosionLarge,			//for demolition
	ST_GrenadeBounceDefault,	//a grenade bounced
	ST_DiedDefault,				//a regular death scream
	ST_DiedQuiet,				//a death scream from a silent kill weapon
	ST_DoorBreach,				//someone breached this door
	ST_DemolitionExplode,		//demolition explode
	ST_ProxMineExplode,			//proxmine explode
	ST_MineChirp,				//a mine chirp
	ST_JetpackExplosion,		//a Trandoshan just flew
	ST_Reloading,				//the AIs weapon is out of ammo
	ST_Flashlight,				//when the AI sees someone (unused)
	ST_Damaged,					//we were damaged
	ST_EnemySpotted,			//this enemy was spotted
	ST_Triggered,				//this actor was triggered
	ST_Bumped,					//this actor ran into someone
	ST_Targeted,				//a projectile has been spawned that we might want to dodge
	ST_LostEnemy,				//this actor lost sight of his enemy
	ST_KilledSomeone,			//we just killed someone
};

//This is the actual response type to a stimulus. It is based on
//the StimulusGroup of the highest priority stimulus
enum EStimulusResponse {
	SR_None,
	SR_WalkToward,
	SR_RunToward,
	SR_RunAway,
	SR_LookAt,
	SR_Crouch,	
	SR_TakeCover,
	SR_Duck,
	SR_Dodge,
	SR_DiveAway,
	SR_ReturnGrenade,
	SR_Bump,
	SR_GrenadeReaction,
	SR_CheckCorpse,
	SR_MeleeStrike,
	SR_Follow,
	SR_ExplosionShield,
	SR_LookAtInstigator,
	SR_RunTowardInstigator,
};

// A convenient enum for directions
enum EDirection
{
	DIR_None,
	DIR_Up,
	DIR_Down,
	DIR_Left,
	DIR_Right,
	DIR_Forward,
	DIR_Backward,
	DIR_Auto,	//choose automatically
};

//	Detail mode enum.

enum EDetailMode
{
	DM_Low,
	DM_High,
	DM_SuperHigh
};

//
// SOUND ENUMS

// Sound occlusion
enum ESoundOcclusion
{
	OCCLUSION_Default,
	OCCLUSION_None,
	OCCLUSION_BSP,
	OCCLUSION_StaticMeshes,
};

// Sound slots for actors.
enum ESoundSlot
{
	SLOT_None,
	SLOT_Misc,
	SLOT_Pain,
	SLOT_Ambient,
	SLOT_Interact,
	SLOT_Talk,
	SLOT_Interface,
	SLOT_Music,
};


// Music transitions.
enum EMusicTransition
{
	MTRAN_None,
	MTRAN_Instant,
	MTRAN_Segue,
	MTRAN_Fade,
	MTRAN_FastFade,
	MTRAN_SlowFade,
};


// Enum for material types
// WARNING: ONLY ADD VALUES AT THE END. DO NOT INSERT VALUES.
enum EMaterialType {
	MT_None,
	MT_Rock,
	MT_Dirt,
	MT_Mud,
	MT_MetalSolid,
	MT_MetalGrate,
	MT_MetalHollow,
	MT_Sand,
	MT_Concrete,
	MT_Wood,
	MT_Forest,
	MT_Brush,
	MT_Puddle,
	MT_Water,
	MT_Marble,
	MT_Debris_GEO,
	MT_Debris_RAS,
	MT_Debris_YYY, 
	MT_Glass,
	MT_HumanFlesh,
	MT_TrandoFlesh,
	MT_BugFlesh,
	MT_DroidMetal,
	MT_WookieeFlesh,
	MT_Shield,
	MT_PersonalShield,
	MT_MetalPipe,
	MT_EliteFlesh,
	MT_NoEffectSpawned,
	MT_ShieldGreen
};
// WARNING: ONLY ADD VALUES AT THE END. DO NOT INSERT VALUES.


//enum for music battle engine
enum EBattleEngineBattleCriteria
{
	BATTLECRITERIA_Attack,
	BATTLECRITERIA_Proximity,
	BATTLECRITERIA_AttackAndProximity
};

//enum for music fade types
enum EMusicFadeType
{
	MFT_Linear,
	MFT_Sine,
	MFT_Log,
	MFT_LogInverse
};

// Lighting info.
var(LightColor) float
	LightBrightness;
var(Lighting) float
	LightRadius;
var(Lighting)byte 
	LightRadiusInner;		
var(LightColor) byte
	LightHue,
	LightSaturation;
var(Lighting) byte
	LightPeriod,
	LightPhase,
	LightCone;

// Priority Parameters
// Actor's current physics mode.
var(Movement) const enum EPhysics
{
	PHYS_None,
	PHYS_Walking,
	PHYS_Falling,
	PHYS_Swimming,
	PHYS_Flying,
	PHYS_Rotating,
	PHYS_Projectile,
	PHYS_Interpolating,
	PHYS_MovingBrush,
	PHYS_Spider,
	PHYS_Trailer,
	PHYS_Ladder,
	PHYS_RootMotion,
    PHYS_Karma,
    PHYS_KarmaRagDoll
} Physics;

// Drawing effect.
var(DisplayAdvanced) const enum EDrawType
{
	DT_None,
	DT_Sprite,
	DT_Mesh,
	DT_Brush,
	DT_RopeSprite,
	DT_VerticalSprite,
	DT_Terraform,
	DT_SpriteAnimOnce,
	DT_StaticMesh,
	DT_DrawType,
	DT_Particle,
	DT_AntiPortal,
	DT_FluidSurface
} DrawType;

var(DisplayAdvanced) const StaticMesh StaticMesh;		// StaticMesh if DrawType=DT_StaticMesh

// Owner.
var const Actor	Owner;			// Owner actor.
var const Actor	Base;           // Actor we're standing on.

struct noexport ActorRenderDataPtr { var int Ptr; };
struct noexport LightRenderDataPtr { var int Ptr; };

var const native ActorRenderDataPtr	ActorRenderData;
var const native LightRenderDataPtr	LightRenderData;
var	const native int				RenderRevision;

enum EFilterState
{
	FS_Maybe,
	FS_Yes,
	FS_No
};

var const native EFilterState	StaticFilterState;

struct BatchReference
{
	var int	BatchIndex,
			ElementIndex;
};

var const native array<BatchReference>	StaticSectionBatches;
var(DisplayAdvanced) const name	ForcedVisibilityZoneTag; // Makes the visibility code treat the actor as if it was in the zone with the given tag.

// Lighting.
var			  bool		 bLightPriorityOverride; //used in hud spot light, but can be used with any light
var			  bool       bLightObstructed;
var(Lighting) bool	     bSpecialLit;			// Only affects special-lit surfaces.
var(DisplayAdvanced) bool	     bActorShadows;			// Light casts actor shadows.
var(Lighting) bool	     bReceiveDynamicShadows; // Whether it is affected by the dynamic shadows or not (all pawns by default)
var(Lighting) bool	     bBlobShadow;			// Use just a blob instead of dynamic shadows.
var(Lighting) bool	     bCorona;			   // Light uses Skin as a corona.
var(Lighting) bool		 bLightingVisibility;	// Calculate lighting visibility for this actor with line checks.
var(Lighting) bool		 bDisableFog;			// Disable fog on this object
var(DisplayAdvanced) bool		 bUseDynamicLights;
var bool				 bLightChanged;			// Recalculate this light's lighting now.

var(Lighting) bool		bLightStaticEnv;		
var(Lighting) bool		bLightDynamicEnv;

// Flags.
var(Advanced) const bool	bStatic;			// Does not move or change over time. This is needed for dynamic light effects but should be changed rarely.
var(Advanced)		bool	bHidden;			// Is hidden during gameplay.
var(Advanced) const bool	bNoDelete;			// Cannot be deleted during play.
var			  const	bool	bDeleteMe;			// About to be deleted.
var transient const bool	bTicked;			// Actor has been updated.
var(Lighting)		bool	bDynamicLight;		// This light is dynamic.
var					bool	bTimerLoop;			// Timer loops (else is one-shot).
var					bool    bOnlyOwnerSee;		// Only owner can see this actor.
var(Advanced)		bool    bHighDetail;		// Only show up in high or super high detail mode.
var(Advanced)		bool	bSuperHighDetail;	// Only show up in super high detail mode.
var					bool	bOnlyDrawIfAttached;	// don't draw this actor if not attached (useful for net clients where attached actors and their bases' replication may not be synched)
var					bool	bCriticalAttachment; // This attachment should always update, even if it's parent hasn't been rendered for a while
var(Advanced)		bool	bStasis;			// In StandAlone games, turn off if not in a recently rendered zone turned off if  bStasis  and physics = PHYS_None or PHYS_Rotating.
var					bool	bTrailerAllowRotation; // If PHYS_Trailer and want independent rotation control.
var					bool	bTrailerSameRotation; // If PHYS_Trailer and true, have same rotation as owner.
var					bool	bTrailerPrePivot;	// If PHYS_Trailer and true, offset from owner by PrePivot.
var					bool	bWorldGeometry;		// Collision and Physics treats this actor as world geometry
var(DisplayAdvanced) bool    bAcceptsProjectors;	// Projectors can project onto this actor
var					bool	bOrientOnSlope;		// when landing, orient base on slope of floor
var			  const	bool	bOnlyAffectPawns;	// Optimisation - only test ovelap against pawns. Used for influences etc.
var(DisplayAdvanced) bool	bDisableSorting;	// Manual override for translucent material sorting.
var(MovementAdvanced) bool	bIgnoreEncroachers; // Ignore collisions between movers and 

var					bool    bShowOctreeNodes;
var					bool    bWasSNFiltered;      // Mainly for debugging - the way this actor was inserted into Octree.
var					bool	bMovedInEditor;		//whether this actor was moved in the editor
// Networking flags
var			  const	bool	bNetTemporary;				// Tear-off simulation in network play.
var					bool	bOnlyRelevantToOwner;			// this actor is only relevant to its owner.
var transient const	bool	bNetDirty;					// set when any attribute is assigned a value in unrealscript, reset when the actor is replicated
var					bool	bAlwaysRelevant;			// Always relevant for network.
var					bool	bReplicateInstigator;		// Replicate instigator to client (used by bNetTemporary projectiles).
var					bool	bReplicateMovement;			// if true, replicate movement/location related properties
var					bool	bSkipActorPropertyReplication; // if true, don't replicate actor class variables for this actor
var					bool	bUpdateSimulatedPosition;	// if true, update velocity/location after initialization for simulated proxies
var					bool	bTearOff;					// if true, this actor is no longer replicated to new clients, and 
														// is "torn off" (becomes a ROLE_Authority) on clients to which it was being replicated.
var					bool	bOnlyDirtyReplication;		// if true, only replicate actor if bNetDirty is true - useful if no C++ changed attributes (such as physics) 
														// bOnlyDirtyReplication only used with bAlwaysRelevant actors
var					bool	bReplicateAnimations;		// Should replicate SimAnim
var const           bool    bNetInitialRotation;        // Should replicate initial rotation
var					bool	bCompressedPosition;		// used by networking code to flag compressed position replication
var					bool	bAlwaysZeroBoneOffset;		// if true, offset always zero when attached to skeletalmesh
// DS_SHADER
var(Advanced) bool bDontBatch; // Prevent this actor from being batched 
// end DS_SHADER

// Display.
var(Display)  bool      bUnlit;					// Lights don't affect actor.
var(DisplayAdvanced)  bool      bShadowCast;			// Casts static shadows.
var(DisplayAdvanced)  bool		bStaticLighting;		// Uses raytraced lighting.
var(DisplayAdvanced)  bool		bUseLightingFromBase;	// Use Unlit/AmbientGlow from Base
var(Marker)			bool	bDrawHUDMarkerIcon;	// Determines if this actor, if associated with a marker, should have a HUD icon draw at its position
// Whether to use hardware occlusion culling or not
// Defaults to false. Overridden in Pawn, Emitter, StaticMeshActor
var(Display)		bool	bUseHWOcclusionTests;
var(Display)		bool	EnhancedVisionFade;

// Advanced.
var			  bool		bHurtEntry;				// keep HurtRadius from being reentrant
var(Advanced) bool		bGameRelevant;			// Always relevant for game
var(Advanced) bool		bCollideWhenPlacing;	// This actor collides with the world when placing.
var			  bool		bTravel;				// Actor is capable of travelling among servers.
var(Advanced) bool		bMovable;				// Actor can be moved.
var			  bool		bDestroyInPainVolume;	// destroy this actor if it enters a pain volume
var			  bool		bCanBeDamaged;			// can take damage
var(Advanced) bool		bShouldBaseAtStartup;	// if true, find base for this actor at level startup, if collides with world and PHYS_None or PHYS_Rotating
var			  bool		bPendingDelete;			// set when actor is about to be deleted (since endstate and other functions called 
												// during deletion process before bDeleteMe is set).

var 				bool	bOwnerNoSee;		// Everything but the owner can see this actor.
var(Advanced)		bool	bCanTeleport;		// This actor can be teleported.
var					bool	bClientAnim;		// Don't replicate any animations - animation done client-side
var					bool    bDisturbFluidSurface; // Cause ripples when in contact with FluidSurface.
var			  const	bool	bAlwaysTick;		// Update even when players-only.
var(Advanced)	bool		bCanBeAutoAimed;	// Whether or not we can automatically aim at this guy

// Collision flags.
var(CollisionAdvanced) bool       bAlignBottom;			// Actor aligned to bottom of cylinder.
var(CollisionAdvanced) const bool bCollideActors;		// Collides with other actors.
var(CollisionAdvanced) bool       bCollideWorld;		// Collides with the world.
var(CollisionAdvanced) bool       bBlockActors;			// Blocks other nonplayer actors.
var(CollisionAdvanced) bool       bBlockPlayers;		// Blocks other player actors.
var(CollisionAdvanced) bool       bProjTarget;			// Projectiles should potentially target this actor.
var(CollisionAdvanced) bool		  bBlockZeroExtentTraces; // block zero extent actors/traces
var(CollisionAdvanced) bool		  bBlockNonZeroExtentTraces;	// block non-zero extent actors/traces
var(CollisionAdvanced) bool       bAutoAlignToTerrain;  // Auto-align to terrain in the editor
var(CollisionAdvanced) bool		  bUseCylinderCollision;// Force axis aligned cylinder collision (useful for static mesh pickups, etc.)
var(CollisionAdvanced) const bool bBlockKarma;			// Block actors being simulated with Karma.
var					   bool		  bUseAltKarmaPrim;
var					   bool		  bImmobile;			// Used with TRACE_Immobile
var					   bool       bNoRagdollPawnCollision;

var       			bool    bNetNotify;                 // actor wishes to be notified of replication events

//-----------------------------------------------------------------------------
// Physics.

// Options.
var						bool		bIgnoreOutOfWorld; // Don't destroy if enters zone zero
var(MovementAdvanced)	bool		bBounce;           // Bounces when hits ground fast.
var(MovementAdvanced)	bool		bFixedRotationDir; // Fixed direction of rotation.
var(MovementAdvanced)	bool		bRotateToDesired;  // Rotate to DesiredRotation.
var						bool		bInterpolating;    // Performing interpolating.
var						const bool	bJustTeleported;   // Used by engine physics - not valid for scripts.
var						bool		bPriorityRagdoll;  // This ragdoll always gets a slot, even if it has to steal one
var						bool		bIsMover;

// Net variables.
enum ENetRole
{
	ROLE_None,              // No role at all.
	ROLE_DumbProxy,			// Dumb proxy of this actor.
	ROLE_SimulatedProxy,	// Locally simulated proxy of this actor.
	ROLE_AutonomousProxy,	// Locally autonomous proxy of this actor.
	ROLE_Authority,			// Authoritative control over the actor.
};
var ENetRole RemoteRole, Role;
var const transient int		NetTag;
var float NetUpdateTime;	// time of last update
var float NetUpdateFrequency; // How many seconds between net updates.
var float NetPriority; // Higher priorities means update it more frequently.
var Pawn                  Instigator;    // Pawn responsible for damage caused by this actor.
var(Sound) sound          AmbientSound;  // Ambient sound effect.
var const name			AttachmentBone;		// name of bone to which actor is attached (if attached to center of base, =='')

var       const LevelInfo Level;         // Level this actor is on.
var transient const Level	XLevel;			// Level object.
var(Advanced)	float		LifeSpan;		// How old the object lives before dying, 0=forever.

//-----------------------------------------------------------------------------
// Structures.

// Identifies a unique convex volume in the world.
struct noexport PointRegion
{
	var zoneinfo Zone;       // Zone.
	var int      iLeaf;      // Bsp leaf.
	var byte     ZoneNumber; // Zone number.
};

//--Live
struct native export XboxAddr
{
	var int ina;
	var int inaOnline;
	var int wPortOnline;
	var byte abEnet[6];
	var byte abOnline[20];
	var String Gamertag;
	var String xuid;
};
//Live--

//-----------------------------------------------------------------------------
// Major actor properties.

// Scriptable.
var const PointRegion		Region;			// Region this actor is in.
var				float       TimerRate;		// Timer event, 0=no timer.
var(DisplayAdvanced) const mesh		Mesh;	// Mesh if DrawType=DT_Mesh.
var transient float			LastRenderTime;	// last time this actor was rendered.
var(Events) name			Tag;			// Actor's tag name.
var name					HiddenTag;		// This tag is manipulated internally by some objects to capture events
var transient array<int>	Leaves;			// BSP leaves this actor is in.
var(Events) name			Event;			// The event this actor causes.
var Inventory				Inventory;		// Inventory chain.
var		const	float       TimerCounter;	// Counts up until it reaches TimerRate.
var transient MeshInstance	MeshInstance;	// Mesh instance.
var(DisplayAdvanced) float	LODBias;
var(Object) name			InitialState;
var(Object) name			Group;

// Offset navigation points
struct NavPtLocator
{
	var() Vector Offset;
	var() float  Yaw;
};

var(Marker) editinline Array<NavPtLocator>	NavPtLocs;
var(Marker) Array<NavigationPoint> NavPts;
var(Marker) class<NavigationPoint> NavPtClass;

// Internal.
var const array<Actor>    Touching;		 // List of touching actors.
var const transient array<int>  OctreeNodes;// Array of nodes of the octree Actor is currently in. Internal use only.
var const transient Box	  OctreeBox;     // Actor bounding box cached when added to Octree. Internal use only.
var const actor           Deleted;       // Next actor in just-deleted chain.
var const float           LatentFloat;   // Internal latent function use.

// Internal tags.
var const native int CollisionTag;
var const transient int JoinedTag;

// The actor's position and rotation.
var const transient PhysicsVolume CurrentPhysicsVolume;// physics volume this actor is currently in
var(Movement) const vector	Location;		// Actor's location; use Move to set.
var(Movement) const rotator Rotation;		// Rotation.
var(Movement) vector		Velocity;		// Velocity.
var			  vector        Acceleration;	// Acceleration.

// Attachment related variables
var(Movement)	name	AttachTag;
var const array<Actor>  Attached;			// array of actors attached to this actor.
var const vector		RelativeLocation;	// location relative to base/bone (valid if base exists)
var const rotator		RelativeRotation;	// rotation relative to base/bone (valid if base exists)

//var(MovementAdvanced) bool bHardAttach;     // Uses 'hard' attachment code. bBlockActor and bBlockPlayer must also be false.
											// This actor cannot then move relative to base (setlocation etc.).
											// Dont set while currently based on something!
											// 
//var const     Matrix    HardRelMatrix;		// Transform of actor in base's ref frame. Doesn't change after SetBase.

var	(Marker)  SquadMarker	SquadMarker;	// If the character is currently involved in a squadmarker or object is part of a marker

//var	(Marker)  vector  HUDMarkerIconOffset; // Offset, in world coordinates, at which the marker icon should be drawn.
var(Marker) float HUDMarkerIconZOffset;

// Projectors
struct noexport ProjectorRenderInfoPtr { var int Ptr; };	// Hack to to fool C++ header generation...
struct noexport StaticMeshProjectorRenderInfoPtr { var int Ptr; };
var const native array<ProjectorRenderInfoPtr> Projectors;// Projected textures on this actor
var const native array<StaticMeshProjectorRenderInfoPtr>	StaticMeshProjectors;

//-----------------------------------------------------------------------------
// Display properties.

var(DisplayAdvanced) pconly Material		Texture;			// Sprite texture.if DrawType=DT_Sprite
var StaticMeshInstance		StaticMeshInstance; // Contains per-instance static mesh data, like static lighting data.
var const export model		Brush;				// Brush if DrawType=DT_Brush.
var(Display) const float	DrawScale;			// Scaling factor, 1.0=normal size.
var(Display) const vector	DrawScale3D;		// Scaling vector, (1.0,1.0,1.0)=normal size.
var(DisplayAdvanced) vector	PrePivot;			// Offset from box center for drawing.
var(Display) array<Material> Skins;				// Multiple skin support - not replicated.
var			Material		RepSkin;			// replicated skin (sets Skins[0] if not none)
var(DisplayAdvanced) ConvexVolume	AntiPortal;			// Convex volume used for DT_AntiPortal
var(DisplayAdvanced) float  CullDistance;       // 0 == no distance cull, < 0 only drawn at distance > 0 cull at distance
var(Lighting) float	ScaleGlow;

var(Display) byte			AmbientGlow;		// Ambient brightness, or 255=pulsing.
// Style for rendering sprites, meshes.
var(DisplayAdvanced) enum ERenderStyle
{
	STY_None,
	STY_Normal,
	STY_Masked,
	STY_Translucent,
	STY_Modulated,
	STY_Alpha,
	STY_Additive,
	STY_Subtractive,
	STY_Particle,
	STY_AlphaZ,
} Style;

// Material Based Hit Effects
var(Advanced)	EMaterialType			TypeOfMaterial; // Override for StaticMesh/Material settings
var(Sound) ESoundOcclusion SoundOcclusion;		// Sound occlusion approach.

var(Advanced)	float		AutoAimOffset;		// Offset to apply in Z when autoaiming

var(Advanced)	static class<DamageVariance>	MyDamageVariance; // Types of things it takes damage from or ignores, made static 12/1/03 BJD

var(Display) float 	IconScale;			// Can be set to scale an icon for the editor
var(Display) Color	EnhancedVisionColor;// Color when player in enhanced vision mode, Set to black to turn off


//-----------------------------------------------------------------------------
// Collision.

// Collision size.
var(Collision) const float CollisionRadius;		// Radius of collision cyllinder.
var(Collision) const float CollisionHeight;		// Half-height cyllinder.



// Physics properties.
var						float			Mass;				// Mass of this actor.
var(MovementAdvanced)	float			Buoyancy;			// Water buoyancy.
var(Movement)			rotator			RotationRate;		// Change in rotation per second.
var(MovementAdvanced)	rotator			DesiredRotation;	// Physics will smoothly rotate actor to this rotation if bRotateToDesired.
var						Actor			PendingTouch;		// Actor touched during move which wants to add an effect after the movement completes 
var const				vector			ColLocation;		// Actor's old location one move ago. Only for debugging

const MAXSTEPHEIGHT = 35.0; // Maximum step height walkable by pawns
const MINFLOORZ = 0.7; // minimum z value for floor normal (if less, not a walkable floor)
					   // 0.7 ~= 45 degree angle for floor
					   
// ifdef WITH_KARMA

// Used to avoid compression
struct KRBVec
{
	var float	X, Y, Z;
};

struct KRigidBodyState
{
	var KRBVec	Position;
	var Quat	Quaternion;
	var KRBVec	LinVel;
	var KRBVec	AngVel;
};
					   
var(Karma) export editinline KarmaParamsCollision KParams; // Parameters for Karma Collision/Dynamics.
var const native int KStepTag;

// REVISIT (NathanM): Make these constants eventually?
var(Karma) float MaxKarmaSpeed;

// endif

//-----------------------------------------------------------------------------
// Animation replication; only replicated if bReplicateAnimations is true
struct AnimRep
{
	var name AnimSequence; 
	var byte Bone;
	var byte Priority;		// Internally a bitfield. Loop flag is bit 7.
	var byte RateScale;		// note that with compression, max replicated ratescale is 4.0
	var byte AnimFrame;
};
const SIMANIMSIZE = 6;					// how many channels will we replicate?
var AnimRep	SimAnim[SIMANIMSIZE];		// Save/Replicate up to SIMANIMSIZE channels.

//-----------------------------------------------------------------------------
// Networking.

// Symmetric network flags, valid during replication only.
var const bool bNetInitial;       // Initial network update.
var const bool bNetOwner;         // Player owns this actor.
var const bool bNetRelevant;      // Actor is currently relevant. Only valid server side, only when replicating variables.
var const bool bDemoRecording;	  // True we are currently demo recording
var const bool bClientDemoRecording;// True we are currently recording a client-side demo
var const bool bRepClientDemo;		// True if remote client is recording demo
var const bool bClientDemoNetFunc;// True if we're client-side demo recording and this call originated from the remote.
var const bool bDemoOwner;			// Demo recording driver owns this actor.
var bool	   bNoRepMesh;			// don't replicate mesh

//Editing flags
var(Display) bool			bHiddenEd;     // Is hidden during editing.
var(Advanced) bool			bHiddenEdGroup;// Is hidden by the group brower.
var(Display) bool			bDirectional;  // Actor shows direction arrow during editing.
var const bool				bSelected;     // Selected in UnrealEd.
var(Advanced) bool			bEdShouldSnap; // Snap to grid in editor.
var transient bool			bEdSnap;       // Should snap to grid in UnrealEd.
var transient const bool	bTempEditor;   // Internal UnrealEd.
var(CollisionAdvanced) bool	bPathColliding;// this actor should collide (if bWorldGeometry && bBlockActors is true) during path building (ignored if bStatic is true, as actor will always collide during path building)
var transient bool			bPathTemp;	 // Internal/path building
var	bool					bScriptInitialized; // set to prevent re-initializing of actors spawned during level startup
var(Advanced) bool			bLockLocation; // Prevent the actor from being moved in the editor.
var(AI) bool				bOfSquadInterest;	//whether this Actor is of interest to the squad
var bool					bAdditionalTarget;	//whether this Actor should be added to AdditionalTargetsList
var bool					bValidEnemy;	//whether this actor can be a valid enemy
var bool				bSquadExamined;		//whether the squad has examined this Actor
var class<LocalMessage> MessageClass;
var float					LastTimeLookedAt; //the last time this actor was looked at by a squadmember
var	EMaterialType			LastTouchedMaterialType;
var Actor					NextSquadInterestActor;
var Actor					NextTargetActor;

//-----------------------------------------------------------------------------
// Enums.

// Travelling from server to server.
enum ETravelType
{
	TRAVEL_Absolute,	// Absolute URL.
	TRAVEL_Partial,		// Partial (carry name, reset server).
	TRAVEL_Relative,	// Relative URL.
};


// double click move direction.
enum EDoubleClickDir
{
	DCLICK_None,
	DCLICK_Left,
	DCLICK_Right,
	DCLICK_Forward,
	DCLICK_Back,
	DCLICK_Active,
	DCLICK_Done
};

enum eKillZType
{
	KILLZ_None,
	KILLZ_Lava,
	KILLZ_Suicide
};

enum ESpace
{
	SPACE_Global,
	SPACE_Mesh,
	SPACE_Clear
};

//-----------------------------------------------------------------------------
// natives.

// Execute a console command in the context of the current level and game engine.
native function string ConsoleCommand( string Command );

// NathanM: Copies the Materials into the skins array
native final function CopyMaterialsToSkins();
native final function CopyMaterialsToPrecacheMaterials();
native final function ClearRenderData();

native final function NavigationPoint GetAnchor();

native event AddToSquadInterest();
native event RemoveFromSquadInterest();
native event AddToTargetList();
native event RemoveFromTargetList();

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	// Location
	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement
					&& (((RemoteRole == ROLE_AutonomousProxy) && bNetInitial)
						|| ((RemoteRole == ROLE_SimulatedProxy) && (bNetInitial || bUpdateSimulatedPosition) && ((Base == None) || Base.bWorldGeometry))
						|| ((RemoteRole == ROLE_DumbProxy) && ((Base == None) || Base.bWorldGeometry))) )
		Location;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& ((DrawType == DT_Mesh) || (DrawType == DT_StaticMesh) || (DrawType==DT_Particle))
					&& (((RemoteRole == ROLE_AutonomousProxy) && bNetInitial)
						|| ((RemoteRole == ROLE_SimulatedProxy) && (bNetInitial || bUpdateSimulatedPosition) && ((Base == None) || Base.bWorldGeometry))
						|| ((RemoteRole == ROLE_DumbProxy) && ((Base == None) || Base.bWorldGeometry))) )
		Rotation, DesiredRotation;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& RemoteRole<=ROLE_SimulatedProxy )
		Base,bOnlyDrawIfAttached;

	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& RemoteRole<=ROLE_SimulatedProxy && (Base != None) && !Base.bWorldGeometry)
		RelativeRotation, RelativeLocation, AttachmentBone;

	// Physics
	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& (((RemoteRole == ROLE_SimulatedProxy) && (bNetInitial || bUpdateSimulatedPosition))
						|| ((RemoteRole == ROLE_DumbProxy) && (Physics == PHYS_Falling))) )
		Velocity, Acceleration;

	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& (((RemoteRole == ROLE_SimulatedProxy) && bNetInitial)
						|| (RemoteRole == ROLE_DumbProxy)) )
		Physics;

	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && bReplicateMovement 
					&& (RemoteRole <= ROLE_SimulatedProxy) && (Physics == PHYS_Rotating) )
		bFixedRotationDir, bRotateToDesired, RotationRate;

	// sounds.
	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) && (!bNetOwner || !bClientAnim) )
		AmbientSound;

	//unreliable if( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) && (!bNetOwner || !bClientAnim) 
	//				&& (AmbientSound!=None) )
	//	SoundRadius;

	// Animation. 
	unreliable if( (!bSkipActorPropertyReplication || bNetInitial) 
				&& (Role==ROLE_Authority) && (DrawType==DT_Mesh) && bReplicateAnimations )
		SimAnim;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) )
		bHidden, bValidEnemy;//, bHardAttach;

	// Properties changed using accessor functions (Owner, rendering, and collision)
	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) && bNetDirty )
		Owner, DrawScale, DrawType, bCollideActors,bCollideWorld,bOnlyOwnerSee,Texture,Style, RepSkin;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) && bNetDirty 
					&& (bCollideActors || bCollideWorld) )
		bProjTarget, bBlockActors, bBlockPlayers, CollisionRadius, CollisionHeight;

	// Properties changed only when spawning or in script (relationships, rendering, lighting)
	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) )
		Role,RemoteRole,bNetOwner,LightType,bTearOff;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
					&& bNetDirty && bNetOwner )
		Inventory;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
					&& bNetDirty && bReplicateInstigator )
		Instigator;

	// Infrequently changed mesh properties
	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
					&& bNetDirty && (DrawType == DT_Mesh) )
		AmbientGlow,bUnlit,PrePivot;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
					&& bNetDirty && !bNoRepMesh && (DrawType == DT_Mesh) )
		Mesh;

	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
				&& bNetDirty && (DrawType == DT_StaticMesh) )
		StaticMesh;

	// Infrequently changed lighting properties.
	unreliable if ( (!bSkipActorPropertyReplication || bNetInitial) && (Role==ROLE_Authority) 
					&& bNetDirty && (LightType != LT_None) )
		LightEffect, LightBrightness, LightHue, LightSaturation,
		LightRadius, LightPeriod, LightPhase, bSpecialLit;
}

//=============================================================================
// Actor error handling.

// Handle an error and kill this one actor.
native(233) final function Error( coerce string S );

//=============================================================================
// General functions.

// Latent functions.
native(256) final latent function Sleep( float Seconds );

// Collision.
native(262) final function SetCollision( optional bool NewColActors, optional bool NewBlockActors, optional bool NewBlockPlayers );
native(283) final function bool SetCollisionSize( float NewRadius, float NewHeight );
native event ResetCollisionSize();
native final function SetDrawScale(float NewScale);
native final function SetDrawScale3D(vector NewScale3D);
native final function SetStaticMesh(StaticMesh NewStaticMesh);
native final function SetDrawType(EDrawType NewDrawType);

// Movement.
native final event bool Move( vector Delta );
native final function bool SetLocation( vector NewLocation, optional bool bNoCheck );
native final event bool SetRotation( rotator NewRotation );

// SetRelativeRotation() sets the rotation relative to the actor's base
native final function bool SetRelativeRotation( rotator NewRotation );
native final function bool SetRelativeLocation( vector NewLocation );

native(3969) final function bool MoveSmooth( vector Delta );
native(3971) final function AutonomousPhysics(float DeltaSeconds);

// Relations.
native(298) final function SetBase( actor NewBase, optional vector NewFloor );
native(272) final function SetOwner( actor NewOwner );

//=============================================================================
// Animation.

native final function string GetMeshName();

// Animation functions.
native(259) final function PlayAnim( name Sequence, optional name Bone, optional float Rate, optional float StartFrame, optional int Channel );
native(260) final function LoopAnim( name Sequence, optional name Bone, optional float Rate, optional float StartFrame, optional int Channel );
native(282) final function bool IsAnimating(optional int Channel);
native(261) final latent function FinishAnim(optional int Channel);
native(263) final function bool HasAnim( name Sequence );
native final function StopAnimating( optional bool ClearAllButBase );
native final function bool IsPlayingAnim( name Sequence, optional name Bone );

simulated event AnimLoopEnd( float LoopStart );	// called by the AnimLoopEnd notify

// Animation notifications.
event AnimEnd( int Channel );

// Skeletal animation.
simulated native final function LinkMesh( mesh NewMesh, optional bool bKeepAnim );
simulated native final function LinkAnimToMesh( MeshAnimation Anim, mesh TargetMesh );
native final function BoneRefresh();

native final function vector  GetBoneLocation( name BoneName, optional ESpace Space );
native final function rotator GetBoneRotation( name BoneName, optional ESpace Space );

native final function SetBoneLocation( name BoneName, vector BoneTrans, optional ESpace Space );
native final function SetBoneRotation( name BoneName, rotator BoneTurn, optional ESpace Space );
native final function SetBoneScale( name BoneName, float BoneScale );
native final function GetAnimParams( name BoneName, out name OutSeqName, out float OutAnimFrame );
native final function bool AnimIsInGroup( name BoneName, name GroupName );  

native final function bool AttachToBone( actor Attachment, name BoneName );
native final function bool DetachFromBone( actor Attachment );

native final function name GetSocketBoneName( name SocketName );
native final function name FindNearestBone( vector Location );

function bool IsValidEnemy()
{
	return bValidEnemy;
}

event Actor SpawnAttachment(Name BoneName, class<Actor> ActorClass, optional StaticMesh Mesh)
{
	local Actor NewActor;	

	NewActor = Spawn(ActorClass, self);
	if (Mesh != None)
		NewActor.SetStaticMesh(Mesh);

	if (NewActor != None)
	{
		AttachToBone(NewActor, BoneName);
		NewActor.SetRelativeLocation(vect(0,0,0)); 
		NewActor.SetRelativeRotation(rot(0,0,0));		
	}

	return NewActor;
}

//gets rid of whatever attachment is on the Pawn
event DestroyAttachment(Name BoneName)
{
	local int i;
	for (i=0; i < Attached.Length; ++i)
	{
		if ( Attached[i] != None && Attached[i].AttachmentBone == BoneName )
		{				
			Attached[i].Destroy();
			return;
		}
	}
}

function SetAttachmentCollision(bool bEnable)
{
	local int i;
	for (i=0; i < Attached.Length; ++i)
	{
		if ( Attached[i] != None )
		{				
			Attached[i].bBlockZeroExtentTraces = bEnable;
			return;
		}
	}
}

simulated function Emitter SpawnSkeletalEffect(class<Emitter> EffectClass)
{
	local Emitter Effect;

	if( EffectClass != None && Level.NetMode != NM_DedicatedServer && DrawType == DT_Mesh )
	{
		Effect = Spawn( EffectClass, self );
		Effect.SkeletalMeshActor = self;
		Effect.SetBase(self);
		return Effect;
	}
	return None;
}

//=========================================================================
// Rendering.

native final function plane GetRenderBoundingSphere();
native final function DrawDebugLine( vector LineStart, vector LineEnd, byte R, byte G, byte B); // SLOW! Use for debugging only!
native final function vector GetCollisionCenter();

//=========================================================================
// Physics.

native final function DebugClock();
native final function DebugUnclock();

// Physics control.
native(301) final latent function FinishInterpolation();
native(3970) final function SetPhysics( EPhysics newPhysics );

native final function OnlyAffectPawns(bool B);

// ifdef WITH_KARMA
native final function quat KGetRBQuaternion();

native final function KGetRigidBodyState(out KRigidBodyState RBstate);
native final function KDrawRigidBodyState(KRigidBodyState RBState, bool AltColour); // SLOW! Use for debugging only!
native final function vector KRBVecToVector(KRBVec RBvec);
native final function KRBVec KRBVecFromVector(vector v);

native final function KSetMass( float mass );
native final function float KGetMass();

// Set inertia tensor assuming a mass of 1. Scaled by mass internally to calculate actual inertia tensor.
native final function KSetInertiaTensor( vector it1, vector it2 );
native final function KGetInertiaTensor( out vector it1, out vector it2 );

native final function KSetDampingProps( float lindamp, float angdamp );
native final function KGetDampingProps( out float lindamp, out float angdamp );

native final function KSetFriction( float friction );
native final function float KGetFriction();

native final function KSetRestitution( float rest );
native final function float KGetRestitution();

native final function KSetCOMOffset( vector offset );
native final function KGetCOMOffset( out vector offset );
native final function KGetCOMPosition( out vector pos ); // get actual position of actors COM in world space

native final function KSetImpactThreshold( float thresh );
native final function float KGetImpactThreshold();

native final function KWake();
native final function bool KIsAwake();
native final function KAddImpulse( vector Impulse, vector Position, optional name BoneName );

native final function KSetStayUpright( bool stayUpright, bool allowRotate );
native final function KSetStayUprightParams( float stiffness, float damping );

native final function KSetBlockKarma( bool newBlock );

native final function KSetActorGravScale( float ActorGravScale );
native final function float KGetActorGravScale();

// Disable/Enable Karma contact generation between this actor, and another actor.
// Collision is on by default.
native final function KDisableCollision( actor Other );
native final function KEnableCollision( actor Other );

// Ragdoll-specific functions
native final function KSetSkelVel( vector LinVelocity, optional vector AngVelocity, optional bool AddToCurrent );
native final function float KGetSkelMass();
native final function KFreezeRagdoll();

// You MUST turn collision off (KSetBlockKarma) before using bone lifters!
native final function KAddBoneLifter( name BoneName, InterpCurve LiftVel, float LateralFriction, InterpCurve Softness ); 
native final function KRemoveLifterFromBone( name BoneName ); 
native final function KRemoveAllBoneLifters(); 

// Used for only allowing a fixed maximum number of ragdolls in action.
native final function KMakeRagdollAvailable();
native final function bool KIsRagdollAvailable();

native final function KBreakRagdollJoint( name BoneName, optional bool DestroyChain );

// Determine how much damage to take
native simulated function float CalculateDamageFrom(class<DamageType> DamType, float DamGiven, optional class<DamageVariance> DamVariance);

// event called when Karmic actor hits with impact velocity over KImpactThreshold
event KImpact(actor other, vector pos, vector impactVel, vector impactNorm, EMaterialType MaterialHit); 

// event called when karma actor's velocity drops below KVelDropBelowThreshold;
event KVelDropBelow();

// event called when karma actor comes to rest
event KAtRest();

// event called when a ragdoll convulses (see KarmaParamsSkel)
event KSkelConvulse();

// event called just before sim to allow user to 
// NOTE: you should ONLY put numbers into Force and Torque during this event!!!!
event KApplyForce(out vector Force, out vector Torque);

// This is called from inside C++ physKarma at the appropriate time to update state of Karma rigid body.
// If you return true, newState will be set into the rigid body. Return false and it will do nothing.
event bool KUpdateState(out KRigidBodyState newState);

// endif

//=========================================================================
// Music

native final function PlayMusic(Sound Song, optional bool OverrideFadeDefaults, optional float FadeInTime, optional
									EMusicFadeType FadeInType, optional float FadeOutTime, optional EMusicFadeType FadeOutType);
native final function SetImuseState(Sound State, optional bool OverrideFadeDefaults, optional float FadeInTime, optional
									EMusicFadeType FadeInType, optional float FadeOutTime, optional EMusicFadeType FadeOutType);
native final function StopAllMusic( float FadeOutTime );
native final function StopSound( Actor Owner, Sound StopSound, float FadeOutTime );
native final function EnableBattleEngine( bool onOff );
native final function SetBattleEngineParams(EBattleEngineBattleCriteria criteria,
	int baddieCount, float radiusInner, float radiusOuter);
native final function SetBattleMusicMulti( Sound multiSound );
native final function ForceBattleMusic();
native final function ForceBattleFlourish();

//=========================================================================
// Engine notification functions.

//
// Major notifications.
//
event Destroyed()
{
	if (bOfSquadInterest)
	{
		RemoveFromSquadInterest();
	}
	if (bAdditionalTarget)
	{
		RemoveFromTargetList();
	}
}

event GainedChild( Actor Other );
event LostChild( Actor Other );
event Tick( float DeltaTime );
event PostNetworkReceive();

//
// Triggers.
//
event Trigger( Actor Other, Pawn EventInstigator );
event UnTrigger( Actor Other, Pawn EventInstigator );
event BeginEvent();
event EndEvent();

simulated function TacticalModeBegin( PlayerController Player );
simulated function TacticalModeEnd( PlayerController Player );

simulated function NotifyTargeted( PlayerController Player )
{
	if( SquadMarker != None )
		SquadMarker.NotifyTargeted( Player );
}
simulated function NotifyUnTargeted( PlayerController Player )
{
	if( SquadMarker != None )
		SquadMarker.NotifyUnTargeted( Player );
}

//
// Physics & world interaction.
//
event Timer();
event HitWall( vector HitNormal, actor HitWall, EMaterialType KindOfMaterial );
event Falling();
event Landed( vector HitNormal );
event ZoneChange( ZoneInfo NewZone );
event PhysicsVolumeChange( PhysicsVolume NewVolume );
event Touch( Actor Other );
event PostTouch( Actor Other ); // called for PendingTouch actor after physics completes
event UnTouch( Actor Other );
event Bump( Actor Other );
event BaseChange();
event Attach( Actor Other );
event Detach( Actor Other );
event bool EncroachingOn( actor Other );
event EncroachedBy( actor Other );
event FinishedInterpolation()
{
	bInterpolating = false;
}

event EndedRotation();			// called when rotation completes
event UsedBy( Pawn user ); // called if this Actor was touching a Pawn who pressed Use

simulated event FellOutOfWorld(eKillZType KillType)
{
	SetPhysics(PHYS_None);
	Destroy();
}	

//
// Damage and kills.
//
event KilledBy( pawn EventInstigator, optional class<DamageType> damageType );
event float TakeDamage( float Damage, Pawn EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional Name BoneName );
function bool ReflectLaser( Actor Projectile, vector Location, vector Direction, optional name HitBone );

//
// Trace a line and see what it collides with first.
// Takes this actor's collision properties into account.
// Returns first hit actor, Level if hit level, or None if hit nothing.
//
native(277) final function Actor Trace
(
	out vector      HitLocation,
	out vector      HitNormal,
	vector          TraceEnd,
	optional vector TraceStart,
	optional bool   bTraceActors,
	optional vector Extent,
	optional out material Material,
	optional out Name BoneName,
	optional out EMaterialType MaterialType
);

native final function Actor TraceColliding
(
	out vector      HitLocation,
	out vector      HitNormal,
	vector          TraceEnd,
	optional vector TraceStart,
	optional vector Extent,
	optional out material Material,
	optional out Name BoneName
);

// returns true if did not hit world geometry
native(548) final function bool FastTrace
(
	vector          TraceEnd,
	optional vector TraceStart
);

//
// Spawn an actor. Returns an actor of the specified class, not
// of class Actor (this is hardcoded in the compiler). Returns None
// if the actor could not be spawned (either the actor wouldn't fit in
// the specified location, or the actor list is full).
// Defaults to spawning at the spawner's location.
//
native(278) final function actor Spawn
(
	class<actor>      SpawnClass,
	optional actor	  SpawnOwner,
	optional name     SpawnTag,
	optional vector   SpawnLocation,
	optional rotator  SpawnRotation
);

//
// Destroy this actor. Returns true if destroyed, false if indestructable.
// Destruction is latent. It occurs at the end of the tick.
//
native(279) final function bool Destroy();

// Networking - called on client when actor is torn off (bTearOff==true)
event TornOff();

//=============================================================================
// Timing.

// Causes Timer() events every NewTimerRate seconds.
native(280) final function SetTimer( float NewTimerRate, bool bLoop );

//=============================================================================
// Sound functions.

/* Play a sound effect.
*/
native(264) final function PlaySound
(
	sound				Sound,
	optional ESoundSlot Slot,
	optional float		Volume,
	optional bool		bNoOverride,
	optional float		Radius,
	optional float		Pitch,
	optional bool		Attenuate
);
 
native final function PauseSound(sound Sound, bool bPause );

native final function bool IsSoundPaused(sound Sound, bool bPause );
native final function bool IsSoundActive(sound Sound);

/* play a sound effect, but don't propagate to a remote owner
 (he is playing the sound clientside)
 */
native simulated final function PlayOwnedSound
(
	sound				Sound,
	optional ESoundSlot Slot,
	optional float		Volume,
	optional bool		bNoOverride,
	optional bool		Attenuate
);

/* Get a sound duration.
*/
native final function float GetSoundDuration( sound Sound );

//=============================================================================
// Force Feedback.
// jdf ---
native(566) final function PlayFeedbackEffect( String EffectName );
native(567) final function StopFeedbackEffect( optional String EffectName ); // Pass no parameter or "" to stop all
native(568) final function bool ForceFeedbackSupported( optional bool Enable );
// --- jdf

//=============================================================================
// AI functions.

/* Inform other creatures that you've made a noise
 they might hear (they are sent a HearNoise message)
 Senders of MakeNoise should have an instigator if they are not pawns.
*/
native final function PostStimulusToWorld(EStimulusType type);
native final function PostStimulusAtLocation(EStimulusType type, vector Location);
native final function PostStimulusToIndividual(EStimulusType type, Actor Actor);

native final event vector CalcTrajectoryVelocity( vector InitialPosition, vector TargetPosition, float HorizontalVelocity, optional bool AllowFlatArcs );

/* PlayerCanSeeMe returns true if any player (server) or the local player (standalone
or client) has a line of sight to actor's location.
*/
native(532) final function bool PlayerCanSeeMe(optional PlayerController PC);

native final function vector SuggestFallVelocity(vector Destination, vector Start, float MaxZ, float MaxXYSpeed);
 
//=============================================================================
// Regular engine functions.

// Teleportation.
event bool PreTeleport( Teleporter InTeleporter );
event PostTeleport( Teleporter OutTeleporter );

// Level state.
event BeginPlay();

//========================================================================
// Disk access.

// Find files.
native(539) final function string GetMapName( string NameEnding, string MapName, int Dir );
native(545) final function GetNextSkin( string Prefix, string CurrentSkin, int Dir, out string SkinName, out string SkinDesc );
native(547) final function string GetURLMap();
native final function string ExpandRelativeURL(string URL); //--Live
native final function string RemoveURLOption(string URL, string Option);
native final function string GetNextInt( string ClassName, int Num );
native final function GetNextIntDesc( string ClassName, int Num, out string Entry, out string Description );
native final function bool GetCacheEntry( int Num, out string GUID, out string Filename );
native final function bool MoveCacheEntry( string GUID, optional string NewFilename );  

//=============================================================================
// Iterator functions.

// Iterator functions for dealing with sets of actors.

/* AllActors() - avoid using AllActors() too often as it iterates through the whole actor list and is therefore slow
*/
native(304) final iterator function AllActors     ( class<actor> BaseClass, out actor Actor, optional name MatchTag );

/* DynamicActors() only iterates through the non-static actors on the list (still relatively slow, bu
 much better than AllActors).  This should be used in most cases and replaces AllActors in most of 
 Epic's game code. 
*/
native(313) final iterator function DynamicActors     ( class<actor> BaseClass, out actor Actor, optional name MatchTag );

/* ChildActors() returns all actors owned by this actor.  Slow like AllActors()
*/
native(305) final iterator function ChildActors   ( class<actor> BaseClass, out actor Actor );

/* BasedActors() returns all actors based on the current actor (slow, like AllActors)
*/
native(306) final iterator function BasedActors   ( class<actor> BaseClass, out actor Actor );

/* TouchingActors() returns all actors touching the current actor (fast)
*/
native(307) final iterator function TouchingActors( class<actor> BaseClass, out actor Actor );

/* TraceActors() return all actors along a traced line.  Reasonably fast (like any trace)
*/
native(309) final iterator function TraceActors   ( class<actor> BaseClass, out actor Actor, out vector HitLoc, out vector HitNorm, vector End, optional vector Start, optional vector Extent, optional out name BoneName, optional out EMaterialType MaterialType );

/* RadiusActors() returns all actors within a give radius.  Slow like AllActors().  Use CollidingActors() or VisibleCollidingActors() instead if desired actor types are visible
(not bHidden) and in the collision hash (bCollideActors is true)
*/
native(310) final iterator function RadiusActors  ( class<actor> BaseClass, out actor Actor, float Radius, optional vector Loc );

/* VisibleActors() returns all visible actors within a radius.  Slow like AllActors().  Use VisibleCollidingActors() instead if desired actor types are 
in the collision hash (bCollideActors is true)
*/
native(311) final iterator function VisibleActors ( class<actor> BaseClass, out actor Actor, optional float Radius, optional vector Loc );

/* VisibleCollidingActors() returns visible (not bHidden) colliding (bCollideActors==true) actors within a certain radius.
Much faster than AllActors() since it uses the collision hash
*/
native(312) final iterator function VisibleCollidingActors ( class<actor> BaseClass, out actor Actor, float Radius, optional vector Loc, optional bool bIgnoreHidden );

/* CollidingActors() returns colliding (bCollideActors==true) actors within a certain radius.
Much faster than AllActors() for reasonably small radii since it uses the collision hash
*/
native(321) final iterator function CollidingActors ( class<actor> BaseClass, out actor Actor, float Radius, optional vector Loc );

//=============================================================================
// Color functions
native(549) static final operator(20) color -     ( color A, color B );
native(550) static final operator(16) color *     ( float A, color B );
native(551) static final operator(20) color +     ( color A, color B );
native(552) static final operator(16) color *     ( color A, float B );

//=============================================================================
// Scripted Actor functions.

/* RenderOverlays()
called by player's hud to request drawing of actor specific overlays onto canvas
*/
function RenderOverlays(Canvas Canvas);

// RenderTexture
event RenderTexture(ScriptedTexture Tex);

//
// Called immediately before gameplay begins.
//
event PreBeginPlay()
{
	// Handle autodestruction if desired.
	if( !bGameRelevant && (Level.NetMode != NM_Client) && !Level.Game.BaseMutator.CheckRelevance(Self) )
	{
		Destroy();
		return;
	}
	if (bOfSquadInterest)
	{
		AddToSquadInterest();
	}
	if (bAdditionalTarget)
		AddToTargetList();
}

//
// Broadcast a localized message to all players.
// Most message deal with 0 to 2 related PRIs.
// The LocalMessage class defines how the PRI's and optional actor are used.
//
event BroadcastLocalizedMessage( class<LocalMessage> MessageClass, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	Level.Game.BroadcastLocalized( self, MessageClass, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );
}

// Called immediately after gameplay begins.
//
event PostBeginPlay();

// Called after PostBeginPlay.
//
simulated event SetInitialState()
{
	bScriptInitialized = true;
	if( InitialState!='' )
		GotoState( InitialState );
	else
		GotoState( 'Auto' );
}

// Called after either loading & starting a map, or loading a savegame map.
// Any state that is not saved, but needs to be set up before play, is set up here.
event PostLoadBeginPlay();

// called after PostBeginPlay.  On a net client, PostNetBeginPlay() is spawned after replicated variables have been initialized to
// their replicated values
event PostNetBeginPlay();

simulated function UpdatePrecacheMaterials()
{
	local int i;
	
	CopyMaterialsToPrecacheMaterials();

	if ( Skins.Length > 0 )
		for ( i=0; i<Skins.Length; i++ )
			if ( Skins[i] != None )
				Level.AddPrecacheMaterial( Skins[i] );
}

simulated function UpdatePrecacheStaticMeshes()
{
	if ( (DrawType == DT_StaticMesh) && !bStatic && !bNoDelete )
		Level.AddPrecacheStaticMesh(StaticMesh);
}

/* HurtRadius()
 Hurt locally authoritative actors within the radius.
*/
simulated final event HurtRadius( float DamageAmount, float DamageRadius, class<DamageType> DamageType, float Momentum, vector HitLocation, optional Actor ExcludedActor )
{
	local actor Victims;
	local float damageScale, dist, InvDamageRadius;
	local vector dir;
	local vector DamageImpulse;

	if( bHurtEntry )
		return;

	InvDamageRadius = 1 / DamageRadius;

	bHurtEntry = true;
	foreach VisibleCollidingActors( class 'Actor', Victims, DamageRadius, HitLocation )
	{
		// don't let blast damage affect fluid - VisibleCollisingActors doesn't really work for them - jag
		if( (Victims != self) && (Victims != ExcludedActor) && (Victims.Role == ROLE_Authority) && (!Victims.IsA('FluidSurfaceInfo')) )
		{
			dir = Victims.Location - HitLocation;
			dist = VSize(dir);
			dir = dir/dist;			
			
			if( dist < Victims.CollisionRadius )
				damageScale = 1;
			else
				damageScale = 1 - ( ( dist - Victims.CollisionRadius ) * InvDamageRadius );

			DamageImpulse = Momentum * dir;
			DamageImpulse.Z += damageType.default.KDeathUpKick;
			DamageImpulse *= damageScale;

			Victims.TakeDamage
			(
				damageScale * DamageAmount,
				Instigator, 
				Victims.Location,
				DamageImpulse,
				DamageType
			);
		} 
	}
	bHurtEntry = false;
}

// Called when carried onto a new level, before AcceptInventory.
//
event TravelPreAccept();

// Called when carried into a new level, after AcceptInventory.
//
event TravelPostAccept();

// Called by PlayerController when this actor becomes its ViewTarget.
//
function BecomeViewTarget();

// Returns the string representation of the name of an object without the package
// prefixes.
//
event String GetItemName( string FullName )
{
	local int pos;

	pos = InStr(FullName, ".");
	While ( pos != -1 )
	{
		FullName = Right(FullName, Len(FullName) - pos - 1);
		pos = InStr(FullName, ".");
	}

	return FullName;
}

// Returns the human readable string representation of an object.
//
simulated function String GetHumanReadableName()
{
	return GetItemName(string(class));
}

final function ReplaceText(out string Text, string Replace, string With)
{
	local int i;
	local string Input;
		
	Input = Text;
	Text = "";
	i = InStr(Input, Replace);
	while(i != -1)
	{	
		Text = Text $ Left(Input, i) $ With;
		Input = Mid(Input, i + Len(Replace));	
		i = InStr(Input, Replace);
	}
	Text = Text $ Input;
}

// Set the display properties of an actor.  By setting them through this function, it allows
// the actor to modify other components (such as a Pawn's weapon) or to adjust the result
// based on other factors (such as a Pawn's other inventory wanting to affect the result)
function SetDisplayProperties(ERenderStyle NewStyle, Material NewTexture, bool bLighting )
{
	Style = NewStyle;
	texture = NewTexture;
	bUnlit = bLighting;
}

function SetDefaultDisplayProperties()
{
	Style = Default.Style;
	texture = Default.Texture;
	bUnlit = Default.bUnlit;
}

// Get localized message string associated with this actor
static function string GetLocalString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2
	)
{
	return "";
}

function MatchStarting(); // called when gameplay actually starts
function SetGRI(GameReplicationInfo GRI);

function String GetDebugName()
{
	return GetItemName(string(self));
}

/* DisplayDebug()
list important actor variable on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
the ShowDebug exec is used
*/
simulated function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	local string T;
	local float XL;
	local int i;
	local Actor A;
	local name anim;
	local float frame;

	Canvas.Style = ERenderStyle.STY_Normal;
	Canvas.StrLen("TEST", XL, YL);
	YPos = YPos + YL;
	Canvas.SetPos(4,YPos);
	Canvas.SetDrawColor(255,0,0);
	T = GetDebugName();
	if ( bDeleteMe )
		T = T$" DELETED (bDeleteMe == true)";

	Canvas.DrawText(T, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.SetDrawColor(255,255,255);

	if ( Level.NetMode != NM_Standalone )
	{
		// networking attributes
		T = "ROLE ";
		Switch(Role)
		{
			case ROLE_None: T=T$"None"; break;
			case ROLE_DumbProxy: T=T$"DumbProxy"; break;
			case ROLE_SimulatedProxy: T=T$"SimulatedProxy"; break;
			case ROLE_AutonomousProxy: T=T$"AutonomousProxy"; break;
			case ROLE_Authority: T=T$"Authority"; break;
		}
		T = T$" REMOTE ROLE ";
		Switch(RemoteRole)
		{
			case ROLE_None: T=T$"None"; break;
			case ROLE_DumbProxy: T=T$"DumbProxy"; break;
			case ROLE_SimulatedProxy: T=T$"SimulatedProxy"; break;
			case ROLE_AutonomousProxy: T=T$"AutonomousProxy"; break;
			case ROLE_Authority: T=T$"Authority"; break;
		}
		if ( bTearOff )
			T = T$" Tear Off";
		Canvas.DrawText(T, false);
		YPos += YL;
		Canvas.SetPos(4,YPos);
	}
	T = "Physics ";
	Switch(PHYSICS)
	{
		case PHYS_None: T=T$"None"; break;
		case PHYS_Walking: T=T$"Walking"; break;
		case PHYS_Falling: T=T$"Falling"; break;
		case PHYS_Swimming: T=T$"Swimming"; break;
		case PHYS_Flying: T=T$"Flying"; break;
		case PHYS_Rotating: T=T$"Rotating"; break;
		case PHYS_Projectile: T=T$"Projectile"; break;
		case PHYS_Interpolating: T=T$"Interpolating"; break;
		case PHYS_MovingBrush: T=T$"MovingBrush"; break;
		case PHYS_Spider: T=T$"Spider"; break;
		case PHYS_Trailer: T=T$"Trailer"; break;
		case PHYS_Ladder: T=T$"Ladder"; break;
	}
	T = T$" in physicsvolume "$GetItemName(string(CurrentPhysicsVolume))$" on base "$GetItemName(string(Base));
	if ( bBounce )
		T = T$" - will bounce";
	Canvas.DrawText(T, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	Canvas.DrawText("Location: "$Location$" Rotation "$Rotation, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("Velocity: "$Velocity$" Speed "$VSize(Velocity), false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("Acceleration: "$Acceleration, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	
	Canvas.DrawColor.B = 0;
	Canvas.DrawText("Collision Radius "$CollisionRadius$" Height "$CollisionHeight);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	Canvas.DrawText("Collides with Actors "$bCollideActors$", world "$bCollideWorld$", proj. target "$bProjTarget);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	Canvas.DrawText("Blocks Actors "$bBlockActors$", players "$bBlockPlayers);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	T = "Touching ";
	ForEach TouchingActors(class'Actor', A)
		T = T$GetItemName(string(A))$" ";
	if ( T == "Touching ")
		T = "Touching nothing";
	Canvas.DrawText(T, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	Canvas.DrawColor.R = 0;
	T = "Rendered: ";
	Switch(Style)
	{
		case STY_None: T=T; break;
		case STY_Normal: T=T$"Normal"; break;
		case STY_Masked: T=T$"Masked"; break;
		case STY_Translucent: T=T$"Translucent"; break;
		case STY_Modulated: T=T$"Modulated"; break;
		case STY_Alpha: T=T$"Alpha"; break;
	}		

	Switch(DrawType)
	{
		case DT_None: T=T$" None"; break;
		case DT_Sprite: T=T$" Sprite "; break;
		case DT_Mesh: T=T$" Mesh "; break;
		case DT_Brush: T=T$" Brush "; break;
		case DT_RopeSprite: T=T$" RopeSprite "; break;
		case DT_VerticalSprite: T=T$" VerticalSprite "; break;
		case DT_Terraform: T=T$" Terraform "; break;
		case DT_SpriteAnimOnce: T=T$" SpriteAnimOnce "; break;
		case DT_StaticMesh: T=T$" StaticMesh "; break;
	}

	if ( DrawType == DT_Mesh )
	{
		T = T$GetItemName(string(Mesh));
		if ( Skins.length > 0 )
		{
			T = T$" skins: ";
			for ( i=0; i<Skins.length; i++ )
			{
				if ( skins[i] == None )
					break;
				else
					T =T$GetItemName(string(skins[i]))$", ";
			}
		}

		Canvas.DrawText(T, false);
		YPos += YL;
		Canvas.SetPos(4,YPos);
		
		// mesh animation
		GetAnimParams('',Anim,frame);
		T = "AnimSequence "$Anim$" Frame "$frame;
	}
	else if ( (DrawType == DT_Sprite) || (DrawType == DT_SpriteAnimOnce) )
		T = T$Texture;
	else if ( DrawType == DT_Brush )
		T = T$Brush;
		
	Canvas.DrawText(T, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
	
	Canvas.DrawColor.B = 255;	
	Canvas.DrawText("Tag: "$Tag$" Event: "$Event$" STATE: "$GetStateName(), false);
	YPos += YL;
	Canvas.SetPos(4,YPos);

	Canvas.DrawText("Instigator "$GetItemName(string(Instigator))$" Owner "$GetItemName(string(Owner)));
	YPos += YL;
	Canvas.SetPos(4,YPos);

	Canvas.DrawText("Timer: "$TimerCounter$" LifeSpan "$LifeSpan$" AmbientSound "$AmbientSound);
	YPos += YL;
	Canvas.SetPos(4,YPos);
}

// NearSpot() returns true is spot is within collision cylinder
simulated final function bool NearSpot(vector Spot)
{
	local vector Dir;

	Dir = Location - Spot;
	
	if ( abs(Dir.Z) > CollisionHeight )
		return false;

	Dir.Z = 0;
	return ( VSize(Dir) <= CollisionRadius );
}

simulated final function bool TouchingActor(Actor A)
{
	local vector Dir;

	Dir = Location - A.Location;
	
	if ( abs(Dir.Z) > CollisionHeight + A.CollisionHeight )
		return false;

	Dir.Z = 0;
	return ( VSize(Dir) <= CollisionRadius + A.CollisionRadius );
}

/* StartInterpolation()
when this function is called, the actor will start moving along an interpolation path
beginning at Dest
*/	
simulated function StartInterpolation()
{
	GotoState('');
	SetCollision(True,false,false);
	bCollideWorld = False;
	bInterpolating = true;
	SetPhysics(PHYS_None);
}

/* Reset() 
reset actor to initial state - used when restarting level without reloading.
*/
function Reset();

/* 
Trigger an event
*/
event TriggerEvent( Name EventName, Actor Other, Pawn EventInstigator )
{
	local Actor A;

	if ( EventName == '' )
		return;

	if (Level.Game.bLogEvents)
		Log("TriggerEvent: "$EventName$" Actor: "$Other$" Instigator: "$EventInstigator);

	ForEach DynamicActors( class 'Actor', A, EventName )
	{
		A.Trigger(Other, EventInstigator);
	}
}

/*
Untrigger an event
*/
function UntriggerEvent( Name EventName, Actor Other, Pawn EventInstigator )
{
	local Actor A;

	if ( EventName == '' )
		return;

	ForEach DynamicActors( class 'Actor', A, EventName )
		A.Untrigger(Other, EventInstigator);
}

event bool IsInVolume(Volume aVolume)
{
	local Volume V;
	
	ForEach TouchingActors(class'Volume',V)
		if ( V == aVolume )
			return true;
	return false;
}
	 
function bool IsInPain()
{
	local PhysicsVolume V;

	ForEach TouchingActors(class'PhysicsVolume',V)
		if ( V.bPainCausing && (V.DamagePerSec > 0) )
			return true;
	return false;
}

function PlayTeleportEffect(bool bOut, bool bSound);

function bool CanSplash()
{
	return false;
}

function vector GetCollisionExtent()
{
	local vector Extent;

	Extent = CollisionRadius * vect(1,1,0);
	Extent.Z = CollisionHeight;
	return Extent;
}

simulated function bool EffectIsRelevant(vector SpawnLocation, bool bForceDedicated )
{
	local PlayerController P;
	local bool bResult;
	
	if ( Level.NetMode == NM_DedicatedServer )
		bResult = bForceDedicated;
	else if ( Level.NetMode == NM_Client )
		bResult = true;
	else if ( (Instigator != None) && Instigator.IsHumanControlled() )
		bResult =  true;
	else if ( SpawnLocation == Location )
		bResult = ( Level.TimeSeconds - LastRenderTime < 3 );
	else if ( (Instigator != None) && (Level.TimeSeconds - Instigator.LastRenderTime < 3) )
		bResult = true;
	else
	{	
		P = Level.GetLocalPlayerController();
		if ( P == None )
			bResult = false;
		else 
			bResult = ( (Vector(P.Rotation) Dot (SpawnLocation - P.ViewTarget.Location)) > 0.0 );
	}
	return bResult;
}

function bool CanBeAutoAimedBy(Actor A)
{
	return bCanBeAutoAimed;
}

// Ask whether an event belongs to this actor; used in the editor
event bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return false;
}

// Ask whether a tag belongs to this actor; used in the editor
event bool ContainsPartialTag(string StartOfTagName)
{
	local string TagString;
	TagString = string(Tag);
	if ( Caps(StartOfTagName) == Left(Caps(TagString), Len(StartOfTagName) ) )
	{
		return true;
	}

	TagString = string(ForcedVisibilityZoneTag);
	if ( Caps(StartOfTagName) == Left(Caps(TagString), Len(StartOfTagName) ) )
	{
		return true;
	}
	
	TagString = string(AttachTag);
	if ( Caps(StartOfTagName) == Left(Caps(TagString), Len(StartOfTagName) ) )
	{
		return true;
	}

	return false;
}


defaultproperties
{
     DrawType=DT_Sprite
     bReceiveDynamicShadows=True
     bLightingVisibility=True
     bUseDynamicLights=True
     bLightStaticEnv=True
     bLightDynamicEnv=True
     bAcceptsProjectors=True
     bReplicateMovement=True
     bReplicateAnimations=True
     bMovable=True
     bBlockZeroExtentTraces=True
     bBlockNonZeroExtentTraces=True
     bJustTeleported=True
     RemoteRole=ROLE_DumbProxy
     Role=ROLE_Authority
     NetUpdateFrequency=100
     NetPriority=1
     LODBias=1
     NavPtClass=Class'Engine.SquadMarkerSpot'
     Texture=Texture'Engine.S_Actor'
     DrawScale=1
     DrawScale3D=(X=1,Y=1,Z=1)
     ScaleGlow=1
     Style=STY_Normal
     IconScale=1
     CollisionRadius=22
     CollisionHeight=22
     Mass=100
     MaxKarmaSpeed=800
     MessageClass=Class'Engine.LocalMessage'
}

