class GOAL_WaitForEvent extends GoalObject
	native;

//we should be storing a link, ideally
var name Event;
var bool Triggered;

function Trigger()
{
	Triggered = true;
	if (NextGoal != None)
		Utility = NextGoal.Utility;
}


defaultproperties
{
}

