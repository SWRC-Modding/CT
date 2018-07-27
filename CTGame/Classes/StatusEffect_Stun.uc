class StatusEffect_Stun extends StatusEffect;

var() color	FlashColor;
var() float NightVisionFlashScale;
var() int	FlashBloom;

static function CreateStatusEffects( Actor InstigatingActor, float MaxDuration, float AffectRadius, float DamagePerSec )
{
	local Pawn P;
	local vector DirToP;
	local float DistToP;
	local float StunTime;

	foreach InstigatingActor.VisibleCollidingActors( class 'Pawn', P, AffectRadius )
	{
		if( !P.bDeleteMe )
		{
			DirToP = InstigatingActor.Location - P.Location;
			DistToP = VSize( DirToP );
			DirToP /= DistToP;

			if( !P.IsHumanControlled() ||
				( ( DirToP Dot vector(P.Controller.Rotation) ) > cos( P.Controller.FovAngle * 0.5 * 3.14159 / 180.0 ) ) )
			{
				StunTime = 1 - ( DistToP / ( AffectRadius - P.CollisionRadius ) );
				StunTime = FClamp( StunTime, 0, 1 );
				StunTime *= MaxDuration;

				AddStatusEffectTo( P, InstigatingActor, StunTime, DamagePerSec );				
			}
		}
	}
}

function Activate( float Duration, float DamagePerSec )
{	
	local Pawn PawnOwner;
	local CTBot ControllerBot;

	Super.Activate( Duration, DamagePerSec );	

	PawnOwner = Pawn( Owner );
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

				if( Duration > 6.5 && frand() > 0.5 )				
					ControllerBot.AddReflexAnimGoal( 'Stunned' );				
				else
					ControllerBot.AddReflexAnimGoal( 'StunnedBreathe', '', true, Duration );
			}
			else if( PawnOwner.IsA( 'Turret' ) )
			{
				ControllerBot = CTBot(PawnOwner.Controller);
				ControllerBot.AddReflexSleepGoal( Duration );
				ControllerBot.SetEnemy(None);
				ControllerBot.Target = None;
			}
		}
		else
		{			
			if( PlayerController(PawnOwner.Controller).Helmet.FrameFX.VisionMode == PawnOwner.VisionPawnUser[2] )
			{
				FlashColor.R = Min( 255, FlashColor.R * NightVisionFlashScale );
				FlashColor.G = Min( 255, FlashColor.G * NightVisionFlashScale );
				FlashColor.B = Min( 255, FlashColor.B * NightVisionFlashScale );
			}
			PlayerController(PawnOwner.Controller).BlendInAddColor( FlashColor, 0.25, Duration - 2, 1.75 );
			PlayerController(PawnOwner.Controller).AnimateBloomFilter( FlashBloom, 0.25, Duration - 2, 1.75 );
			PlayerController(PawnOwner.Controller).AnimateBlur( 200, 0.25, Duration - 2, 1.75 );
		}
	}
}

function IncreaseDuration( float AdditionalTime )
{
	local Pawn PawnOwner;
	local float InTime, SustainTime, FadeOutTime, HoldFraction;

	Super.IncreaseDuration( AdditionalTime );

	PawnOwner = Pawn( Owner );
	if( PawnOwner != None && PawnOwner.Controller != None )
	{
		if( !PawnOwner.IsHumanControlled())
		{
			if (PawnOwner.CanChangeAnims())
				AIController(PawnOwner.Controller).AddReflexAnimGoal( 'StunnedBreathe', '', true, AdditionalTime );
			else if( PawnOwner.IsA( 'Turret' ) )			
				AIController(PawnOwner.Controller).AddReflexSleepGoal( AdditionalTime );			
		}
		else
		{
			InTime = 0.1;
			HoldFraction = 0.75;
			SustainTime = RemainingTime * HoldFraction;
			FadeOutTime = RemainingTime - SustainTime - InTime;

			if( PlayerController(PawnOwner.Controller).Helmet.FrameFX.VisionMode == PawnOwner.VisionPawnUser[2] )
			{
				FlashColor.R = Min( 255, FlashColor.R * NightVisionFlashScale );
				FlashColor.G = Min( 255, FlashColor.G * NightVisionFlashScale );
				FlashColor.B = Min( 255, FlashColor.B * NightVisionFlashScale );
			}
			PlayerController(PawnOwner.Controller).BlendInAddColor( FlashColor, InTime, SustainTime, FadeOutTime );
			PlayerController(PawnOwner.Controller).AnimateBloomFilter( FlashBloom, InTime, SustainTime, FadeOutTime );
			PlayerController(PawnOwner.Controller).AnimateBlur( 200, InTime, SustainTime, FadeOutTime );			
		}
	}
}


defaultproperties
{
     FlashColor=(B=180,G=180,R=180)
     NightVisionFlashScale=1.5
     FlashBloom=255
     DamageType=Class'CTGame.CTDamageStun'
}

