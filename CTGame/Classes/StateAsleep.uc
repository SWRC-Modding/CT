class StateAsleep extends StateShutdown
	native
	notplaceable;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.R = 0;
	DebugColor.G = 0;
	DebugColor.B = 0;	
	return DebugColor;
}


defaultproperties
{
}

