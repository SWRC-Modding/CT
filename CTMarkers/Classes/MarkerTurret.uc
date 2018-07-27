// ====================================================================
//  Class:  CTMarkers.MarkerTurret
//
//  Turret Marker Object
// ====================================================================

class MarkerTurret extends MarkerTurretNative
	placeable
	showcategories(DisplayAdvanced);

simulated function EMarkerAction CurrentMarkerAction()
{
	if (bDisabledByAlternate || !Turret(Anchor).IsAvailableFor(None))
		return MA_None;
	return MA_Activate;
}


defaultproperties
{
     ActionString="Manning Turret"
     ActivatePromptText="PRESS @ TO ENGAGE TURRET MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL TURRET MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Turret.TurretSM1StandBreathe'
}

