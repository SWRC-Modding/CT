class DeathCondition_And extends DeathCondition_Logical;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( Super.IsValid( Target, DeathInfo ) && ( ConditionA.IsValid( Target, DeathInfo ) && ConditionB.IsValid( Target, DeathInfo ) ) );
}


defaultproperties
{
}

