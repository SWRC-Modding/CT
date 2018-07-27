// ====================================================================
//  Class:  CTGame.KarmaProp
//  Parent: CTGame.Prop
//
//  Base Class for Props
// ====================================================================

class KarmaProp extends Prop
	native
	showcategories(Karma)
	placeable;

var(Karma) bool		bKTakeShot;
var(Karma) bool		bTakeDamageOnWorldImpact;
var(Karma) bool		bOrientImpactEffect;
var(Karma) EStimulusType ImpactStimulus;

var(Karma) int		DamageThreshold;

var(Karma) float	ImpactDamageMultiplier;
var(Karma) float	ImpactDamageMinVelocity;
var(Karma) class<DamageType> ImpactDamageType;

var(Karma) array<sound>		ImpactSounds;
var(Karma) float			ImpactVolume;
var(Karma) class<actor>		ImpactEffect;
var(Karma) float			LifeTime;			// How long after activation does this prop wait to auto de-res
var(Karma) float			ImpactInterval;
var transient float			LastImpactTime;

function PostBeginPlay()
{
	if( bTakeDamageOnWorldImpact )
		KParams.KImpactThreshold = 10;

	Super.PostBeginPlay();
}

function Timer()
{
	if( Health > 0 )
		TakeDamage( Health, none, Location, vect(0,0,0), class'DamageType' );
	else
		GotoState('PropDestroyed');
}

// Default behaviour when shot is to apply an impulse and kick the KActor.
function AffectKarma(int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, class<DamageType> damageType)
{
	local vector ApplyImpulse;

	TriggerEvent(Event,self,None);

	DamageThreshold -= Damage;

	if(bKTakeShot && DamageThreshold <= 0)
	{
		if( LifeTime > 0 )
			SetTimer( LifeTime, false );

		KWake();
		ApplyImpulse = momentum;		
		
		KAddImpulse(ApplyImpulse,HitLocation);		
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

event KImpact( Actor Other, vector Pos, vector ImpactVel, vector ImpactNorm, EMaterialType MaterialHit)
{
	local float Speed;
	local int numSounds;
	local int soundNum;

	// If its time for another impact.
	if(Level.TimeSeconds > LastImpactTime + ImpactInterval)
	{
		if (ImpactStimulus != ST_Invalid)
			PostStimulusToWorld(ImpactStimulus);

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

	if( bTakeDamageOnWorldImpact && ImpactDamageMultiplier > 0 && (Other == None || Other.bWorldGeometry) )
	{
		// NathanM: This is a hack to prevent a karma crash bug that happens if you deal damage right away
		if( ExplosionRadius == 0 )
			ExplosionRadius = 1;
		if( ExplosionDamage == 0 )
            ExplosionDamage = 1;

		Speed = VSize( Velocity );
		if( Speed >= ImpactDamageMinVelocity )
			TakeDamage( Speed * ImpactDamageMultiplier, None, Location, Velocity, class'Crushed' );
	}
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{
	AffectKarma( Damage, instigatedBy, hitlocation, momentum, damageType );
	return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );	
}

auto state Invulnerable
{
	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		AffectKarma( Damage, instigatedBy, hitlocation, momentum, damageType );
		return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );
	}	
}

state() Damagable
{
	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		AffectKarma( Damage, instigatedBy, hitlocation, momentum, damageType );
		return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );
	}
}

state() PropDamaged
{
	function BeginState()
	{
		if( DamagedState.ImpulseMagnitude > 0 )
			AffectKarma( 0, None, Location, DamagedState.ImpulseDir * DamagedState.ImpulseMagnitude, DamagedState.ImpulseDamageType );
		Super.BeginState();		
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		AffectKarma( Damage, instigatedBy, hitlocation, momentum, damageType );
		return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );
	}
}

state() PropDestroyed
{
	function BeginState()
	{
		bKTakeShot = true;
		Super.BeginState();	
		if( DestroyedState.ImpulseMagnitude > 0 )
			AffectKarma( 0, None, Location, DestroyedState.ImpulseDir * DestroyedState.ImpulseMagnitude, DestroyedState.ImpulseDamageType );
	}

	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{
		AffectKarma( Damage, instigatedBy, hitlocation, momentum, damageType );
		return Super.TakeDamage( Damage, instigatedBy, hitlocation, momentum, damageType, BoneName );
	}
}

state() Exploding
{
	function Timer()
	{
		Super.Timer();
		if( DestroyedState.ImpulseMagnitude > 0 )
			AffectKarma( 0, None, Location, DestroyedState.ImpulseDir * DestroyedState.ImpulseMagnitude, DestroyedState.ImpulseDamageType );
	}
}

// Default behaviour when triggered is to wake up the physics.
function Trigger( actor Other, pawn EventInstigator )
{
	bKTakeShot = true;

	if( Physics != PHYS_Karma )
		SetPhysics(PHYS_Karma);

	if( LifeTime > 0 )
		SetTimer( LifeTime, false );

	KWake();
}


defaultproperties
{
     bKTakeShot=True
     ImpactDamageType=Class'Engine.Crushed'
     Physics=PHYS_Karma
     Begin Object Class=KarmaParams Name=KarmaPropKParams
         KAngularDamping=0.1
         KBuoyancy=1
         bHighDetailOnly=False
         KFriction=0.8
         KRestitution=0.1
         KImpactThreshold=100
         Name="KarmaPropKParams"
     End Object
     KParams=KarmaParams'Engine.KarmaProp.KarmaPropKParams'
}

