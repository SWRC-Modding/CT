// ====================================================================
//  Class:  CTMarkers.MarkerProximityMine
//
//  Test Marker Object
// ====================================================================

class MarkerSetProximityMine extends MarkerSetProximityMineNative
	placeable;


defaultproperties
{
     PlaceEvent="MproxMinePlace"
     ArmEvent="ProxMineArm"
     HUDIconY=3
     MarkerAnims(0)="SetProximityMine"
     ActionString="Setting Charge"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.SetTrap.TrapSM1ProximityMineSet',bIconOnAnchorPt=True)
     bCanCancelAfterUnderway=True
     PreferredPawnClass=Class'CTCharacters.Commando62Delta'
     InitiateCue=PAE_SetTrapInitiate
     ConfirmCue=PAE_SetTrapConfirm
     CancelCue=PAE_SetTrapCancel
     CancelConfirmCue=PAE_SetTrapCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.SetTrap.TrapSM1ProximityMineSet'
}

