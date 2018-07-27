// ====================================================================
//  Class:  CTGame.ActivateTrap
//
//  Set Trap:
//	Upon activation, it goes into the DetectedTrap state.
//	Upon completion of setting the trap, it goes into the SetTrap state.
//	If a pawn is nearby to detonate the trap, it goes into the ExplodeTrap state.
//  
//  Disarm Trap:
//  The initial default state is the SetTrap (the trap is set) state.
//	Upon activation, it goes into the DetectedTrap state.
//	Upon deactivation, it goes into the DisarmTrap state.
//
// Created by 2003, December 8, Jenny Huang
// ====================================================================

class ActivateTrap extends ActivateItem
	native;

var()	ActivationInfo		TrapDetected;
var()	ActivationInfo		TrapDisarmed;
var()	ActivationInfo		TrapSet;
var()	ActivationInfo		TrapExplode;
var()	int					TeamIndex;
var()	float				TriggerRadius;
var		float				NextCheckTime;
var		static StaticMesh	DetonationPreviewMesh;
var		Actor				DetonationPreviewActor;
var()	Name				EventTargeted;  
var()	static float		AlertTimeLimit;
var		float				LastNoDetectTime;
var(Sound)	static Sound		AlertSound;
var		const bool			bSetTrapReferences;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

native event IncrementRadiusTrapReferences();
native event DecrementRadiusTrapReferences();

function PostBeginPlay()
{
	local float scale;
	//local vector Start, End, HitNormal, HitLocation;
	//local Actor HitActor;
	//local Rotator rot;

	super.PostBeginPlay();

	/*
	HitNormal.X=0;
	HitNormal.Y=0;
	HitNormal.Z=1;
	*/
	if (DetonationPreviewMesh != None && TriggerRadius > 1)
	{
		//Log("Created Bomb DetonationPreviewActor");
		/*
		Start = Location;
		Start.Z += 50;
		End = Location;
		End.Z -= 50;
		HitActor = Trace(HitLocation, HitNormal, End, Start, false);
		*/

		DetonationPreviewActor = Spawn(class'IntangibleActor',,,Location,Rotation);//Rotator(HitNormal));
		DetonationPreviewActor.SetStaticMesh(DetonationPreviewMesh);
		scale = TriggerRadius/512.0;
		DetonationPreviewActor.SetDrawScale(scale);
		DetonationPreviewActor.bHidden = true;

	}
}

event Destroyed()
{
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.Destroy();
	super.Destroyed();
}

