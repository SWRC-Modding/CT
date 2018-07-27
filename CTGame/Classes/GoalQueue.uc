//=============================================================================
// Clone Troooper Weapon base class
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class GoalQueue extends Object
	native;

enum EGoalStatus {
	GS_Inactive,	//the goal hasn't been activated yet
	GS_PlanFailure,	//the goal failed because of a plan failure (no solution possible)
	GS_ExecutionFailure,	//the goal failed because it of a problem during execution (recovery possible)
	GS_InProgress, //the goal is currently in progress
	GS_Success //goal completed successfully
};

var	GoalObject Head;
var GoalObject Tail;
var int	FailuresLeft;
var CTBot	Bot;	//the Bot that owns this goal queue

native final function Clear(); //remove all goals from the queue
native final function AddGoal(GoalObject Goal);


defaultproperties
{
     FailuresLeft=10
}

