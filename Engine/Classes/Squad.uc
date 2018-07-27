//=============================================================================
// operational AI control for TeamGame
// 
//=============================================================================
class Squad extends ReplicationInfo
	dependsOn(PawnAudioTable)
	native;

var Pawn SquadLeader;
//var TeamPlayerReplicationInfo LeaderPRI;
//var CTSquad NextSquad;	// list of squads on a team

var enum EStanceType
{
	STT_Invalid,
	STT_FormUp,
	STT_Hold,
	STT_EngagePosition,
	STT_EngageTarget,
	STT_SearchAndDestroy,
	STT_EngageCautious,
	STT_LastOrder,
	STT_Default,
} StanceType;

enum ESquadCombatRole //this could really mimic the movemodes, maneuvers, etc.
{
	SCR_None,
	SCR_StackTBD,
	SCR_StackCrouch,
	SCR_StackStand,
	SCR_Other,
};

enum ESquadMovementRole
{
	SMR_None,			//for players -- no orders are passed to squad member
	SMR_TBD,
	SMR_FreeAgent,
	SMR_Point,
	SMR_FollowPoint1,
	SMR_FollowPoint2,
	SMR_FollowPoint3,	//currently we never have one, but just in case...

};

struct MemberInfo
{
	var Pawn				Pawn;
	var ESquadMovementRole	MovementRole;
	var ESquadCombatRole	CombatRole;
	//var Object				Maneuver; //points to the maneuver the squad member is currently engaged in
	//var int					SubTeam; //some markers might assign guys to subteams
	var int					HUDIconX;  // These icons can be found in the texture HUDTextures.Icons.CloneVisorIcons
	var int					HUDIconY;  // where the bottom right icon is 0,0 and the one left of it is 1,0
};

enum ESquadObjective 
{
	SO_Goto,
	SO_WaitForEvent,
	SO_GotoUnlessEvent,
	SO_InitiateMarker,
};

//For sequencing voice lines...
struct SquadVoiceLine
{
	var	Pawn							Member;
	var PawnAudioTable.EPawnAudioEvent	Cue;
	var float							TriggerInterval;
	var Sound							Sound;
	var Object							Subject;
	var float							TimeMustPlayBy;
};

//could use Scripted Actions here...
struct SquadObjective{

	var() ESquadObjective ObjectiveType;
	var() Name Tag;
	var() Actor Actor; //can be used instead of Tag to refer to NavPoint
	var() Sound CompleteSound; //played when objective is completed
	var() String CompleteText;	//displays when objective completed
	var() float Radius; //how close the "Head" member must get to the objective to mark it complete
	var() bool bOptional;
	var() bool bMarkerAllStances;
	var() bool bLeapFrog;
	//var		bool bComplete;
};

var Array<MemberInfo> SquadMembers; //non-leader members
var(AI) editinline Array<SquadObjective> Objectives;
var Array<SquadVoiceLine> VoiceLines;
var class<SquadStance> Stance;
var class<SquadStance> HUDStance;
var class<SquadStance> LastCommandedStance;
var Actor LastCommandedStanceArgument;
var(Sound) Array<SoundBanter> PotentialBanter;

var Actor CommandedEnemy;

var() static float DefaultSearchAndDestroyRadius;
var float TetherHighPriority;
var float TetherLowPriority;
var float TetherHPActivateRadiusSqr;
var float TetherLPActivateRadiusSqr;
var float TetherHPDeactivateRadiusSqr;
var float TetherLPDeactivateRadiusSqr;

var int CurrentSoundID;	//the ID of the current squad voice line
var Sound CurrentSound;	//the currently playing sound

var float LastRecallTime; //the time of the last recall order
var float LastCommandTime;
var float LastDirectCommandTime;	//the time the player last gave a stance order
var	int	  RecentDirectCommandCount;
var float LastSDChangeTime;
var float NextUpdateTime;
var float NextVoiceUpdateTime;
var float LastExamineTime;
var float WhenBattleBegun;
var float WhenBattleEnded;

var		bool BeCautious;
var(AI) bool PlayerSquad; //if this represents the PlayerSquad
var(AI) bool bUsePlayerPosForObjectives;
var		bool bObjectivePrepped;
var		bool bInBattle;
var		bool bSquadSeesEnemy;	//Whether anyone on the squad has detected an enemy
var		bool bEnemySeesSquad;	//Whether any enemy has targeted any squad member
var		bool bEnemiesPresent;	//Whether there are enemies in the vicinity
var		bool bNonPlayersIncapacitated;

native event AddToSquad(Pawn P);
native function RemoveFromSquad(Pawn P);
native event EnemyKilled(Pawn Victim, Controller Killer, class<DamageType> damageType);
native function AddVoiceLine(Pawn Member, PawnAudioTable.EPawnAudioEvent Cue, optional Object Subject, optional float TriggerInterval, optional float TimeOut);
//native function AddBanter(SoundBanter Banter, bool FallbackToCue);
native function PlayerWentBerserk(Pawn PlayerPawn);
native function PlayerShootingGhosts(Pawn PlayerPawn);
native function int CountAliveMembers(optional bool bIncludeLeader);
native function Pawn GetClosestMemberWithViewOf(Vector point, float CutoffDistance, optional Pawn Exclude, optional bool IncludeLeader, optional bool MustBeAvailable);
//native function DetermineBattleStatus();

function SetStance(class<SquadStance> Stance, optional bool DirectOrder, optional Actor StanceObjective);

function MemberKilled(Pawn Victim, Controller Killer, class<DamageType> damageType);

// Ask whether an event belongs to this actor; used in the editor
function bool ContainsPartialTag(string StartOfTagName)
{
	local string TagString;
	local int i;
	for ( i = 0; i < Objectives.Length; ++i )
	{
		TagString = string(Objectives[i].Tag);
		if ( Caps(StartOfTagName) == Left(Caps(TagString), Len(StartOfTagName) ) )
		{
			return true;
		}
	}
	return Super.ContainsPartialTag(StartOfTagName);
}



defaultproperties
{
     HUDStance=Class'Engine.SquadStance'
     DefaultSearchAndDestroyRadius=768
     LastRecallTime=-10000
     bUsePlayerPosForObjectives=True
}

