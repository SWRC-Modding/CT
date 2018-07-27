// ====================================================================
//  Class:  CTMarkers.MarkerSplitUp
//
//  Test Marker Object
// ====================================================================

class MarkerGetBactaNative extends CTMarker
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)




cpptext
{
	UBOOL Update();
	virtual UBOOL IsRelevantFor(APawn *Pawn) { return (Pawn->HealthLevel < HL_Green); }
	virtual bool SortParticipants(int PotentialParticipants, int TotalParticipants);

}

defaultproperties
{
     HUDIconX=1
     HUDIconY=7
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_BactaInitiate
     ConfirmCue=PAE_HealthAcknowledge
     CancelCue=PAE_BactaCancel
     CancelConfirmCue=PAE_BactaCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

