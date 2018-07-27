class StateScriptWrapper extends StateObject
	native
	notplaceable;

var float Priority;
var Name  WrappedState;

function color GetDebugColor()
{
	local color DebugColor;	
	DebugColor.R = 100;
	DebugColor.G = 50;	
	return DebugColor;
}


defaultproperties
{
}

