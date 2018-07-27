// ====================================================================
//  Class:  CTMarkers.MarkerDisarmTrap
//
//  Test Marker Object
// ====================================================================

class MarkerTriggerLoop extends CTMarker
	native
	placeable;

var(Marker) Name CancelUntriggerEvent;
var(Marker) Name CancelTriggerEvent;
var(Marker) Actor DirectTrigger;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

event bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(CancelUntriggerEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	EventString = string(CancelTriggerEvent);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}
	return Super.ContainsPartialEvent(StartOfEventName);
}




cpptext
{
	UBOOL Update();
	virtual UBOOL FinishMarker();
	virtual UBOOL CanInitiateFor(APawn *Pawn);
	//virtual UBOOL IsRelevantFor(APawn *Pawn);

}

defaultproperties
{
     HUDIconY=2
     MarkerAnims(0)="HackUnlockDoorStart"
     MarkerAnims(1)="HackUnlockDoorLoop"
     MarkerAnims(2)="HackUnlockDoorStop"
     ActionString="Using Panel"
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_HackDoorInitiate
     ConfirmCue=PAE_HackDoorConfirm
     CancelCue=PAE_HackDoorCancel
     CancelConfirmCue=PAE_HackDoorCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

