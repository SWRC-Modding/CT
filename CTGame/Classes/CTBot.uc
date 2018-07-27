class CTBot extends ScriptedController
	native;

enum EStatePhase {
	SP_Inactive,	//currently inactive, but enabled
	//ACTIVE phases
	SP_Activate,	//in the process of activating
	SP_Update,		//the normal phase of the currently "Active" state
	SP_Shutdown		//in the process of shutting down
};

enum EBotGestureSuite {
	BGS_None,
	BGS_StateControlled,
	BGS_Auto,
	BGS_Incapacitated,
};

const WoundedAccuracyFactor = 1.0f;

var()	static int			MaxDodgeAngle;			//The angle withing which we will try to dodge (in Rotator Units)
var()	static float		MinDodgeTime;			//The minimum number of seconds between dodges
var()	static float		MinGrenadeTime;			//The minimum number of seconds between grenade throws
var()	static float		CoverBoredomTime;		//The frequency with with an AI will attempt to find new cover
var()	static float		GrenadeProbability;		//The change we'll throw a grenade this round
var()	static float		MinBurstInterval;		//The smallest amount of time allowed between shots (UNUSED)

var		float				NextBurstTime;			//Next earliest possible time to fire a burst (UNUSED)
var		float				NextCoverSearchTime;	//Next earliest possible cover search time
var		float				NextGrenadeTime;		//Next earliest possible grenade throw time
var		float				NextDodgeTime;			//A tracker for when we can next dodge

var		NavigationPoint		ReservedNode;			//Reserved node
var		LocalSituation		Situation;				//this might be something derived from LocalSituation
var		float				NextDecisionTime;		//Next time to make a state decision
var		Array<StateObject>	States;					//editinlinenew?
var		GoalQueue			MainGoals;				//the primary goal queue that most states will use
var		GoalQueue			ScriptedGoals;			//the goal queue for scripted goals
var		GoalQueue			SquadGoals;				//the goal queue for squad goals
var		GoalQueue			ReflexGoals;			//the goal queue for quick, non interruptable reflexes
var		GoalQueue			GestureGoals;			//goals that can be run in parallel to the active state (should NOT contain movement goals)
var		StateObject			CurrentState;
var		StateObject			NextState;
var		float				BestUtility;			//the winning state's utility
var		Array<Stimulus>		StimuliReceived;		//all the stimuli that have been received by this AI
var()	float				EnemyMemoryTime;		//how long it takes Bot to forget about current Enemy about not seeing it
var		float				LastTimeEnemySeen;		//the last time this enemy was seen
var		int					LastManeuver;			//the last maneuver that we executed
var		int					ShotCounter;			//our current shot counter
var()	static const int	BurstCount;				//the number of shots we try to fire when we shoot
var		Array<Stimulus>		UnprocessedStimuli;
//var	Array<Stimulus>		ProcessedStimuli;		//potentially, an array to keep stimuli for the long-term
var		Stimulus			StrongestStimulus;
var		GOAL_WaitForEvent	WaitForEventGoal;
var		GOAL_Shoot			CurrentShootGoal;
var		CoverPoint			SuggestedCombatPosition;
var		Actor				SuggestedEnemy;			//an enemy to attack
var		Actor				TetherTarget;			//an actor that we might be tethered to
var		Pawn				TetherPawn;				//TetherPawn = Cast<Pawn>(TetherTarget)
var		Actor				PreventTether;			//an actor that Investigate, Attack states use as a limiter, used with PreventRange
var		float				PreventRange;
var		float				NextGunSafetyCheck;
var		float				TimeOfLastBattleStimulus;
var		float				TimeOfLastStimulus;
var		Vector				OffsetDirection;
var		Pawn				AllyInWay;				// this is for debug draw purposes only
var		float				AllyInWayRadius;		// this is for debug draw purposes only
var		float				ClosestAllyAngle;		// this is the ally closest to our angle of fire
var		Actor				GrenadeTarget;			// a grenade target location
var		EStatePhase			Phase;
var		EBotGestureSuite	GestureSuite;
var		BYTE				BumpCount;
var		BYTE				FriendlyFireCount;
var		float				LastFriendlyFireTime;
var		float				NextIdleGestureTime;
var		float				NextLookDirectorTime;
var		class<StateObject>  LastStateClass;
var		float				StateActivationTime;	//at what time the current state was activated
var		float				LastStateLength;		//how long the previous state was active
var		float				DecisionInterval;
var()	float				EmoteProbability;
var		float				DodgeErrorTime;			//amount of time we fire at the old position of our Enemy when he dodges
var		float				LastBumpTime;
var()	float				PostWorldCheckFireDuration;	//amount of time to keep firing after failed world check
var		float				LastClearShotTime;

