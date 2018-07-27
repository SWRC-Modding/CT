// ====================================================================
//  Class:  CTGame.DisarmTrap
//
// Created by 2003, December 8, Jenny Huang
// ====================================================================

class DisarmTrap extends ActivateTrap
	;

simulated function InternalSetup()
{
	if (!IsInState('DetectedTrap'))
		GotoState('DetectedTrap');
}

simulated function AbortSetup()
{
	super.AbortSetup();
	if (IsInState('DetectedTrap'))
		GotoState('SetTrap');
}

state DetectedTrap
{
	function Tick(float deltaTime)
	{
		Global.Tick(deltaTime);

		if (TriggerRadius > 1 && Level.TimeSeconds > NextCheckTime)
		{
			PingTrap();
		}
	}

	function NextState()
	{
		GotoState('DisarmTrap');
	}
}

auto state() SetTrap
{
	function BeginState()
	{
		bActivated = true;
		bIsUsable = true;
		bTriggeredByDamage = false;
		MakeTransition(TrapSet);
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.GotoState('Enabled');
			SquadMarker.bScriptInitialized = true;
		}
		if (NavPts.Length > 0)
		{
			NavPts[0].SetBlocked(true);
		}
		IncrementRadiusTrapReferences();
	}
}

state () Disabled
{
	ignores NotifyTargeted, TacticalModeBegin;

	function BeginState()
	{
		bActivated = false;
		bIsUsable = false;
		bDrawHUDMarkerIcon=false;
		if (SquadMarker != None)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
		if (NavPts.Length > 0)
		{
			NavPts[0].SetBlocked(false);
		}
		DecrementRadiusTrapReferences();
	}
}



defaultproperties
{
     TrapDetected=(Mesh=StaticMesh'MarkerIcons.SetTrap.Trap',Duration=5)
     TrapDisarmed=(TransitionSound=Sound'GEN_Sound.Interface.consoleTypeConfirm_01',Duration=5)
     TrapSet=(Mesh=StaticMesh'MarkerIcons.SetTrap.Trap',Duration=5)
     TrapExplode=(TransitionEffect=Class'CTEffects.Explosion_Trap_Dirt',MyDamageType=Class'CTGame.CTDamageExplosionTrap',Damage=800,DamageRadius=640)
     DetonationPreviewMesh=StaticMesh'MarkerIcons.SetTrap.DtatonRadius'
     AlertSound=Sound'GEN_Sound.Interface.int_GEN_proxMineChirp_01'
     ActivateItemType=AIT_DisarmTrap
     bActivated=True
     ActivatePromptText="PRESS AND HOLD @ TO DISARM"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     StaticMesh=StaticMesh'MarkerIcons.SetTrap.Trap'
     NavPtLocs(0)=(Offset=(X=-80))
}

