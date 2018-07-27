class Accessory extends Actor native;

enum EAccessoryDetachStyle
{
	ADS_BlowUp,
	ADS_BlowOut
};

var() bool IsDetachable;						// Can we be detached?
var() bool Explosive;							// Do we explode when shot?
var() bool Invulnerable;
var	  bool WentKarma;

var() class<Emitter> ExplosionEffect;			// Explosion Effect
var() class<DamageType> ExplosionDamageType;	// Explosion Damage Type
var() float ExplosionRadius;					// Explosion Radius
var() float ExplosionDamage;					// Explosion Damage

var() float DetachedLifespan;					// Lifespan after detachment
var() float MinDetachForce;						// The minimum force require to detach this actor
var() float	Health;								// Amount of health for given accessory
var() float ArmorFactor;						// Amount of damage absorbed by this armor

var() float MaxDetachImpulse;					// Maximum size of the impulse to detach something

var() EAccessoryDetachStyle DetachStyle;		// How to apply impulse when detaching

var() class<Emitter>	TrailerClass;			// an effect to trail the part after it's been detached
var	  Emitter			Trailer;

function PostBeginPlay()
{
    Super.PostBeginPlay();

	if( Explosive )
		ArmorFactor = 1.0;
}

function DetachAccessory(vector HitLocation, vector Momentum)
{
	local float OldMomentumMag, NewMometnumMag;

	if( Base != None && AttachmentBone != '' )
		Base.DetachFromBone( self );

	SetOwner( None );
	bProjTarget = true;
	bBlockZeroExtentTraces = true;
	//bProjTarget = false;	
	//bBlockZeroExtentTraces = false;
	bBlockNonZeroExtentTraces = false;
	SetCollision( true, false, false );
	SetPhysics( PHYS_Karma );
	KSetBlockKarma( true );	
	Lifespan = ( DetachedLifespan * 0.5 ) + ( frand() * DetachedLifespan * 0.5 );
	WentKarma = true;
	Level.ActiveKarmaAccessories++;

	if( TrailerClass != None )
	{
		Trailer = Spawn( TrailerClass );
        Trailer.SetBase( self );
	}

	if( !VIsZero( Momentum ) )
	{
		OldMomentumMag = VSize( Momentum );
		NewMometnumMag = FMin( OldMomentumMag, MaxDetachImpulse );
        KAddImpulse( Momentum * ( NewMometnumMag / OldMomentumMag ), HitLocation );
	}
}

simulated function Destroyed()
{
	if( Trailer != None )
		Trailer.FadeOut();

	if( WentKarma )
		Level.ActiveKarmaAccessories--;

	Super.Destroyed();
}

function float TakeDamage( float Damage, Pawn InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional Name BoneName )
{
	local float MomentumMag;
	local vector DetachImpulse;

	if( Invulnerable )
		return 0;

	Damage = CalculateDamageFrom(DamageType, Damage);
	Health -= Damage * ArmorFactor;

	if( Physics == PHYS_Karma )
		KAddImpulse(momentum,HitLocation);

	if( Damage > 0 && Base != None && AttachmentBone != '' && !DamageType.default.KApplyImpulseToRoot )
	{
		Base.TakeDamage( Damage * (1-ArmorFactor), InstigatedBy, HitLocation, Momentum, DamageType, AttachmentBone );
	}

	if( Health <= 0 )
	{		
		if( IsDetachable && Base != None && AttachmentBone != '' )
		{
			MomentumMag = VSize( Momentum );					
			if( MomentumMag >= MinDetachForce && Level.ActiveKarmaAccessories < Level.MaxKarmaAccessories )
			{				
				if( DetachStyle == ADS_BlowUp )
				{					
					DetachImpulse = Momentum;					
					if( abs(DetachImpulse.Z) < abs(DetachImpulse.X) && abs(DetachImpulse.Z) < abs(DetachImpulse.Y) )
						DetachImpulse.Z += MomentumMag;

					DetachAccessory(HitLocation, DetachImpulse);					
				}
				else if( DetachStyle == ADS_BlowOut )
				{					
					DetachAccessory(HitLocation, Normal(Location - Base.Location) * MomentumMag);
				}				
			}
			else if( Health != -1 )
			{
				Spawn( ExplosionEffect, , , Location, Rotation );	
				Destroy();
			}
		}
		else if( Health != -1 )
		{
			// The DamageToRoot check is a quick hack to prevent explosions from setting the attachment off
			// It just doesn't feel right to have explosions set off an arm attachment	
			if( Explosive && !DamageType.default.KApplyImpulseToRoot )
				HurtRadius( ExplosionDamage, ExplosionRadius, ExplosionDamageType, ExplosionDamageType.default.KDamageImpulse, Location );			

			Spawn( ExplosionEffect, , , Location, Rotation );	
			Destroy();
		}
	}		

	return Damage;
}


defaultproperties
{
     DetachedLifespan=10
     MinDetachForce=100
     Health=-1
     MaxDetachImpulse=2000
     DrawType=DT_StaticMesh
     bCollideActors=True
     bProjTarget=True
     bBlockNonZeroExtentTraces=False
     CollisionRadius=1
     CollisionHeight=1
     Begin Object Class=KarmaParams Name=AccessoryKParams
         KLinearDamping=0
         KAngularDamping=0
         KBuoyancy=1
         KStartEnabled=True
         bHighDetailOnly=False
         KFriction=0.8
         KRestitution=0.3
         KImpactThreshold=100
         Name="AccessoryKParams"
     End Object
     KParams=KarmaParams'Engine.Accessory.AccessoryKParams'
}

