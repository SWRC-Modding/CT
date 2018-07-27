// ====================================================================
//  Class:  CTMarkers.MarkerSetTrap
//
//  Test Marker Object
// ====================================================================

class MarkerHackDoor extends MarkerHackDoorNative
	placeable;


defaultproperties
{
     MarkerAnims(0)="HackUnlockDoorStart"
     MarkerAnims(1)="HackUnlockDoorLoop"
     MarkerAnims(2)="HackUnlockDoorStop"
     ActionString="Hacking Panel"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.DoorHackUnlock.DoorHackUnlockSM1Breathe',bIconOnAnchorPt=True)
     PreferredPawnClass=Class'CTCharacters.Commando40Delta'
     ActivatePromptText="PRESS @ TO ENGAGE DOOR SLICE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL DOOR SLICE MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.DoorHackUnlock.DoorHackUnlockSM1Breathe'
}

