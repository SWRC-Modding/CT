//=============================================================================
// Emitter: An Unreal Emitter Actor.
//=============================================================================
class Emitter extends Actor
	native
	placeable;

#exec Texture Import File=Textures\S_Emitter.pcx  Name=S_Emitter Mips=Off MASKED=1


var()	export	editinline	array<ParticleEmitter>	Emitters;

var		(Global)		bool				AutoDestroy;
var		(Global)		bool				AutoReset;
var		(Global)		bool				DisableFogging;
var		(Global)		rangevector			GlobalOffsetRange;
var		(Global)		range				TimeTillResetRange;
var		(Global)		actor				SkeletalMeshActor;

var		(Sound)		Sound				SpawnSound;

var		(Bloom)	int						BloomFilter;
var		(Bloom)	float					InTime;
var		(Bloom)	float					SustainTime;
var		(Bloom)	float					OutTime;

var		(CameraShake)	float			ShakeRadius;
var		(CameraShake)	vector			ShakeMag;
var		(CameraShake)	rotator			ShakeRotationMag;
var		(CameraShake)	float			ShakeInTime;
var		(CameraShake)	float			ShakeSustainTime;
var		(CameraShake)	float			ShakeOutTime;
var		(CameraShake)	float			ShakeFrequency;

var		transient	int					Initialized;
var		transient	box					BoundingBox;
var		transient	float				EmitterRadius;
var		transient	float				EmitterHeight;
var		transient	bool				ActorForcesEnabled;
var		transient	vector				GlobalOffset;
var		transient	float				TimeTillReset;
var		transient	bool				UseParticleProjectors;
var		transient	ParticleMaterial	ParticleMaterial;
var		transient	bool				DeleteParticleEmitters;

// shutdown the emitter and make it auto-destroy when the last active particle dies.
native function Kill();
 
simulated function PostNetBeginPlay()
{
	local int i;
	local PlayerController PC;
	local float DistToPC, FadeFactor;

	bMovable = true;

	if( AutoDestroy )
	{
		// This is a temp fix for several visual problems. Eventually, all AutoDestroy/NonPlaceable emitters should have
		// seconds before inactive set to 0		
		for( i=0; i < Emitters.Length; i++ )
		{
			if( Emitters[i] != None )
				Emitters[i].SecondsBeforeInactive = 0;
		}
	}	

	if( ( Level.NetMode == NM_StandAlone || Role < ROLE_Authority ) && ( BloomFilter > 0 || ShakeRadius > 0 ) )
	{		
		PC = Level.GetLocalPlayerController();		
		if( PlayerCanSeeMe( PC ) )
		{
			if( BloomFilter > 0 )
				PC.AnimateBloomFilter( BloomFilter, InTime, SustainTime, OutTime );

			if( PC.Pawn != None )
				DistToPC = VSize( PC.Pawn.Location - Location );
			else
				DistToPC = VSize( PC.Location - Location );

			if( DistToPC < ShakeRadius )
			{
				FadeFactor = DistToPC / ShakeRadius;
				PC.ShakeView(ShakeInTime, ShakeSustainTime, ShakeOutTime, ShakeMag.X * FadeFactor, ShakeMag.Y * FadeFactor, ShakeMag.Z * FadeFactor, ShakeRotationMag.Yaw * FadeFactor, ShakeRotationMag.Pitch * FadeFactor, ShakeFrequency);
			}
		}
	}
}


simulated function UpdatePrecacheMaterials()
{
	local int i;
	for( i=0; i<Emitters.Length; i++ )
	{
		if( Emitters[i] != None )
		{
			if( Emitters[i].Texture != None )
				Level.AddPrecacheMaterial(Emitters[i].Texture);
		}
	}
}

event Trigger( Actor Other, Pawn EventInstigator )
{
	local int i;
	for( i=0; i<Emitters.Length; i++ )
	{
		if( Emitters[i] != None )
			Emitters[i].Trigger();
	}
}

simulated function FadeOut()
{
	local int i;	
	
	for( i = 0; i < Emitters.length; i++ )
	{
		if( Emitters[i] != None )
		{
			Emitters[i].RespawnDeadParticles = false;
			Emitters[i].AutoDestroy = true;
			Emitters[i].SecondsBeforeInactive = 0;
			Emitters[i].MaxActiveParticles = Emitters[i].ActiveParticles;
		}
	}
	
	AutoDestroy = true;	
}

simulated function Destroyed()
{
	Super.Destroyed();
	if(SpawnSound != None)
		StopSound( self, SpawnSound, 0.0 );
}




defaultproperties
{
     ShakeFrequency=1
     DrawType=DT_Particle
     bUnlit=True
     bUseHWOcclusionTests=True
     RemoteRole=ROLE_None
     Texture=Texture'Engine.S_Emitter'
     Style=STY_Particle
}

