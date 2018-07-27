class StatePatrol extends StateObject
	native
	notplaceable;

var int GoalIndex;
var int Step;
var bool ChangedGodStatus;
var bool InitialGodStatus;

function color GetDebugColor()
{
	local color DebugColor;	
	DebugColor.R = 0;
	DebugColor.G = 0;
	DebugColor.B = 255;
	return DebugColor;
}


defaultproperties
{
}

