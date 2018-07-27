//=============================================================================
// Scout used for path generation.
//=============================================================================
class Scout extends Pawn
	native
	notplaceable;

var const float MaxLandingVelocity;

function PreBeginPlay()
{
	Destroy(); //scouts shouldn't exist during play
}


defaultproperties
{
     DamageMultipliers(0)=(BoneName="Head",Multiplier=2)
     AccelRate=1
     bCollideActors=False
     bCollideWorld=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
     CollisionRadius=52
     bPathColliding=True
}

