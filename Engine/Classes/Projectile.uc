//=============================================================================
// Projectile.
//
// A delayed-hit projectile that moves around for some time after it is created.
//=============================================================================
class Projectile extends Actor
	abstract
	native;

//-----------------------------------------------------------------------------
// Projectile variables.

// Motion information.
var()		float   Speed;               // Initial speed of projectile.
var()		float   MaxSpeed;            // Limit on speed of projectile (0 means no limit)
var			float	TossZ;
var			Actor	ZeroCollider;
var			bool	bSwitchToZeroCollision; // if collisionextent nonzero, and hit actor with bBlockNonZeroExtents=0, switch to zero extent collision
var			bool	bDelayDestroy;
var			bool	bLaserBolt;
var()		int		AccelFactor;		// How fast to accelerate 

// Damage attributes.
var()   float				Damage; 
var()	float				DamageRadius;
var		class<DamageType>	MyDamageType;

// Projectile sound effects
var()   sound			SpawnSound;			// Sound made when projectile is spawned.
var()   sound			ImpactSound;		// Sound made when projectile hits something.
var()	EStimulusType	WallHitStimulus;

// Impact FX
var()	class<Emitter>		ProjectileEffect;
var		Emitter				ParticleFX;
var		float				ExploWallOut;	// distance to move explosions out from wall

// Used by missiles. Declared in projectile for dependency reasons
var   Actor			   Target;

var() class<MaterialTypeInfo>	HitEffectsByMaterial;
var() float						HitEffectsProbability;


replication
{
	reliable if (Role==ROLE_Authority)
		Target;
}

native static function vector GetTossVelocity(Pawn P, Rotator R);

simulated function Destroyed()
{
	if( ParticleFX != None )
	{
		if( bLaserBolt )
			ParticleFX.Destroy();
		else
			ParticleFX.FadeOut();
	}
		
	Super.Destroyed();
}

simulated function DelayDestroy()
{
	Enable('Tick');
	bDelayDestroy = true;
}

//TODO: Replicate acceleration on netInitial
//TODO: Double tick bug
function PostBeginPlay()
{
	super.PostBeginPlay();

	if ( bLaserBolt )
	{
		Velocity = vect(0,0,0);
		Acceleration = vector( Rotation ) * AccelFactor;
		NetPriority+=10.0f; // make this a very high priority
	}
	else
		Velocity = Speed * vector(Rotation);
}

simulated function PostNetBeginPlay()
{
	local Weapon InstigatorWeapon;

	Super.PostNetBeginPlay();

	//gdr We need to tick laster bolts because their velocity is updated in tick.
	if(!bLaserBolt)
		Disable('Tick');

	if ( Level.bDropDetail )
	{
		LightType = LT_None;
		bDynamicLight = false;
	}
	
	if ((None != ProjectileEffect) && ( Level.NetMode != NM_DedicatedServer ))
	{
		ParticleFX = Spawn(ProjectileEffect);
		ParticleFX.SetBase(self);
	}

	if ( (Instigator != None) && (Instigator.Weapon != None) )
	{
		InstigatorWeapon = Instigator.Weapon;
		if ( InstigatorWeapon.bWeaponZoom && InstigatorWeapon.ZoomDamageMultiplier != 1.0 )
			Damage *= Instigator.Weapon.ZoomDamageMultiplier;
		else
			Damage = default.Damage;
	}
}

simulated function Tick(float deltaTime)
{
	super.Tick(deltaTime);

	//Log("Tick "$self$" Time "$Level.TimeSeconds$" Velocity "$Velocity$" Accel "$Acceleration$" AccelFactor "$AccelFactor$" Damage "$Damage);

	if (bDelayDestroy)
	{
		Destroy();
		bDelayDestroy = false;
	}
}

//==============
// Encroachment
function bool EncroachingOn( actor Other )
{
	if ( (Other.Brush != None) || (Brush(Other) != None) )
		return true;
		
	return false;
}