//var		class<GoalObject>	ActiveGoalClass;				// For Debugging purposes only, CUTME later

//var		float				SpeedServoGain;

var		const EStimulusResponse HostileStimuliResponses[SG_MAX];
var		const EStimulusResponse BenignStimuliResponses[SG_MAX];

// Booleans
var		bool				StateWasForced;
var		bool				ExecutingCriticalGoal;
var		bool				bPlayEmoteHear;
var		bool				ShouldReloadSoon;
var		bool				CanReloadNow;
var		bool				CanDodgeNow;				//Indicates that dodging is a appropriate. Should be set to false during Maneuvers that should not be interrupted.
var		bool				bLeapFrog;					//If we're leap-frogging
var		bool				bBotCombatEnabled;
var		bool				bAutoSelectRunOrWalk;
var		bool				bShouldCrouch;
var		bool				bWantsToFire;
var		bool				bLastSafeToShoot;
var		bool				bAttackFromCrouch;
var		bool				bLookDirectorEnabled;
var		bool				bBotCombatAllowsMelee;
var		bool				bUseAdditionalTargets;
var		bool				bWasProcessingReflexes;
var		bool				bCanDecelerate;
var		bool				bPreventTetherOneWay;
var		bool				bCheckCorpseFire;
var		bool				bAutoAntiArmor;
var		bool				bAutoSnipe;
var		bool				bAutoGrenade;
var		bool				bIgnoreFF;
var		bool				bAbortShootToDefend;
var		bool				bCheckFavoriteWeapon;
var		bool				bCanGesture;

//var		bool				bKeepGestureGoals;

var()	class<StateObject>	RequiredStates[MAXSTATEOBJECTS]; //the names of the StateObjects needed by this AI
var()	float				HearingMultiplier;

const AUTORUNDISTANCE = 400.0;

//native final function StateObject AddState(Class StateObjectClass);
native event float MaxAimError();
native final function AssignToSquad(CTBot Bot);
native final function StateObject GetState(Class StateObjectClass, optional bool CreateIfNotFound /* = true */);
native final function StateScriptWrapper GetWrapperState(Name WrappedState, optional bool CreateIfNotFound /* = true */);
native final function ForceState(class<StateObject> StateToForce, bool bCreateIfNotFound, optional float TimeLength);
native final function UnForceState();
native event EnableState(class<StateObject> StateToDisable, bool bEnable, optional bool CreateIfNotFound);
native function FriendlyFireCallback(Pawn Ally, class<DamageType> DamageType, int ShotCount);
native event SuggestEnemy(Actor Enemy);
native function AddScriptedGotoGoal(Actor SomeActor, optional float Tolerance, optional bool OrientToPoint);
native function AddScriptedShootGoal(Actor SomeActor, float Duration, optional int NumShots);
//native final function ReleaseFromForcedState();
native event SetScriptingPriority(float Priority);
native function BeginPawnRevive(Pawn P);

event color GetDebugColor()
{
	local color DebugColor;

	if( CurrentState != None )
		DebugColor = CurrentState.GetDebugColor();
	else
		return DebugColor;
}

