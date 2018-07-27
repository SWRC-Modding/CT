class StateMarker extends StateObject
	native
	notplaceable;

//var float Priority;
var float PriorityNoGoals;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.R = 0;
	DebugColor.G = 255;
	DebugColor.B = 255;
	return DebugColor;
}


defaultproperties
{
}

