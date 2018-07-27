// ====================================================================
//  Class:  CTMarkers.MarkerAntiArmor
//
//  Test Marker Object
// ====================================================================

class MarkerAntiArmor extends MarkerWeapon
	placeable;



defaultproperties
{
     MarkerWeaponType=MWT_AntiArmor
     CompleteCue=PAE_AntiArmorComplete
     HUDIconY=9
     ActionString="Anti-Armor"
     PreferredPawnClass=Class'CTCharacters.Commando62Delta'
     InitiateCue=PAE_AntiArmorInitiate
     ConfirmCue=PAE_AntiArmorConfirm
     CancelCue=PAE_AntiArmorCancel
     CancelConfirmCue=PAE_AntiArmorCancelConfirm
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.Sniper.AntiArmorSM1FireAntiArmor'
}

