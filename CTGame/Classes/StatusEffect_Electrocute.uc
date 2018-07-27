class StatusEffect_Electrocute extends StatusEffect;

var() color	FadeColor;

static function CreateStatusEffects( Actor InstigatingActor, float MaxDuration, float AffectRadius, float DamagePerSec )
{
	local DroidDispenser Dispenser;	
	local float DistToDispenser;
	local float EffectTime;

	Super.CreateStatusEffects( InstigatingActor, MaxDuration, AffectRadius, DamagePerSec );	

	foreach InstigatingActor.VisibleCollidingActors( class 'DroidDispenser', Dispenser, AffectRadius )
	{
		DistToDispenser = VSize( InstigatingActor.Location - Dispenser.Location );

		EffectTime = 1 - ( DistToDispenser / ( AffectRadius - Dispenser.CollisionRadius ) );
		EffectTime = FClamp( EffectTime, 0, 1 );
		EffectTime *= MaxDuration;

		AddStatusEffectTo( Dispenser, InstigatingActor, EffectTime, DamagePerSec );		
	}
}

function Activate( float Duration, float DamagePerSec )
{	
	local Pawn PawnOwner;
	local CTBot ControllerBot;

	Super.Activate( Duration, DamagePerSec );

	PawnOwner = Pawn( Owner );

	//only make it last half as long on clones
	if (PawnOwner.IsA('CloneCommando'))
		RemainingTime = RemainingTime/2.0;

	if( PawnOwner != None && PawnOwner.Controller != None )
	{
		if( !PawnOwner.IsHumanControlled() )
		{
			if( PawnOwner.CanChangeAnims() )
			{
				if( PawnOwner.Weapon != None )
					PawnOwner.Weapon.ServerStopFire();

				PawnOwner.Velocity = vect(0,0,0);
				PawnOwner.Acceleration = vect(0,0,0);

				ControllerBot = CTBot(PawnOwner.Controller) ;
				ControllerBot.ReflexGoals.Clear();
				//ControllerBot.MainGoals.Clear();
				ControllerBot.bFire = 0;

				ControllerBot.AddReflexAnimGoal( 'ElectrocutionBreathe', '', true, RemainingTime );				
			}
		}
		else
		{
			PlayerController(PawnOwner.Controller).BlendInMultColor( FadeColor, 0.25, Duration - 2, 1.75 );			
			PlayerController(PawnOwner.Controller).AnimateBlur( 200, 0.25, Duration - 2, 1.75 );
		}
	}
}

function IncreaseDuration( float AdditionalTime )
{
	local Pawn PawnOwner;

	Super.IncreaseDuration( AdditionalTime );

	PawnOwner = Pawn( Owner );
	if( PawnOwner != None && PawnOwner.Controller != None )
	{
		if( !PawnOwner.IsHumanControlled())
		{
			if (PawnOwner.CanChangeAnims())
				AIController(PawnOwner.Controller).AddReflexAnimGoal( 'ElectrocutionBreathe', '', true, AdditionalTime );
		}
		else
		{
			if( RemainingTime > 2 )
			{
				PlayerController(PawnOwner.Controller).BlendInMultColor( FadeColor, 0.25, RemainingTime - 2, 1.75 );			
				PlayerController(PawnOwner.Controller).AnimateBlur( 200, 0.25, RemainingTime - 2, 1.75 );
			}
		}
	}
}


defaultproperties
{
     FadeColor=(B=255,G=255,R=255)
     DamageType=Class'CTGame.CTDamageElectricity'
     EffectClass=Class'CTEffects.EMP_Electrocution'
}

