class DeathCondition_BoneName extends DeathCondition;

var(DeathCondition) name BoneName;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( Super.IsValid( Target, DeathInfo ) && ( DeathInfo.HitBone == BoneName ) );
}


defaultproperties
{
}

