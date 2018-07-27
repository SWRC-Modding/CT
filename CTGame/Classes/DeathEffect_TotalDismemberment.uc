class DeathEffect_TotalDismemberment extends DeathEffect;


function DoDeathEffect( Pawn Target, DeathEffectInfo DeathInfo )
{
	Target.CompletelyDismember( VSize( DeathInfo.Momentum ) );

	if( Target.DeathEffect != None )
		Target.Spawn( Target.DeathEffect );

	Target.Destroy();
}



defaultproperties
{
}

