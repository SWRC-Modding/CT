// ====================================================================
//  Class:  CTMarkers.ExplosiveBarrel
//  Parent: Engine.KarmaProp
//
//  Base Class for Props
// ====================================================================

class DemolitionCrate extends Prop
	placeable;

var(Marker) ActivateItem DemoCharge;

event bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(DemoCharge.Event);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return Super.ContainsPartialEvent(StartOfEventName);
}


state() PropDestroyed
{
	function BeginState()
	{
		DestroyedState.bDisableCollision = true;
		Super.BeginState();
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			if (DemoCharge != None)
				DemoCharge.Destroy();
			SquadMarker.Destroy();
		}
	}
}


defaultproperties
{
     ExplosionRadius=512
     ExplosionDamage=1536
     ExplosionDamageType=Class'CTGame.CTDamageExplosion'
     DestroyedState=(TransitionEffect=Class'CTEffects.Explosion_Lrg_Barrel',TransitionSound=Sound'GEN_Sound.Impacts_Explos.exp_GEN_crate_01')
}

