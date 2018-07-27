// ====================================================================
//  Class:  CTGame.DemolitionCharge
//
// Created by 2004, February 29, John Hancock
// Begun as duplicate of SetTrap. Kept SetTrap as
// separate in case we wanted to go back or use both
// ====================================================================

class ExplosiveCharge extends ActivateItem
	native
	abstract;

var()	ActivationInfo		ChargeUnplaced;
var()	ActivationInfo		ChargePlaced;
var()	ActivationInfo		ChargeArmed;
var()	ActivationInfo		ChargeExplode;
var()	Prop				PropToDestroy;
var		static EStimulusType	ExplosionStimulus;
var		static bool			bExplodeOnTimer;	//In general, we don't want this changed

simulated function InternalSetup()
{
	if (!IsInState('Placed'))
		GotoState('Placed');
}

event bool ContainsPartialEvent(string StartOfEventName)
{
	local bool ActivateInfoContains;
	ActivateInfoContains = ActivateInfoContainsPartialEvent(ChargeUnplaced, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(ChargePlaced, StartOfEventName);
	if (ActivateInfoContains)
		return true;


	ActivateInfoContains = ActivateInfoContainsPartialEvent(ChargeExplode, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	ActivateInfoContains = ActivateInfoContainsPartialEvent(ChargeArmed, StartOfEventName);
	if (ActivateInfoContains)
		return true;

	return Super.ContainsPartialEvent(StartOfEventName);
}

auto state() Unplaced
{	
	function BeginState()
	{
		bIsUsable = true;
		MakeTransition(ChargeUnplaced);
		//Marker states are totally driven by 
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.GotoState('Enabled');
			SquadMarker.bScriptInitialized = true;
		}
	}
	function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
	{	
		//we're not really here, so any damage our collision model takes should
		//get passed on to the prop
		if (PropToDestroy != None)
			PropToDestroy.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType, BoneName);

		return Damage;
	}
}

state() Placed
{
	function BeginState()
	{	
		MakeTransition(ChargePlaced);
	}
	function Tick(float deltaTime)
	{
		if (User != None)
			BeginActivating(ChargePlaced, deltaTime);
	}
	function NextState()
	{
		GotoState('Armed');
	}
}

function float TakeDamage( float Damage, Pawn instigatedBy, Vector hitlocation, Vector momentum, class<DamageType> damageType, optional Name BoneName )
{	
	if (bActivated && bTriggeredByDamage)
	{
		GotoState('Explode');		//Trigger(self, instigatedBy);	
		return Damage;
	}
	else
		return 0;
}

state() Armed
{
	function BeginState()
	{
		DeductAmmo();
		bActivated = true;
		bIsUsable = false;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}

		MakeTransition(ChargeArmed);
		if (ChargeArmed.Duration > 0 && bExplodeOnTimer)
			SetTimer(ChargeArmed.Duration, false);
	}
	function Timer()
	{
		GotoState('Explode');
	}
}

state() Explode
{
	function BeginState()
	{
		bIsUsable = false;
		ActivateItemType=AIT_Demolition;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
			SquadMarker.MakeObsolete();
		MakeTransition(ChargeExplode);
		if (PropToDestroy != None)
			PropToDestroy.GotoState('PropDestroyed');
		PostStimulusToWorld(ExplosionStimulus);
		bHidden=true;
		Destroy();
		if (NavPts.Length > 0)
			NavPts[0].SetBlocked(false);
	}
}

state() Disabled
{
	function BeginState()
	{
		bHidden = true;
		bIsUsable = false;
		if (SquadMarker != None)
		{
			SquadMarker.MakeObsolete();
			SquadMarker.bScriptInitialized = true;
		}
	}
	function EndState()
	{
		bHidden = false;
	}
}


defaultproperties
{
     ExplosionStimulus=ST_ExplosionLarge
     bTriggeredByDamage=True
     bLockPlayerLocation=True
     ActivatePromptText="PRESS AND HOLD @ TO PLACE EXPLOSIVE CHARGE"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     DrawType=DT_StaticMesh
     NavPtLocs(0)=(Offset=(X=85),Yaw=32768)
}

