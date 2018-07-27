class DeathEffect_PlayDying extends DeathEffect;


function DoDeathEffect( Pawn Target, DeathEffectInfo DeathInfo )
{
    Target.PlayDying( DeathInfo.DamageType, DeathInfo.HitLocation, DeathInfo.HitBone );
}


defaultproperties
{
}

