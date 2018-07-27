// ====================================================================
//  Class:  CTMarkers.MarkerHackTerminal
//
// ====================================================================

class MarkerHackTerminal extends MarkerHackTerminalNative
	placeable;

//var Name PlaceEvent;

//var class<Emitter> EffectClass;
//should we have sounds and anims lists for each character?
//or specifically labelled sounds and anims?
//the nice thing about this is it would generate the names at script time...


defaultproperties
{
     MarkerAnims(0)="HackUnlockConStationStart"
     MarkerAnims(1)="HackUnlockConStationLoop"
     MarkerAnims(2)="HackUnlockConStationStop"
     ActionString="Hacking Terminal"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.DoorHackUnlock.DoorHackUnlockSM1Breathe',bIconOnAnchorPt=True)
     PreferredPawnClass=Class'CTCharacters.Commando40Delta'
     ActivatePromptText="PRESS @ TO ENGAGE SLICE CONSOLE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL SLICE CONSOLE MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.DoorHackUnlock.DoorHackUnlockSM1Breathe'
}

