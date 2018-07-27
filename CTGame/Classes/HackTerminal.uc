// ====================================================================
//  Class:  CTGame.HackTerminal
//
// Created 2004, February 5, Jenny Huang
// May 20, 2004, John Hancock, 
// ====================================================================

class HackTerminal extends ActivateItem
	native;
	
var()	ActivationInfo		DisabledState;
var()	ActivationInfo		StandbyState;
var()	ActivationInfo		HackedState;
var()	ActivationInfo		ActiveState;

var	Actor OptionalDesk;

simulated function AbortSetup()
{
	super.AbortSetup();
	if (IsInState('Active'))
		GotoState('Standby');
}

function ChangeDesk(ActivationInfo info)
{
	if (OptionalDesk == None)
		return;

	OptionalDesk.CopyMaterialsToSkins();
	OptionalDesk.Skins[info.MaterialIndex] = info.Material;
}

simulated function InternalSetup()
{
	if (!IsInState('Active'))
		GotoState('Active');
}

state() Disabled
{
	function BeginState()
	{	
		bIsUsable = false;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
		MakeTransition(DisabledState);
		ChangeDesk(DisabledState);
	}
}

auto state() Standby
{	
	function BeginState()
	{
		bIsUsable = true;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.GotoState('Enabled');
			SquadMarker.bScriptInitialized = true;
		}
		MakeTransition(StandbyState);
		ChangeDesk(StandbyState);
	}
}

state Active
{
	function BeginState()
	{
		bIsUsable = true;
		MakeTransition(ActiveState);
		ChangeDesk(ActiveState);
	}

	function Tick(float deltaTime)
	{
		if (User != None)
			BeginActivating(ActiveState, deltaTime);
	}
	function NextState()
	{
		GotoState('Hacked');
	}
}

state Hacked
{
	function BeginState()
	{	
		bIsUsable = false;
		//If we're hooked up to a squad marker, then make
		//the SquadMarker obsolete if it isn't active
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
			SquadMarker.MakeObsolete();
		MakeTransition(HackedState);
		ChangeDesk(HackedState);
	}
}

event bool ContainsPartialEvent(string StartOfEventName)
{
	local bool ActivateInfoContains;
	ActivateInfoContains = ActivateInfoContainsPartialEvent(DisabledState, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(StandbyState, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(HackedState, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(ActiveState, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	return Super.ContainsPartialEvent(StartOfEventName);
}


defaultproperties
{
     HackedState=(TransitionSound=Sound'GEN_Sound.Interface.int_GEN_HackedConsole_01')
     ActiveState=(Duration=10)
     bLockPlayerLocation=True
     bSavesProgress=True
     bNonPlayerUsesTimer=True
     HUDIconY=5
     DurationRemaining=10
     PlayerOffset=85
     ActivatePromptText="PRESS AND HOLD @ TO SLICE CONSOLE"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     DrawType=DT_StaticMesh
     bBlockActors=True
     bBlockPlayers=True
     bBlockKarma=True
     NavPtLocs(0)=(Offset=(X=96),Yaw=32768)
     HUDMarkerIconZOffset=-52
}

