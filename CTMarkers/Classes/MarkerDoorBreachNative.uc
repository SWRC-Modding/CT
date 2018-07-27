// ====================================================================
//  Class:  CTGame.DoorBreachMarker
//
//  Test Marker Object
// ====================================================================

class MarkerDoorBreachNative extends CTMarker
	native;

var   static Name DestroySwitchEvent; //this is going to destroy the lock
var	  static Name TossGrenadeEvent;
var(Marker)	class<GrenadeProj> GrenadeClass;
var(Marker) Actor GrenadeTarget;

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
	UBOOL Initiate(APawn* Instigator);
	UBOOL Update();
	virtual void ParticipantResume(APawn *Member, UGoalObject *CurrentGoal);
	//virtual bool SortParticipants(int PotentialParticipants, int TotalParticipants);

}

defaultproperties
{
     MaxMembers=3
     HUDIconY=7
     bCanCancelAfterUnderway=True
     InitiateCue=PAE_BreachInitiate
     ConfirmCue=PAE_BreachConfirm
     CancelCue=PAE_BreachCancel
     CancelConfirmCue=PAE_BreachCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

