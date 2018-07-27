// ====================================================================
//  Class:  CTMarkers.MarkerReviveOther
//
//  Test Marker Object
// ====================================================================

class MarkerReviveOther extends CTMarker
	native
	placeable
	noautoload;

var float Duration;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)




cpptext
{
	virtual UBOOL Update();
	virtual void CompleteMarker();

}

defaultproperties
{
     Duration=2.7
     HUDIconX=1
     HUDIconY=7
     ActionString="Assisting SquadMember"
     Participants(0)=(IconMesh=StaticMesh'MarkerIcons.Revive.reviveSM1HealImmobile')
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_ReviveInitiate
     CancelCue=PAE_ReviveCancel
     CancelConfirmCue=PAE_ReviveCancelConfirm
     ActivatePromptText="PRESS @ TO ENGAGE REVIVE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL REVIVE MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Revive.reviveSM1HealImmobile'
     InitialState="Disabled"
}

