class DeathEffect_DestroyBody extends DeathEffect;

function DoDeathEffect( Pawn Target, DeathEffectInfo DeathInfo )
{
	if( Target.DeathEffect != None )
		Target.Spawn( Target.DeathEffect );

	Target.Destroy();
}

defaultproperties
{
}