function Possess(Pawn aPawn)
{
	local int i;

	super.Possess(aPawn);

	switch( aPawn.StartState )
	{
	case SS_Wander:
		GetState(class'StateWander',true);
		break;
	case SS_PerchedCeiling:
	case SS_Perched:
		ForceState(class'StatePerched',true);
		break;
	case SS_Shutdown:		
		ForceState(class'StateShutdown',true);
		break;
	case SS_Asleep:		
		ForceState(class'StateAsleep',true);
		break;
	case SS_StationaryAttack:		
		for( i = 0; i < States.length; i++ )
		{
			if( States[i].IsA('StateAbstractAttack') )			
				States[i].Enabled = false;							
		}

		GetState(class'StateStationaryAttack',true);
		break;
	}

	if( Pawn.PatrolRoute.length > 0 )
		GetState(class'StatePatrol', true);
}

//Go into our normal BotAI
auto state BotAI
{
Begin:
	DebugAILog("Resuming Normal CTBot AI "$Pawn);
}

//When a character dies (or is incapacitated), the killer is notified by this function
function Killed(Pawn Victim, class<DamageType> damageType)
{
	local PawnAudioTable.EPawnAudioEvent Cue;
	local float interval;
	interval = 60.0f;

	Victim.PostStimulusToIndividual(ST_KilledSomeone, self);
	if (Pawn.Squad != None && Victim == Pawn.Squad.CommandedEnemy)
	{
		Cue = PAE_EngageTargetComplete;
		interval = 5.0;
	}
	else if (Situation.TotalEnemies <= 1)
	{
		Cue = PAE_KilledCalm;
	}
	else
	{
		Cue = Victim.KilledEnemyCue;
	}
	if (Pawn.Squad != None)
		Pawn.Squad.AddVoiceLine(Pawn, Cue, Victim, interval, 2.0f); //timeout of 2 sec
	else
		Pawn.PlayOwnedCue(Cue, 60.0f);
}

//////////////////////////////////////////////
// Temp Stimulus Generators
//////////////////////////////////////////////

function NotifyTakeHit(pawn InstigatedBy, vector HitLocation, int Damage, class<DamageType> damageType, vector Momentum)
{
	local PawnAudioTable.EPawnAudioEvent Cue;

	if( InstigatedBy != None )
	{
		InstigatedBy.PostStimulusToIndividual(ST_Damaged, self);
		if (InstigatedBy != Pawn && InstigatedBy.IsAlly(Pawn))
		{
			//Log("Friendly fire from :"$InstigatedBy$" at: "$Pawn$" range: "$VSize(Pawn.Location - InstigatedBy.Location)$" Enemy Dist: "$VSize(InstigatedBy.Controller.Target.Location - InstigatedBy.Location));
			//Only say friendly fire lines when hit by player
			if (InstigatedBy.IsPlayerPawn() && damageType.default.bCountFriendlyFire)
			{
				if (Level.TimeSeconds - LastFriendlyFireTime < 2.0f)
					++FriendlyFireCount;
				else
					FriendlyFireCount = 1; //reset counter to 1 it's been a while
				LastFriendlyFireTime = Level.TimeSeconds;
				//if we've been shot multiple times within the last few seconds, complain
				if (FriendlyFireCount > 4)
				{
					Cue = damageType.default.FriendlyFireEvent;
					if (Cue == PAE_FriendlyFireHealth && Pawn.Health / Pawn.MaxHealth < 0.5f)
						Cue = PAE_FriendlyFireInjured;
					if (Cue != PAE_Invalid)
						Pawn.PlayOwnedCue(Cue, 8.0f);
				}
			}
			if (InstigatedBy.Controller != None)
				InstigatedBy.Controller.FriendlyFireCallback(Pawn, damageType, FriendlyFireCount);
		}
	}
	super.NotifyTakeHit(InstigatedBy, HitLocation, Damage, damageType, Momentum);
}


