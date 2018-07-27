class StatusEffect extends Actor;

var()	float				DamagePerSecond;
var()	class<DamageType>	DamageType;
var()	class<Emitter>		EffectClass;

var		Emitter				Effect;
var		float				RemainingTime;

replication
{
	reliable if( Role == ROLE_Authority )
		RemainingTime;
}


static function CreateStatusEffects( Actor InstigatingActor, float MaxDuration, float AffectRadius, float DamagePerSec )
{
	local Pawn P;	
	local float DistToP;
	local float EffectTime;

	ForEach InstigatingActor.VisibleCollidingActors( class 'Pawn', P, AffectRadius )
	{
		if( P == InstigatingActor )
			continue;

		DistToP = VSize( InstigatingActor.Location - P.Location );

		EffectTime = 1 - ( DistToP / ( AffectRadius - P.CollisionRadius ) );
		EffectTime = FClamp( EffectTime, 0, 1 );
		EffectTime *= MaxDuration;

		AddStatusEffectTo( P, InstigatingActor, EffectTime, DamagePerSec + (frand() - 0.5) );		
	}
}

static function AddStatusEffectTo( Actor Target, Actor InstigatingActor, float Duration, float DamagePerSec )
{
	local StatusEffect NewStatusEffect;
	local int i;

	if( ( !Target.IsA('Pawn') || !Pawn(Target).InGodMode() ) && Target.CalculateDamageFrom( default.DamageType, 100 ) != 0 )
	{
		for( i = 0; i < Target.Attached.length; i++ )
		{
			if( Target.Attached[i] != None && !Target.Attached[i].bDeleteMe && Target.Attached[i].IsA( 'StatusEffect' ) )
			{
				if( Target.Attached[i].IsA( default.Class.Name ) )
				{					
					StatusEffect( Target.Attached[i] ).IncreaseDuration( Duration );
					return;
				}
				else				
					return; // We only allow one status effect on a character at a time				
			}
		}
		
		NewStatusEffect = InstigatingActor.Spawn( default.Class, Target, , Target.Location, Target.Rotation );
		NewStatusEffect.SetBase( Target );
		NewStatusEffect.Activate( Duration, DamagePerSec );
	}
}

function IncreaseDuration( float AdditionalTime )
{
	RemainingTime += AdditionalTime;
}

simulated function PostNetBeginPlay()
{
	local Actor Victim;

	if( Owner != None )
		Victim = Owner;
	else if( Base != None )
		Victim = Base;

	if( EffectClass != None && Victim != None && !Victim.bDeleteMe && Level.NetMode != NM_DedicatedServer )
	{
		Effect = Spawn( EffectClass, Victim );
		Effect.SkeletalMeshActor = Victim;
		Effect.SetBase( Victim );
	}
}

function Activate( float Duration, float DamagePerSec )
{	
	RemainingTime = Duration;
	DamagePerSecond = DamagePerSec;
	if (Owner.IsA('Pawn'))
		Pawn(Owner).LastYelpTime = -1e10f; //always make sure they play the first sound
}


simulated function Tick( float DeltaTime )
{
	if( RemainingTime <= 0 || Owner == None || Owner.bDeleteMe )
	{
		Destroy();
		return;
	}

	RemainingTime -= DeltaTime;	

	if( DamagePerSecond > 0 && Role == ROLE_Authority )	
	{
		Owner.TakeDamage( DamagePerSecond * DeltaTime, Instigator, Owner.Location, vect(0,0,0), DamageType );
		if (Owner.IsA('Pawn'))
			Pawn(Owner).LastYelpTime = Level.TimeSeconds;
	}
	
}

simulated function Destroyed()
{
	if( Effect != None )
		Effect.FadeOut();

	Super.Destroyed();	
}


defaultproperties
{
     DamageType=Class'Engine.DamageType'
     RemainingTime=1
     bHidden=True
     RemoteRole=ROLE_SimulatedProxy
}

