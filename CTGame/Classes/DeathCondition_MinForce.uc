class DeathCondition_MinForce extends DeathCondition;

var(DeathCondition) float MinForce;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( Super.IsValid( Target, DeathInfo ) && VSize( DeathInfo.Momentum ) > MinForce );
}


defaultproperties
{
}

