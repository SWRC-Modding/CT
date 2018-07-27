// ====================================================================
//  Class:  CTMarkers.MarkerTurret
//
//  Turret Marker Object
// ====================================================================

class MarkerTurretNative extends CTMarker
	native;

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
	virtual UBOOL FinishMarker();
	//virtual void MemberKilled(APawn *Victim, AController *Killer, UClass *DamageType);

}

defaultproperties
{
     bCanCancelAfterUnderway=True
     bCancelIfMembersNeeded=True
     InitiateCue=PAE_TurretInitiate
     ConfirmCue=PAE_TurretConfirm
     CancelCue=PAE_TurretCancel
     CancelConfirmCue=PAE_TurretCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

