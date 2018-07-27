//=============================================================================
// ParticleEmitter: Base class for sub- emitters.
//
// make sure to keep structs in sync in UnParticleSystem.h
//=============================================================================

class ParticleEmitter extends Object
	abstract
	editinlinenew
	native;

enum EBlendMode
{
	BM_MODULATE,
	BM_MODULATE2X,
	BM_MODULATE4X,
	BM_ADD,
	BM_ADDSIGNED,
	BM_ADDSIGNED2X,
	BM_SUBTRACT,
	BM_ADDSMOOTH,
	BM_BLENDDIFFUSEALPHA,
	BM_BLENDTEXTUREALPHA,
	BM_BLENDFACTORALPHA,
	BM_BLENDTEXTUREALPHAPM,
	BM_BLENDCURRENTALPHA,
	BM_PREMODULATE,
	BM_MODULATEALPHA_ADDCOLOR,
	BM_MODULATEINVALPHA_ADDCOLOR,
	BM_MODULATEINVCOLOR_ADDALPHA,
	BM_HACK	
};

enum EParticleDrawStyle
{
	PTDS_Regular,
	PTDS_AlphaBlend,
	PTDS_Modulated,
	PTDS_Translucent,
	PTDS_AlphaModulate_MightNotFogCorrectly,
	PTDS_Darken,
	PTDS_Brighten
};

enum EParticleCoordinateSystem
{
	PTCS_Independent,
	PTCS_Relative,
	PTCS_Absolute
};

enum EParticleVelocityDirection
{
	PTVD_None,
	PTVD_StartPositionAndOwner,
	PTVD_OwnerAndStartPosition,
	PTVD_AddRadial
};

enum EParticleStartLocationShape
{
	PTLS_Box,
	PTLS_MovingBox,
	PTLS_Sphere,
	PTLS_Polar,
	PTLS_All
};

enum EParticleEffectAxis
{
	PTEA_NegativeX,
	PTEA_PositiveZ
};

enum EParticleCollisionSound
{
	PTSC_None,
	PTSC_LinearGlobal,
	PTSC_LinearLocal,
	PTSC_Random
};

enum ESkelLocationUpdate
{
	PTSU_None,
	PTSU_SpawnOffset,
	PTSU_Location
};

struct ParticleTimeScale
{
	var () float	RelativeTime;		// always in range [0..1]
	var () float	RelativeSize;
};

struct ParticleColorScale
{
	var () float	RelativeTime;		// always in range [0..1]
	var () color	Color;
};

struct Particle
{
	var vector	Location;
	var vector	OldLocation;
	var vector	Velocity;
	var vector	StartSize;
	var vector	SpinsPerSecond;
	var vector	StartSpin;
	var vector  RevolutionCenter;
	var vector  RevolutionsPerSecond;
	var vector	RevolutionsMultiplier;
	var vector	Size;
	var vector  StartLocation;
	var vector  ColorMultiplier;	
	var vector	OldMeshLocation;
	var color	Color;
	var float	Time;
	var float	MaxLifetime;	
	var int		HitCount;
	var int		Flags;
	var int		Subdivision;
	var int 	BoneIndex;
};

struct ParticleSound
{
	var () sound	Sound;
	var () range	Radius;
	var () range	Pitch;
	var () int		Weight;
	var () range	Volume;
	var () range	Probability;
};

// Vars
var (Acceleration)	vector						Acceleration;

var (Collision)		vector						ExtentMultiplier;
var (Collision)		rangevector					DampingFactorRange;
var (Collision)		array<plane>				CollisionPlanes;
var (Collision)		range						MaxCollisions;
var (Collision)		int							SpawnFromOtherEmitter;
var (Collision)		int							SpawnAmount;
var (Collision)		rangevector					SpawnedVelocityScaleRange;

var (Color)			array<ParticleColorScale>	ColorScale;
var (Color)			float						ColorScaleRepeats;
var (Color)			rangevector					ColorMultiplierRange;
var (Color)			float						Opacity; // Added by Demiurge (PSE)

var (Fading)		plane						FadeOutFactor;
var (Fading)		float						FadeOutStartTime;
var (Fading)		plane						FadeInFactor;
var (Fading)		float						FadeInEndTime;

var (General)		EParticleCoordinateSystem	CoordinateSystem;
var (General)		const int					MaxParticles;
var (General)		EParticleEffectAxis			EffectAxis;

var (Local)			range						AutoResetTimeRange;
var (Local)			string						Name;

var (Location)		vector						StartLocationOffset;
var (Location)		rangevector					StartLocationRange;
var (Location)		rangevector					EndLocationRange;
var (Location)		rangevector					CurrentLocationRange;
var (Location)		float						LocationChangeTime;
var (Location)		float						LocationSpentTime;
var (Location)		int							AddLocationFromOtherEmitter;
var (Location)		EParticleStartLocationShape StartLocationShape;
var (Location)		range						SphereRadiusRange;
var (Location)		rangevector					StartLocationPolarRange;

var (Rendering)		int							AlphaRef;

var (Revolution)	rangevector					RevolutionCenterOffsetRange;
var (Revolution)	rangevector					RevolutionsPerSecondRange;

var (Rotation)		rotator						RotationOffset;
var (Rotation)		vector						SpinCCWorCW;
var (Rotation)		rangevector					SpinsPerSecondRange;
var (Rotation)		rangevector					StartSpinRange;
var (Rotation)		rangevector					RotationDampingFactorRange;

var (Size)			array<ParticleTimeScale>	SizeScale;
var (Size)			float						SizeScaleRepeats;
var (Size)			rangevector					StartSizeRange;

var (SkeletalMesh)	ESkelLocationUpdate			UseSkeletalLocationAs;
var (SkeletalMesh)	range						RelativeBoneIndexRange;
var (SkeletalMesh)	array<name>					SkeletalBoneNames;
var					array<int>					SkeletalBoneIndices;

