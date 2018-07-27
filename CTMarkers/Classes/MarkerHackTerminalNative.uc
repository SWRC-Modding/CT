// ====================================================================
//  Class:  CTMarkers.MarkerHackTerminal
//
// ====================================================================

class MarkerHackTerminalNative extends CTMarker
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
     HUDIconY=5
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_HackTerminalInitiate
     ConfirmCue=PAE_HackTerminalConfirm
     CancelCue=PAE_HackTerminalCancel
     CancelConfirmCue=PAE_HackTerminalCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