//////////////////////////////////////////////
//  Weapon Aim modification
//////////////////////////////////////////////

native function rotator AdjustAim(Ammunition FiredAmmunition, vector projStart, float AimError, optional out int ShouldFire);

//////////////////////////////////////////////
// Action Script Support
//////////////////////////////////////////////

function TakeControlOf(Pawn aPawn)
{
	if ( Pawn != aPawn )
	{
		aPawn.PossessedBy(self);
		Pawn = aPawn;
	}
	//GotoState('Scripting');
}

function SetNewScript(ScriptedSequence NewScript)
{
	SetScriptingPriority(1.0); //Must come BEFORE super.SetNewScript
	super.SetNewScript(NewScript);
}

/*
function EventTrigger(Name EventName)
{
	if (WaitForEventGoal != None && WaitForEventGoal.Event == EventName)
		WaitForEventGoal.Trigger();
}
*/

//Functions to make ScriptedSequence wait events work even when
//we're not in Scripting state
function Trigger( Actor Other, Pawn EventInstigator )
{
	if (WaitForEventGoal != None)
		WaitForEventGoal.Trigger();
	
	// We don't want to send the bot a trigger message if it's on an action script
	if ( CurrentAction != None )
	{
		if( CurrentAction.CompleteWhenTriggered() )
			CompleteAction();
	}
	else
	{
		if( EventInstigator != None )
			EventInstigator.PostStimulusToIndividual(ST_Triggered,self);
		else
			Other.PostStimulusToIndividual(ST_Triggered,self);
	}

	DebugAILog("CTBot::Trigger "$Tag$" for Bot Pawn "$Pawn);	
}

function CompleteAction()
{
	DebugAILog("CTBot::CompleteAction #"$ActionNum$" for Pawn "$Pawn);
	if (ScriptPriorityOnEvent >= 0)
		SetScriptingPriority(ScriptPriorityOnEvent);
	if (bUnforceStateOnEvent)
		UnForceState();
	bUnforceStateOnEvent=false;
	ScriptPriorityOnEvent = -1.0;
	ActionNum++;
}

state Scripting
{
	function LeaveScripting()
	{
		DebugAILog("CTBot::Scripting::LeaveScripting "$Pawn);
		SetScriptingPriority(0.0);		
		GotoState('BotAI'); //this is necessary to prevent some infinite loops in the script code
	}
}


//////////////////////////////////////////////
// Misc
//////////////////////////////////////////////

function ThrowScav()
{
	Super.ThrowScav();

	ReflexGoals.Clear();
	Pawn.ChangeAnimation();
}

//////////////////////////////////////////////
// Properties
//////////////////////////////////////////////


defaultproperties
{
     MaxDodgeAngle=5000
     MinDodgeTime=2
     MinGrenadeTime=10
     CoverBoredomTime=15
     GrenadeProbability=0.3
     EnemyMemoryTime=15
     BurstCount=1
     DecisionInterval=0.5
     EmoteProbability=0.5
     DodgeErrorTime=1
     HostileStimuliResponses(0)=SR_RunToward
     HostileStimuliResponses(1)=SR_RunAway
     HostileStimuliResponses(5)=SR_DiveAway
     HostileStimuliResponses(10)=SR_RunToward
     BenignStimuliResponses(0)=SR_LookAt
     BenignStimuliResponses(1)=SR_LookAt
     BenignStimuliResponses(3)=SR_LookAt
     BenignStimuliResponses(4)=SR_LookAt
     BenignStimuliResponses(5)=SR_RunAway
     BenignStimuliResponses(6)=SR_RunToward
     bPlayEmoteHear=True
     CanReloadNow=True
     bCanDecelerate=True
     PlayerReplicationInfoClass=Class'CTGame.CTPRI'
}

