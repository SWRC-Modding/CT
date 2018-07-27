class StateIdle extends StateObject
	native
	notplaceable;

var float TimeUntilNextIdle;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.G = 255;
	return DebugColor;
}


defaultproperties
{
}

