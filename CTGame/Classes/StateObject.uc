class StateObject extends Object
	native
	abstract
	notplaceable;

var CTBot	Bot;
var bool Enabled; //whether the state is currently enabled
//var EStatePhase Phase;
var GoalQueue Goals; //the goal queue this state uses

function color GetDebugColor();


defaultproperties
{
     Enabled=True
}