//==============
// Touching
simulated singular function Touch(Actor Other)
{
	local actor HitActor;
	local vector HitLocation, HitNormal, VelDir, AdjustedLocation;
	local bool bBeyondOther;
	local float BackDist, DirZ;
	local name HitBone;

	if ( Other == None ) // Other just got destroyed in its touch?
		return;
	if ( Other.bProjTarget || (Other.bBlockActors && Other.bBlockPlayers) )
	{
		if ( Velocity == vect(0,0,0) || Other.IsA('Mover') ) 
		{
			ProcessTouch(Other,Location);
			return;
		}
		
		//get exact hitlocation - trace back along velocity vector
		bBeyondOther = ( (Velocity Dot (Location - Other.Location)) > 0 );
		VelDir = Normal(Velocity);
		DirZ = sqrt(abs(VelDir.Z));
		BackDist = Other.CollisionRadius * (1 - DirZ) + Other.CollisionHeight * DirZ;
		if ( bBeyondOther )
			BackDist += VSize(Location - Other.Location);
		else
			BackDist -= VSize(Location - Other.Location);

	 	HitActor = Trace(HitLocation, HitNormal, Location, Location - 1.1 * BackDist * VelDir, true,,, HitBone);
		if (HitActor == Other)
			AdjustedLocation = HitLocation;
		else if ( bBeyondOther )
			AdjustedLocation = Other.Location - Other.CollisionRadius * VelDir;
		else
			AdjustedLocation = Location;

		if( bLaserBolt && Other.ReflectLaser( self, AdjustedLocation, HitNormal, HitBone ) )
			ReflectProjectile( HitNormal );
		else
		{
			ProcessTouch(Other, AdjustedLocation, HitBone);
			if ( (Role < ROLE_Authority) && (Other.Role == ROLE_Authority) )
				ClientSideTouch(Other, AdjustedLocation);
		}
	}
}

/* ClientSideTouch()
Allows client side actors (with Role==ROLE_Authority on the client, like ragdolls)
to be affected by projectiles
*/
simulated function ClientSideTouch(Actor Other, Vector HitLocation)
{
	if( Damage > 0 )
		Other.TakeDamage(Damage, instigator, Location, Normal(Velocity) * MyDamageType.default.KDamageImpulse, MyDamageType);
}

simulated function ProcessTouch(Actor Other, vector HitLocation, optional name BoneName)
{
	if( Other != Instigator )
	{
		if ( Role == ROLE_Authority && Damage > 0)
		{
			Other.TakeDamage( Damage, instigator, HitLocation, Normal(Velocity) * MyDamageType.default.KDamageImpulse, MyDamageType, BoneName);
			Damage = 0;
		}

		PostStimulusToWorld(ST_WeaponHitDefault);
		SpawnEffect( Other, HitLocation, Normal(-Velocity), Other.TypeOfMaterial );		
	}	

	Destroy();
	//DelayDestroy();
}

simulated function HitWall(vector HitNormal, actor Wall, EMaterialType KindOfMaterial)
{
	if( bLaserBolt && Wall.ReflectLaser( self, Location, HitNormal ) )
		ReflectProjectile( HitNormal );
	else
	{
		if ( Role == ROLE_Authority && Damage > 0 )
		{			
			Wall.TakeDamage( Damage, instigator, Location, Normal(Velocity) * MyDamageType.default.KDamageImpulse, MyDamageType);
			PostStimulusToWorld(WallHitStimulus);
			Damage = 0;
		}	

		SpawnEffect(None, Location, HitNormal, KindOfMaterial);

		Destroy();
		//DelayDestroy();
	}
}

simulated function SpawnEffect(Actor Other, vector HitLocation, vector HitNormal, EMaterialType HitMaterialType)
{	
	if ( EffectIsRelevant(HitLocation,false) )
	{
		if( HitEffectsByMaterial != None && frand() <= HitEffectsProbability )
		{
			if( Other == None || !Other.IsA('Pawn') || Pawn(Other).Controller == None || !Pawn(Other).Controller.IsA('PlayerController') )
			{
				HitEffectsByMaterial.Static.SpawnEffectsFor(self, HitMaterialType, HitLocation, HitNormal);
			}
		}	
	}
}

simulated final function RandSpin(float spinRate)
{
	DesiredRotation = RotRand();
	RotationRate.Yaw = spinRate * 2 * FRand() - spinRate;
	RotationRate.Pitch = spinRate * 2 * FRand() - spinRate;
	RotationRate.Roll = spinRate * 2 * FRand() - spinRate;	
}

simulated function ReflectProjectile( vector HitNormal )
{	
	Velocity = Velocity - 2 * (Velocity dot HitNormal) * HitNormal;	
}

auto State Flying
{
Begin:
	LifeSpan = 10.0;
}


defaultproperties
{
     MaxSpeed=2000
     TossZ=100
     AccelFactor=60000
     MyDamageType=Class'Engine.DamageType'
     WallHitStimulus=ST_WeaponHitDefault
     HitEffectsProbability=1
     Physics=PHYS_Projectile
     DrawType=DT_Mesh
     bAcceptsProjectors=False
     bNetTemporary=True
     bReplicateInstigator=True
     bUpdateSimulatedPosition=True
     bNetInitialRotation=True
     bUnlit=True
     bGameRelevant=True
     bCanBeDamaged=True
     bCollideActors=True
     bCollideWorld=True
     bBlockZeroExtentTraces=False
     bBlockNonZeroExtentTraces=False
     bUseCylinderCollision=True
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=2.5
     LifeSpan=14
     Texture=Texture'Engine.S_Camera'
     CollisionRadius=0
     CollisionHeight=0
}

