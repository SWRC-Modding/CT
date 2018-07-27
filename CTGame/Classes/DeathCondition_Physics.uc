class DeathCondition_Physics extends DeathCondition;

var(DeathCondition) Actor.EPhysics	Physics;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( Super.IsValid( Target, DeathInfo ) && Target.Physics == Physics );
}


defaultproperties
{
}

