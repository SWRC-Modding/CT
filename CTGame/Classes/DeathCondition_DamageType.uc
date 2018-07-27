class DeathCondition_DamageType extends DeathCondition;

var(DeathCondition) class<DamageType> DamageType;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	if( DamageType != None )
		return ( Super.IsValid( Target, DeathInfo ) && DeathInfo.DamageType.IsA( DamageType.Name ) );
	else
		return false;
}


defaultproperties
{
}

