class DeathCondition_Not extends DeathCondition_Logical;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( Super.IsValid( Target, DeathInfo ) &&
			 ( ConditionA == None || !ConditionA.IsValid( Target, DeathInfo ) ) &&
		     ( ConditionB == None || !ConditionB.IsValid( Target, DeathInfo ) ) );
}


defaultproperties
{
}

