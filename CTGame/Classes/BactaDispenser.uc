// ====================================================================
//  Class:  CTGame.BactaDispenser
//
// Created by 2004, March 26, John Hancock
// ====================================================================

class BactaDispenser extends ActivateItem
	native;

var()	ActivationInfo		BactaStandby;
var()	ActivationInfo		BactaActive;
var()	ActivationInfo		BactaRecharging;
var()	ActivationInfo		BactaDisabled;
//var()	int					BactaCharges;
var		BactaDispenser		NextDispenser;
//var()	float				BactaRemaining;
var()	static float		BactaRate;
var()	static class<Emitter>	DispenserEffectClass;	// The effect to spawn on entering the state
var()	static class<Emitter>	PawnEffectClass;
var		Emitter					PawnEffect;


// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

simulated event InternalSetup()
{
	if (!IsInState('Active'))
		GotoState('Active');
}

function PreBeginPlay()
{
	local vector loc;

	super.PreBeginPlay();

	loc = NavPts[0].Location;//GetMarkerSpotLocation(i);
	loc.Z -= NavPts[0].CollisionHeight;
	NavPts[0].bDontIdleHere = true; //tell people not to idle here

	if (SquadMarker == None)
	{
		SquadMarker = Spawn(class'MarkerGetBacta',,, loc, NavPts[0].Rotation);
		SquadMarker.Anchor = self;
		SquadMarker.bValidSetup = SquadMarker.IsSetUpProperly();
	}

	NextDispenser = BactaDispenser(Level.BactaDispenserList);
	Level.BactaDispenserList = self;
}

event bool ContainsPartialEvent(string StartOfEventName)
{
	local bool ActivateInfoContains;
	ActivateInfoContains = ActivateInfoContainsPartialEvent(BactaStandby, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(BactaActive, StartOfEventName);
	if (ActivateInfoContains)
		return true;


	ActivateInfoContains = ActivateInfoContainsPartialEvent(BactaRecharging, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	/*
	ActivateInfoContains = ActivateInfoContainsPartialEvent(BactaEmpty, StartOfEventName);
	if (ActivateInfoContains)
		return true;
	*/

	return Super.ContainsPartialEvent(StartOfEventName);
}

function BeginActivating(ActivationInfo State, float deltaTime)
{
	local Pawn Player;
	local float DeltaHealth;

	if (User == None || User.Pawn == None || User.Pawn.IsDeadOrIncapacitated())
	{
		AbortSetup();
		return;
	}
	//if we're not supposed to use the auto-timer, just return
	if (User != None && !User.bIsPlayer && !bNonPlayerUsesTimer)
		return;

	if (User != None && (!User.bIsPlayer || 
		(PlayerController(User).bUse && PlayerController(User).TargetType == TGT_Panel)))
	{	
		Player = User.Pawn;

		if ( Player.Health <= 0 )
		{
			User = None;
			bActivating = False;
			DurationRemaining = 0.0;
			NextState();
		}
		else if (Player.Health >= Player.MaxHealth /*|| BactaCharges <= 0 || BactaRemaining <= 0*/)
		{
			Player.Health = Player.MaxHealth;
			User = None;
			bActivating = false;
			DurationRemaining = 0.0;
			NextState();
		}
		else
		{
			DeltaHealth = deltaTime * Player.MaxHealth / BactaRate;
			//if (DeltaHealth > BactaRemaining)
			//	DeltaHealth = BactaRemaining;
			if (DeltaHealth > (Player.MaxHealth - Player.Health))
				DeltaHealth = (Player.MaxHealth - Player.Health);
			//BactaRemaining -= DeltaHealth;
			Player.Health += DeltaHealth;
			Player.SetHealthLevel();
			FractionComplete = Player.Health / Player.MaxHealth;
			//DurationRemaining = (Player.MaxHealth - Player.Health)/BactaRate;
		}
	}
	else
	{
		AbortSetup();
	}
}

event AbortSetup()
{
	local bool FullHealth;
	FullHealth = false;
	//if (!bActivated)
	//	SetStaticMesh(default.StaticMesh);
	bActivating = false;
	bPlayerActivated = false;
	if (User != None && User.Pawn.Health == User.Pawn.MaxHealth)
		FullHealth = true;

	User = None;
	if (FullHealth)
	{
		//Log("Bacta Dispenser abort setup - recharging");
		GotoState('Recharging');
	}
	else
	{
		//Log("Bacta dispenser abort setup - standby");
		GotoState('Auto'); //go back to standby state
	}
}

auto state() Standby
{	
	function BeginState()
	{
		User = None;

		bIsUsable = true;
		MakeTransition(BactaStandby);
		//Marker states are totally driven by 
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.GotoState('Enabled');
			SquadMarker.bScriptInitialized = true;
		}
	}
}

event DepleteBacta()
{
	GotoState('Recharging');
	/*
	--BactaCharges;
	if (BactaCharges <= 0)
		GotoState('Disabled');
	else
		GotoState('Recharging');
		*/
}

state() Active
{
	function BeginState()
	{	
		//set the state of objects that depend on us (Doors, Markers)
		//setting bScriptInitialized prevents others' autostates from
		//overriding our settings
		PlayAnim('Load');
		bActivated=false;

		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
		MakeTransition(BactaActive);
	}
	function Tick(float deltaTime)
	{
		if (User != None && bActivated)
			BeginActivating(BactaActive, deltaTime);
	}
	function EndState()
	{
		DestroyAttachment('Eye');
		PlayAnim('Holster');
		if (PawnEffect != None)
			PawnEffect.FadeOut();
		PawnEffect = None;

	}
	function NextState()
	{
		//if the player was using it, heal him
		local Pawn Pawn;
		if (User != None)
		{
			Pawn = User.Pawn;
			Pawn.Health = Pawn.MaxHealth;
		}
		DepleteBacta();
	}
Begin:
	FinishAnim();
	bActivated=true;
	SpawnAttachment('Eye', DispenserEffectClass);
	PawnEffect = User.Pawn.SpawnSkeletalEffect(PawnEffectClass);
}

state() Recharging
{
	function BeginState()
	{
		bIsUsable = false;

		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}

		MakeTransition(BactaRecharging);
		SetTimer(BactaRecharging.Duration, false);
	}
	function Timer()
	{
		GotoState('Standby');
	}
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

		MakeTransition(BactaDisabled);
	}
}




cpptext
{
	virtual ANavigationPoint* GetAnchor() { return NavPts[0]; }

}

defaultproperties
{
     BactaRecharging=(Duration=1)
     BactaRate=4
     ActivateItemType=AIT_BactaDispenser
     bLockPlayerLocation=True
     bNonPlayerUsesTimer=True
     HUDIconX=1
     HUDIconY=7
     PlayerRange=256
     ActivatePromptText="PRESS AND HOLD @ TO HEAL"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     DrawType=DT_StaticMesh
     bBlockActors=True
     bBlockPlayers=True
     bBlockKarma=True
     NavPtLocs(0)=(Offset=(X=135),Yaw=32768)
     HUDMarkerIconZOffset=128
     CollisionRadius=40
     CollisionHeight=200
}

