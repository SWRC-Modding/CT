// ====================================================================
//  Class:  CTMarkers.MarkerSplitUp
//
//  Test Marker Object
// ====================================================================

class MarkerSnipe extends MarkerWeapon
	placeable;


defaultproperties
{
     Accuracy=1
     FOV=0.9
     CompleteCue=PAE_SnipeComplete
     ActionString="Sniping"
     PreferredPawnClass=Class'CTCharacters.Commando07Delta'
     InitiateCue=PAE_SnipeInitiate
     ConfirmCue=PAE_SnipeConfirm
     CancelCue=PAE_SnipeCancel
     CancelConfirmCue=PAE_SnipeCancelConfirm
     ActivatePromptText="PRESS @ TO ENGAGE SNIPE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL SNIPE MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Sniper.SniperSM1ActionBreathe'
}

