class StatusEffect_Immolate extends StatusEffect;

function Activate( float Duration, float DamagePerSec )
{	
	local Pawn PawnOwner;
	local CTBot ControllerBot;	

	Super.Activate( Duration, DamagePerSec );

	PawnOwner = Pawn( Owner );

	/*
	if( PawnOwner.BurntMaterial != None )
	{
		PawnOwner.CopyMaterialsToSkins();
		PawnOwner.Skins[0] = PawnOwner.BurntMaterial;
	}
	*/

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

				ControllerBot = CTBot(PawnOwner.Controller);
				ControllerBot.ReflexGoals.Clear();
				//ControllerBot.MainGoals.Clear();
				ControllerBot.bFire = 0;

				if (PawnOwner.IsA('CloneCommando'))
				{
					if (RemainingTime > 5)
					{
						RemainingTime = 5;
						ControllerBot.AddReflexAnimGoal( 'OnFireBreathe' );
						ControllerBot.AddReflexAnimGoal( 'Extinguish' );
					}
					else
						ControllerBot.AddReflexAnimGoal( 'OnFireBreathe', '', true, RemainingTime );
				}
				else
					ControllerBot.AddReflexAnimGoal( 'OnFireBreathe', '', true, RemainingTime );
			}
		}
		else
		{
			//PlayerController(PawnOwner.Controller).BlendInMultColor( FadeColor, 0.25, Duration - 2, 1.75 );			
			//PlayerController(PawnOwner.Controller).AnimateBlur( 200, 0.25, Duration - 2, 1.75 );
		}
	}
}

function IncreaseDuration( float AdditionalTime )
{
	local Pawn PawnOwner;
	local CTBot ControllerBot;
	local float OldRemainingTime;

	OldRemainingTime = RemainingTime;

	Super.IncreaseDuration( AdditionalTime );
	
	PawnOwner = Pawn( Owner );
	if( PawnOwner != None && PawnOwner.Controller != None )
	{
		if( !PawnOwner.IsHumanControlled())
		{
			if (PawnOwner.CanChangeAnims())
			{
				ControllerBot = CTBot(PawnOwner.Controller);
				if (PawnOwner.IsA('CloneCommando'))
				{
					if (RemainingTime > 5)
					{
						RemainingTime = 5;
						ControllerBot.ReflexGoals.Clear();
						ControllerBot.AddReflexAnimGoal( 'OnFireBreathe' );
						ControllerBot.AddReflexAnimGoal( 'Extinguish' );
					}
					else
					{
						ControllerBot.ReflexGoals.Clear();
						ControllerBot.AddReflexAnimGoal( 'OnFireBreathe', '', true, RemainingTime );
					}
				}
				else
					ControllerBot.AddReflexAnimGoal( 'OnFireBreathe', '', true, AdditionalTime );
			}
		}
	}
}


defaultproperties
{
     DamageType=Class'CTGame.CTDamageFire'
     EffectClass=Class'CTEffects.GEO_Warrior_BodyBurn'
}

