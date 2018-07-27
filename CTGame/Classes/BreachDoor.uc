// ====================================================================
//  Class:  CTGame.BreachDoor
//
//  Player Activation Item - Door Breach
//
// Created 2003, December 8, Jenny Huang
// ====================================================================

class BreachDoor extends ActivateDoor
	;

var()	ActivationInfo		DoorChargeLockedState;
var()	Mover				DoorLeft;
var()	Mover				DoorRight;

simulated function InternalSetup()
{
	super.InternalSetup();
	SetStaticMesh(DoorChargeLockedState.Mesh);
}

simulated function AbortSetup()
{
	super.AbortSetup();
	SetStaticMesh(DoorLockedState.Mesh);
}

function AttachToDoor()
{
	SetPhysics(PHYS_MovingBrush);
	if (DoorRight != None)
		SetBase(DoorRight);
}

function PreBeginPlay()
{
	super.PreBeginPlay();
	if (DoorLeft != None)
		DoorLeft.SquadMarker = SquadMarker;
	if (DoorRight != None)
		DoorRight.SquadMarker = SquadMarker;
	AttachToDoor();
}


auto state() Locked
{	
	function NextState()
	{
		GotoState('ChargeLocked');
	}
}

function UnlockDoor()
{
	//Log("BreachDoor::UnlockDoor");
	AttachToDoor();
	DoorLeft.GotoState('TriggerControl');
	DoorRight.GotoState('TriggerControl');
	DoorLeft.bScriptInitialized = true;
	DoorRight.bScriptInitialized = true;
}

function OpenDoor()
{
	AttachToDoor();
	DoorLeft.StayOpenTime = 4.0;
	DoorRight.StayOpenTime = 4.0;
	DoorLeft.GotoState('TriggerControl', 'Open');
	DoorRight.GotoState('TriggerControl', 'Open');
	PostStimulusToWorld(ST_DoorBreach);
}

function LockDoor()
{
	DoorLeft.GotoState('Locked', 'Close');
	DoorRight.GotoState('Locked', 'Close' );
	DoorLeft.bScriptInitialized = true;
	DoorRight.bScriptInitialized = true;
	if (MatchingComponent != None && !MatchingComponent.IsInState('Locked'))
	{
		MatchingComponent.GotoState('Locked');
		MatchingComponent.bScriptInitialized = true;
	}
}

function SetDoorCollision(bool collide)
{
	DoorLeft.SetCollision(collide, collide, collide);
	DoorRight.SetCollision(collide, collide, collide);
}


state ChargeLocked
{
	function BeginState()
	{
		DeductAmmo();
		//make the hack-panel unusable
		if (MatchingComponent != None)
		{
			ActivateItem(MatchingComponent).bIsUsable = false;
			if (MatchingComponent.SquadMarker != None && !MatchingComponent.SquadMarker.bMarkerActive)
				MatchingComponent.SquadMarker.MakeObsolete();
		}
		MakeTransition(DoorChargeLockedState);
		SetTimer(DoorChargeLockedState.Duration, false);
		//If the marker is not running, disable it once the charge is placed
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
		}
		bIsUsable = false;
	}
	function Timer()
	{
		GotoState('Damaged');
	}
}


defaultproperties
{
     DoorChargeLockedState=(TransitionSound=Sound'GEN_Sound.Impacts_Explos.imp_GEN_proxMineSet_01',Duration=2.5)
     DoorLockedState=(TransitionSound=Sound'GEN_Sound.Interface.int_GEN_doorLocked_01',Duration=2)
     DoorUnlockedState=(TransitionSound=Sound'GEN_Sound.Interface.int_GEN_doorUnlocked_01')
     DoorDamagedState=(TransitionSound=Sound'GEN_Sound.Impacts_Explos.exp_GEN_smallElec_01',MyDamageType=Class'CTGame.CTDamageExplosion',Damage=200,DamageRadius=256)
     DamagedDelay=0
     ActivateItemType=AIT_BreachDoor
     HUDIconY=7
     PlayerOffset=87
     AttachItemClass=Class'CTGame.DoorChargeObject'
     ActivatePromptText="PRESS AND HOLD @ TO BREACH DOOR"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     NavPtLocs(0)=(Offset=(X=194),Yaw=32221)
     NavPtLocs(1)=(Offset=(X=268,Y=210),Yaw=25189)
     NavPtLocs(2)=(Offset=(X=408,Y=230),Yaw=24029)
}

