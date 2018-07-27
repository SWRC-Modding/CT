//=============================================================================
// SquadStance, base class of all squad stances
//=============================================================================
class SquadStance extends Object
	native
	abstract;

enum EStanceTetherObject
{
	STO_None,
	STO_Leader,
	STO_Objective,
	STO_Self,
	STO_FutureObjective,
	//STO_FormationFocus,
};

// Description of a type of squad stance.
var() localized string     StanceName;	 				// string to describe this stance
var PawnAudioTable.EPawnAudioEvent ActivateCue;		// the cue played when this stance is activated
//var Sound HUDSound;
//A bunch of variables about how squad behaves in this stance
var() float TetherHighPriority;
var() float TetherLowPriority;
var() float TetherHPActivateRadius;
var() float TetherLPActivateRadius;
var() float TetherHPDeactivateRadius;
var() float TetherLPDeactivateRadius;
var() Sound HUDActivateSound;
var() Name StanceChangeEvent;

var class<SquadStance> NextStance;
var float NextStanceTime;
var float PreventRange;
var float IdlePreventRange;
var float BactaPreventRange;
var float BattleRestrictionTime; //if this amount of time passes since the last battle stimulus, the clone isn't allowed to shoot
var float TimeBeforeSeekingCover;
//var float TimeBeforeSeekingBacta;

var		int HUDIconX;	//for stance icons
var		int HUDIconY;

var EStanceTetherObject	PreventType;
var EStanceTetherObject TetherType;
var EStanceTetherObject	FacingType;
//var EStanceTetherObject FormationFocus;

var Squad.EStanceType StanceType;
var		bool bLeapFrog;
var		bool bCheckCorpses;
var() bool FavorCombatNearestTether;
var() bool PreventTetherOneWayOnly;

static function string GetStanceName()
{
	return Default.StanceName;
}


defaultproperties
{
     ActivateCue=PAE_AcknowledgeOrder
     TetherHighPriority=0.8
     TetherLowPriority=0.3
     TetherHPActivateRadius=1024
     TetherLPActivateRadius=768
     TetherHPDeactivateRadius=512
     TetherLPDeactivateRadius=384
     NextStanceTime=1e+020
     PreventRange=1024
     IdlePreventRange=1024
     BactaPreventRange=1024
     BattleRestrictionTime=1e+020
     TimeBeforeSeekingCover=6
     PreventTetherOneWayOnly=True
}

