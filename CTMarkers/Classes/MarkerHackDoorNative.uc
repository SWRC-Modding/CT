// ====================================================================
//  Class:  CTMarkers.MarkerSetTrap
//
//  Test Marker Object
// ====================================================================

class MarkerHackDoorNative extends CTMarker
	native;

//var Name PlaceEvent;

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



cpptext
{
	UBOOL Update();
	virtual UBOOL FinishMarker();
	virtual void OnCancel(APawn *Instigator);

}

defaultproperties
{
     HUDIconY=6
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_HackDoorInitiate
     ConfirmCue=PAE_HackDoorConfirm
     CancelCue=PAE_HackDoorCancel
     CancelConfirmCue=PAE_HackDoorCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