simulated function TacticalModeBegin( PlayerController Player )
{
	if (bActivated && DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = false;
}

simulated function TacticalModeEnd( PlayerController Player )
{
	if (Player.Target == self)
		return;
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = true;
}

simulated function NotifyTargeted( PlayerController Player )
{
	local Pawn Ally;
	//Log("Bomb NotifyTargeted");
	if (bActivated && DetonationPreviewActor != None)
	{
		DetonationPreviewActor.bHidden = false;
		if (Player.Pawn.Squad != None)
		{
			Ally = Player.Pawn.Squad.GetClosestMemberWithViewOf(Location, 1536.0f, Player.Pawn);
			if (Ally != None)
				Player.Pawn.Squad.AddVoiceLine(Ally, PAE_MineSpotted, None, 90.0f, 1.0f);
		}
	}
	if (EventTargeted != '')
	{
		TriggerEvent(EventTargeted, self, Player.Pawn);
		EventTargeted = '';
	}
}

simulated function NotifyUnTargeted( PlayerController Player )
{
	//Log("Bomb NotifyUnTargeted");
	if (DetonationPreviewActor != None)
		DetonationPreviewActor.bHidden = true;
}

/*
event Activate(Pawn Instigator)
{
	TeamIndex = Instigator.GetTeamIndex();
	if (TrapDetected.TransitionSound != None)
		PlaySound(TrapDetected.TransitionSound);
}
*/

simulated function UseSetupMesh()
{
	if (TrapDetected.Mesh != None)
		SetStaticMesh(TrapDetected.Mesh);
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

function Tick(float deltaTime)
{
	if (User != None)
		BeginActivating(TrapDetected, deltaTime);
}

function Pawn DetectNearbyPawn()
{
	local Controller C;
	local TeamInfo BombTeam;

	BombTeam = Level.GRI.GetTeam(TeamIndex);
	for (C=Level.ControllerList; C!= None; C=C.NextController)
	{
		if (C.Pawn != None && !C.Pawn.bIsCrouched && !C.Pawn.IsDeadOrIncapacitated() && C.Pawn.Physics != PHYS_Flying
			//&& BombTeam.IsEnemy(C.Pawn.GetTeamIndex())
			&& VDistSq(C.Pawn.Location, Location) < TriggerRadius * TriggerRadius)
		{
			return C.Pawn;
			//Log("Pawn velocity = "$VSize(C.Pawn.Velocity));
			//if (VSizeSq(C.Pawn.Velocity) > 10000)
				
		}
	}
	return None;
}

state DetectedTrap
{
	function BeginState()
	{
		bActivating = true;
		MakeTransition(TrapDetected);
	}
}

function PingTrap()
{
	local Pawn Pawn;
	NextCheckTime = Level.TimeSeconds + 0.25; //check every 1/4 second
	Pawn = DetectNearbyPawn();
	if (Pawn != None)
	{
		NextCheckTime = Level.TimeSeconds + 0.15; //check more frequently if someone is inside radius
		PostStimulusToWorld(ST_MineChirp);
		PlaySound(AlertSound);
		//Log("Alert: Detected Pawn "$Level.TimeSeconds);
		if (Level.TimeSeconds >= LastNoDetectTime + AlertTimeLimit)
		{
			Instigator = Pawn;
			GotoState('Explode');
		}
	}
	else
		LastNoDetectTime = Level.TimeSeconds;

}

state SetTrap
{
	function BeginState()
	{
		bTriggeredByDamage = true;
		bActivated = true;
		bIsUsable = false;
		MakeTransition(TrapSet);
	}

	function Tick(float deltaTime)
	{
		Global.Tick(deltaTime);

		if (TriggerRadius > 1 && Level.TimeSeconds > NextCheckTime)
		{
			PingTrap();
		}
	}
}

state DisarmTrap
{
	function BeginState()
	{
		bActivated = false;
		bIsUsable = false;
		if (SquadMarker != None && !SquadMarker.bMarkerActive)
			SquadMarker.MakeObsolete();
		DecrementRadiusTrapReferences();
		if (NavPts.Length > 0)
		{
			NavPts[0].SetBlocked(false);
		}
		MakeTransition(TrapDisarmed);
		Destroy();
	}
}

state() Explode
{
	function BeginState()
	{
		bIsUsable = false;
		if (SquadMarker != None)
			SquadMarker.MakeObsolete();
		if (NavPts.Length > 0)
		{
			NavPts[0].SetBlocked(false);
		}

		MakeTransition(TrapExplode);
		PostStimulusToWorld(ST_ExplosionDefault);
		bHidden=true;

		DecrementRadiusTrapReferences();
		bBlockZeroExtentTraces=false;
		bBlockNonZeroExtentTraces=false;
		bProjTarget=false;
		Destroy();

	}
}



cpptext
{
	virtual INT AddMyMarker(AActor *S);
	virtual void PostNavListBuild();

}

defaultproperties
{
     TeamIndex=5
     TriggerRadius=512
     AlertTimeLimit=0.75
     bSavesProgress=True
     bNonPlayerUsesTimer=True
     DurationRemaining=5
     ActivateDotProdMax=1
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     DrawType=DT_StaticMesh
     bDrawHUDMarkerIcon=False
     NavPtLocs(0)=(Offset=(X=-80))
}

