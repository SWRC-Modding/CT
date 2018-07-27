// ====================================================================
//  Class:  CTGame.ActivateDoor
//
//  Player Activation Item - Door Hack & Door Breach
//
// Created 2003, December 5, Jenny Huang
// ====================================================================

class ActivateDoor extends ActivateItem
	native;

var()	Actor				MatchingComponent;
var()	ActivationInfo		DoorLockedState;
var()	ActivationInfo		DoorUnlockedState;
var()	ActivationInfo		DoorDamagedState;
var() static float DamagedDelay;

simulated function InternalSetup()
{
	if (!IsInState('Locked'))
		GotoState('Locked');
	bActivating = true;
}

function UnlockDoor()
{
}

function OpenDoor()
{
}

function LockDoor()
{
}

function SetDoorCollision(bool collide)
{
}

state() Disabled
{
	function BeginState()
	{
		bIsUsable = false;
		MakeTransition(DoorLockedState);
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
		LockDoor();
	}
}

auto state() Locked
{	
	function BeginState()
	{
		bIsUsable = true;
		MakeTransition(DoorLockedState);
		//Marker states are totally driven by 
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.GotoState('Enabled');
			SquadMarker.bScriptInitialized = true;
		}
		LockDoor();
	}
	function Tick(float deltaTime)
	{
		if (User != None)
			BeginActivating(DoorLockedState, deltaTime);
	}
}

state() Unlocked
{
	function BeginState()
	{	
		bIsUsable = false;
		//set the state of objects that depend on us (Doors, Markers)
		//setting bScriptInitialized prevents others' autostates from
		//overriding our settings
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
		if (MatchingComponent != None && !MatchingComponent.IsInState('Unlocked'))
		{
			MatchingComponent.GotoState('Unlocked');
			MatchingComponent.bScriptInitialized = true;
		}
		UnlockDoor();
		MakeTransition(DoorUnlockedState);
	}
}

state Damaged
{
	function BeginState()
	{	
		bIsUsable = false;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
			SquadMarker.MakeObsolete();
		if (MatchingComponent != None && !MatchingComponent.IsInState('Damaged'))
			MatchingComponent.GotoState('Damaged');
		SetDoorCollision(false); //turn off collision on doors, so explosion can cause damage on either side
		MakeTransition(DoorDamagedState);
		SetDoorCollision(true);
	}
Begin:
	if (DamagedDelay > 0)
		Sleep(DamagedDelay);
	OpenDoor();

}



defaultproperties
{
     DamagedDelay=0.2
     bLockPlayerLocation=True
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     DrawType=DT_StaticMesh
}

