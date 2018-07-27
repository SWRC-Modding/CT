// ====================================================================
//  Class:  CTGame.HackDoor
//
//  Player Activation Item - Hack Panel
//
// Created 2003, December 2, Jenny Huang
// ====================================================================

class HackDoor extends ActivateDoor
	;

auto state Locked
{	
	function NextState()
	{
		GotoState('Unlocked');
	}
}



defaultproperties
{
     DoorLockedState=(Duration=10)
     DoorUnlockedState=(TransitionSound=Sound'GEN_Sound.Interface.consoleTypeConfirm_01')
     DoorDamagedState=(TransitionSound=Sound'GEN_Sound.Interface.consoleTypeConfirm_01')
     ActivateItemType=AIT_HackDoor
     bSavesProgress=True
     bNonPlayerUsesTimer=True
     HUDIconY=6
     DurationRemaining=10
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     NavPtLocs(0)=(Offset=(X=116),Yaw=32768)
}

