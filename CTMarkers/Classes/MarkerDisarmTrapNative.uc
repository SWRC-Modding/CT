// ====================================================================
//  Class:  CTMarkers.MarkerDisarmTrap
//
//  Test Marker Object
// ====================================================================

class MarkerDisarmTrapNative extends CTMarker
	native;

var static Name RemoveEvent;

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
// (cpptext)



cpptext
{
	UBOOL Initiate(APawn* Instigator);
	UBOOL Update();
	virtual void ParticipantResume(APawn *Member, UGoalObject *CurrentGoal);
	virtual UBOOL FinishMarker();
	virtual void OnCancel(APawn *Instigator);

}

defaultproperties
{
     HUDIconY=2
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_DisarmTrapInitiate
     ConfirmCue=PAE_DisarmTrapConfirm
     CancelCue=PAE_DisarmTrapCancel
     CancelConfirmCue=PAE_DisarmTrapCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

