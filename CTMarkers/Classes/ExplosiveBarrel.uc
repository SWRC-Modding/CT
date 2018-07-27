// ====================================================================
//  Class:  CTMarkers.ExplosiveBarrel
//  Parent: Engine.KarmaProp
//
//  Base Class for Props
// ====================================================================

class ExplosiveBarrel extends KarmaProp
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
		SetPhysics(PHYS_Karma);
		Super.BeginState();

		if (DemoCharge != None)
			DemoCharge.bIsUsable = false;

		if (SquadMarker != None)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.Destroy();
		}
	}
}

simulated function NotifyTargeted( PlayerController Player )
{
	if (DemoCharge != None)
		DemoCharge.NotifyTargeted(Player);
	super.NotifyTargeted(Player);
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	if (DemoCharge != None)
		DemoCharge.NotifyUnTargeted(Player);
	super.NotifyUnTargeted(Player);
}



defaultproperties
{
     bKTakeShot=False
     Health=60
     ExplosionRadius=640
     ExplosionDamage=1024
     ExplosionDamageType=Class'CTGame.CTDamageExplosion'
     DestroyedState=(TransitionSound=Sound'GEN_Sound.Impacts_Explos.exp_GEN_barrel_01',ImpulseDir=(Z=1),ImpulseMagnitude=150,StateLifeSpan=5)
     Physics=PHYS_None
     Begin Object Class=KarmaParams Name=ExplosiveBarrelKParams
         KLinearDamping=0
         KAngularDamping=0
         KBuoyancy=1
         bHighDetailOnly=False
         KFriction=0.8
         KRestitution=0.1
         KImpactThreshold=100
         Name="ExplosiveBarrelKParams"
     End Object
     KParams=KarmaParams'CTMarkers.ExplosiveBarrel.ExplosiveBarrelKParams'
     bValidEnemy=True
}