var (Sound)			Sound						CollideSound;
var (Sound)			float						CollisionSoundProbability;

var (Spawning)		float						ParticlesPerSecond;
var (Spawning)		float						InitialParticlesPerSecond;
	
var (Texture)		EParticleDrawStyle			DrawStyle;
var (Texture)		texture						Texture;
var (Texture)		int							TextureUSubdivisions;
var (Texture)		int							TextureVSubdivisions;
var (Texture)		array<float>				SubdivisionScale;
var (Texture)		int							SubdivisionStart;
var (Texture)		int							SubdivisionEnd;
var (Texture)		bool						BlendBetweenSubdivisions;
var	(Texture)		bool						UseSubdivisionScale;
var (Texture)		bool						UseRandomSubdivision;

var (Tick)			float						SecondsBeforeInactive;
var (Tick)			float						MinSquaredVelocity;

var	(Time)			range						InitialTimeRange;
var (Time)			range						LifetimeRange;
var (Time)			range						InitialDelayRange;

var (Velocity)		rangevector					StartVelocityRange;
var (Velocity)		range						StartVelocityRadialRange;
var (Velocity)		vector						MaxAbsVelocity;
var (Velocity)		rangevector					VelocityLossRange;

var (Warmup)		float						WarmupTicksPerSecond;
var (Warmup)		float						RelativeWarmupTime;

var transient		emitter						Owner;
var transient		float						InactiveTime;
var transient		array<Particle>				Particles;
var transient		int							ParticleIndex;			// index into circular list of particles
var transient		int							ActiveParticles;		// currently active particles
var transient		float						PPSFraction;			// used to keep track of fractional PPTick
var transient		box							BoundingBox;

var transient		vector						RealExtentMultiplier;
var	transient		int							OtherIndex;
var transient		float						InitialDelay;
var transient		vector						GlobalOffset;
var transient		float						TimeTillReset;
var transient		int							PS2Data;
var transient		int							MaxActiveParticles;
var transient		int							CurrentCollisionSoundIndex;
var transient		int							CurrentSpawningSoundIndex;
var transient		int							CurrentMeshSpawningIndex;
var transient		float						MaxSizeScale;
var transient		int							KillPending;
var transient		int							DeferredParticles;
var transient		vector						RealMeshNormal;
var transient		array<vector>				MeshVertsAndNormals;

// Flags
var (Collision)		bool						UseCollision;
var (Collision)		bool						UseCollisionPlanes;
var	(Collision)		bool						UseMaxCollisions;
var (Collision)		bool						UseSpawnedVelocityScale;

var (Color)			bool						UseColorScale;

var (Fading)		bool						FadeIn;
var (Fading)		bool						FadeOut;

var (General)		bool						ResetAfterChange;

var (Local)			bool						RespawnDeadParticles;
var (Local)			bool						AutoDestroy;
var (Local)			bool						AutoReset;
var (Local)			bool						Disabled;
var (Local)			bool						DisableFogging;

var (MeshSpawning)	bool						VelocityFromMesh;
var (MeshSpawning)	bool						UniformMeshScale;
var (MeshSpawning)	bool						UniformVelocityScale;
var (MeshSpawning)	bool						UseColorFromMesh;
var (MeshSpawning)	bool						SpawnOnlyInDirectionOfNormal;

var (Rendering)		bool						AlphaTest;
var (Rendering)		bool						AcceptsProjectors;
var (Rendering)		bool						ZTest;
var (Rendering)		bool						ZWrite;
var (Rendering)		bool						WorldDistortion;

var (Revolution)	bool						UseRevolution;
var (Revolution)	bool						UseRevolutionScale;

var (Rotation)		bool						SpinParticles;
var (Rotation)		bool						DampRotation;

var (Size)			bool						UseSizeScale;
var (Size)			bool						UseRegularSizeScale;
var (Size)			bool						UniformSize;

var (Spawning)		bool						AutomaticInitialSpawning;

var (Trigger)		bool						TriggerDisabled;
var (Trigger)		bool						ResetOnTrigger;

var (Velocity)		bool						UseVelocityScale;

var	transient		bool						Initialized;
var transient		bool						Inactive;
var	transient		bool						RealDisableFogging;
var transient		bool						AllParticlesDead;
var transient		bool						WarmedUp;


native function SpawnParticle( int Amount );
native function Trigger();
native function SetStartLocationRange( float XMax, float XMin, float YMax, float YMin, float ZMax, float ZMin );


defaultproperties
{
     ExtentMultiplier=(X=1,Y=1,Z=1)
     DampingFactorRange=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     SpawnFromOtherEmitter=-1
     ColorMultiplierRange=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     Opacity=1
     FadeOutFactor=(W=1,X=1,Y=1,Z=1)
     FadeInFactor=(W=1,X=1,Y=1,Z=1)
     MaxParticles=10
     EndLocationRange=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     CurrentLocationRange=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     LocationChangeTime=1
     AddLocationFromOtherEmitter=-1
     SpinCCWorCW=(X=0.5,Y=0.5,Z=0.5)
     StartSizeRange=(X=(Min=100,Max=100),Y=(Min=100,Max=100),Z=(Min=100,Max=100))
     RelativeBoneIndexRange=(Max=1)
     DrawStyle=PTDS_Translucent
     SecondsBeforeInactive=1
     LifetimeRange=(Min=4,Max=4)
     MaxAbsVelocity=(X=10000,Y=10000,Z=10000)
     RespawnDeadParticles=True
     AlphaTest=True
     ZTest=True
     UseRegularSizeScale=True
     AutomaticInitialSpawning=True
     TriggerDisabled=True
}

