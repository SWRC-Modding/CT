//=============================================================================
// KarmaActor.
// Just a handy class to derive off to make physics objects.
//=============================================================================

class KActor extends Actor
	native
	placeable;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var(Karma)	bool		bKTakeShot;
var(Karma)	bool		bOrientImpactEffect;
var			bool		bTimingOut;			// we are in the process of timing out

// Ragdoll impact sounds.
var(Karma) array<sound>		ImpactSounds;
var(Karma) float			ImpactVolume;

var() class<actor>			ImpactEffect;
var(Karma) class<DamageType> ImpactDamageType;
var(Karma) int				Health;				// How much health a ragdoll has
var(Karma) float			LifeTime;			// How long after activation does this ragdoll wait to de-res
var(Karma) int				DamageThreshold;	// How much damage to take before going into Karma

var(Karma) float			ImpactInterval;
var transient float			LastImpactTime;

var(Karma) float			ImpactDamageMultiplier;
var(Karma) float			ImpactDamageMinVelocity;

// CL: Simple replication, isn't pretty, look at kcar for better
// example of network karma replication
replication
{
	reliable if(Role == ROLE_Authority)
		ImpactEffect, bOrientImpactEffect, ImpactInterval, LastImpactTime;
}

function PostBeginPlay()
{
	if( KParams != None && KarmaParams(KParams).KStartEnabled && LifeTime > 0 )
	{		
		LifeSpan = LifeTime;
		bTimingOut = true;
	}

	Super.PostBeginPlay();
}

// Default behaviour when shot is to apply an impulse and kick the KActor.
function float TakeDamage(float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName)
{
	if( !bTimingOut && LifeTime > 0 )
	{
		bTimingOut = true;
		LifeSpan = LifeTime;
	}

	if( Health != 0 )
	{
		Health -= Damage;
		if( Health <= 0 )
			Destroy();
	}

	DamageThreshold -= Damage;

	if(bKTakeShot && DamageThreshold <= 0)
	{		
		KWake();

		TriggerEvent(Event,self,None);

		if( damageType.default.KApplyImpulseToRoot )
		{
			KAddImpulse(momentum,HitLocation,'root');			
		}
		else
		{
			KAddImpulse(momentum,HitLocation);
		}
	}

	return Damage;
}

// Default behaviour when triggered is to wake up the physics.
function Trigger( actor Other, pawn EventInstigator )
{
	if( !bTimingOut && LifeTime > 0 )
	{		
		bTimingOut = true;
		LifeSpan = LifeTime;
	}

	bKTakeShot = true;

	if( (Physics != PHYS_Karma) && (Physics != PHYS_KarmaRagdoll) )
		SetPhysics(default.Physics);
	
	KWake();
}

// 
event KImpact(actor other, vector pos, vector impactVel, vector impactNorm, EMaterialType MaterialHit)
{
	local int numSounds, soundNum;

	// If its time for another impact.
	if(Level.TimeSeconds > LastImpactTime + ImpactInterval)
	{
		// If we have some sounds, play a random one.
		numSounds = ImpactSounds.Length;
		if(numSounds > 0)
		{
			soundNum = Rand(numSounds);			
			PlaySound(ImpactSounds[soundNum], , ImpactVolume);
		}
		
		// If we have an effect class (and its relevant), spawn it.
		if( (ImpactEffect != None) && EffectIsRelevant(pos, false) )
		{
			if(bOrientImpactEffect)
				spawn(ImpactEffect, self, , pos, rotator(impactNorm));
			else
				spawn(ImpactEffect, self, , pos);
		}
		
		LastImpactTime = Level.TimeSeconds;
	}
}

event bool EncroachingOn( actor Other )
{
	local float Speed;

	if( ImpactDamageMultiplier > 0 && Other != None && !Other.bDeleteMe && Other.IsA('Pawn') || Other.IsA('KarmaProp') )
	{
		Speed = VSize( Velocity );
		if( Speed >= ImpactDamageMinVelocity )
			Other.TakeDamage( Speed * ImpactDamageMultiplier, None, Other.Location, Velocity, ImpactDamageType );
	}

	return false;
}



cpptext
{
	virtual void Spawned();

}

defaultproperties
{
     bKTakeShot=True
     ImpactDamageType=Class'Engine.Crushed'
     Physics=PHYS_Karma
     DrawType=DT_StaticMesh
     bWorldGeometry=True
     bCollideActors=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     bBlockKarma=True
     RemoteRole=ROLE_None
     CollisionRadius=1
     CollisionHeight=1
     bEdShouldSnap=True
}

