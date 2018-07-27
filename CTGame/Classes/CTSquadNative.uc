//=============================================================================
// Clone Troooper Player Controller
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class CTSquadNative extends Squad
	abstract
	native;

//Currently Only Squad Goals get added here. Marker Goals do not
struct SquadAssignment{
	var Pawn Member;
	var Actor Subject;
	var GoalObject Goal;  //refer to the actual goal in case we need to remove it
	var Object Owner;	// the pointer to the "Owner" of this assignement (what created it)
	var bool bParallelGoal;	//if parallel, goal doesn't affect marker timing
	var class<StateObject> StateClass;

};


var int Head;	//the index of the member at the "front" of the squad (In Offense it's the lead member (1), in defense, it's the player (0) )
var Actor RallyActor;
var Actor OptionalRallyActor;
var Array<SquadAssignment> Assignments;

var float NextCantFireTime;
var int CurrentObjective;
var int NextWaitFor;			//this tracks the next wait for event
var int NextGoto;				//this tracks the next goto objective
var int NextRequiredGoto;		//this tracks the next required goto
var	float LastBattleCueTime;

var NavigationPoint	LatestGotoPoint;
var NavigationPoint NextGotoPoint;
var static StaticMesh RallyPointMesh;
var static pconly StaticMesh SearchAndDestroyMesh;
var static pconly StaticMesh OptionalSDMesh;

var class<Emitter> SecureAreaEffect;
var() class<Emitter> EngageEffectClass;

var Actor FormationFocus;	//this is the actor used to determine current movement roles
var float LastMemberKilledTime;	//when the most recent member was killed
var(Sound) Sound SquadDeathSound;
var(Sound) Sound SquadIncapSound;


native function CompleteWaitFor();
native function Update(float DeltaTime);
native event UpdateVoice(float DeltaTime);
native event ReviveOrder(Pawn Incapacitated);
native function MemberKilled(Pawn Victim, Controller Killer, class<DamageType> damageType);
native function SetStance(class<SquadStance> Type, optional bool DirectOrder, optional Actor StanceObjective);
native event CancelAllMarkers(Pawn Instigator);
native event bool EngageLocation(vector Location);
native event EngageTarget(Actor Target);
native event CancelAllOrders(Pawn Instigator);
native event Recall(Pawn Instigator);
native event ToggleCautious();
native event SetCommandedEnemy(Pawn Target);

event Tick(float DeltaTime)
{
	if (Level.TimeSeconds >= NextUpdateTime){
		Update(DeltaTime);
	}
	if (Level.TimeSeconds >= NextVoiceUpdateTime)
	{
		UpdateVoice(DeltaTime);
	}
}

//This is used for trigger WaitForEvent SquadObjectives
function Trigger( Actor Other, Pawn EventInstigator )
{
	Log("CTSquad::Trigger "$HiddenTag);
	if (HiddenTag != 'None')
	{
		HiddenTag = 'None';
		CompleteWaitFor();
	}
}


defaultproperties
{
     PlayerSquad=True
}

