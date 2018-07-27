// ====================================================================
//  Class:  CTMarkers.MarkerDemolition
//
//  Test Marker Object
// ====================================================================

class MarkerDemolitionNative extends CTMarker
	native;

var static Name PlaceEvent;

//var class<Emitter> EffectClass;
//should we have sounds and anims lists for each character?
//or specifically labelled sounds and anims?
//the nice thing about this is it would generate the names at script time...

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	//UBOOL Initiate(APawn* Instigator);
	UBOOL Update();
	virtual UBOOL FinishMarker();
	virtual void OnCancel(APawn *Instigator);

}

defaultproperties
{
     HUDIconY=4
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_DemolitionInitiate
     ConfirmCue=PAE_DemolitionConfirm
     CancelCue=PAE_DemolitionCancel
     CancelConfirmCue=PAE_DemolitionCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

