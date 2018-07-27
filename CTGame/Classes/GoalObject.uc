class GoalObject extends GoalQueue
	native
	abstract;

var GoalObject NextGoal;
var GoalObject PrevGoal;
var EGoalStatus Status; //whether the GoalObject has begun execution
var bool TopLevel;	//if we were created by a state rather than a goal
//var bool NotifyStateOfDeletion; //whether we need to tell the active state we were deleted
//var StateObject OwnerState; //do we need to keep track of which state created us?
var float Utility;	//for ordering independent goals, only used by some states
//var EGoalStatus Status;


defaultproperties
{
}

