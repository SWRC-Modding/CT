class DeathEffect_PartialDismemberment extends DeathEffect;

var(DeathEffect) bool AllowRagdoll;

function DoDeathEffect( Pawn Target, DeathEffectInfo DeathInfo )
{
	Target.AmbientSound = None;
    Target.bCanTeleport = false; 
    Target.bReplicateMovement = false;
    Target.bTearOff = true;
    Target.bPlayedDeath = true;
		
	Target.HitDamageType = DeathInfo.DamageType; // these are replicated to other clients
    Target.TakeHitLocation = DeathInfo.HitLocation;

	Target.GotoState('Dying');

	if( DeathInfo.HitBone != 'root' )	
		Target.PartiallyDismember( DeathInfo.HitBone, DeathInfo.HitLocation, DeathInfo.Momentum );	

	if( AllowRagdoll && Target.TryToRagdoll( DeathInfo.DamageType, DeathInfo.HitBone ) && Target.GoRagdoll( DeathInfo.DamageType, DeathInfo.HitLocation ) )
	{
		return;
	}
	else
	{
		if( Target.Physics != PHYS_Falling && ( Target.Physics != PHYS_Flying || Target.HasAnim('FlyDeath') ) )
		{
			Target.PlayDyingAnim(DeathInfo.DamageType, DeathInfo.HitLocation);				
			Target.Velocity = vect(0,0,0);
			Target.Acceleration = vect(0,0,0);
			
			if( Target.bFallOnDeath )
				Target.SetPhysics(PHYS_Falling);
			else
				Target.SetPhysics(PHYS_None);
		}
		else
		{
			Target.CompletelyDismember(VSize(DeathInfo.Momentum));
			Target.Destroy();
			return;
		}
	}
}


defaultproperties
{
}

