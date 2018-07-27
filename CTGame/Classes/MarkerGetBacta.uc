// ====================================================================
//  Class:  CTMarkers.MarkerSplitUp
//
//  Test Marker Object
// ====================================================================

class MarkerGetBacta extends MarkerGetBactaNative
	placeable;


defaultproperties
{
     MarkerAnims(0)="BactaDispenser"
     ActionString="Obtaining Bacta"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.Revive.reviveSM1BactaDispenser')
     IrrelevantCue=PAE_BactaNotNeeded
     ActivatePromptText="PRESS @ TO ENGAGE HEAL MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL HEAL MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     IrrelevantPromptText="SQUAD HEALTH CURRENTLY AT MAXIMUM"
     StaticMesh=StaticMesh'MarkerIcons.Revive.reviveSM1BactaDispenser'
}

