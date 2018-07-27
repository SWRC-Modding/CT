class StateShutdown extends StateObject
	native
	notplaceable;


var Pawn.EIdleState	OldIdleState;
var Pawn.EPhysics	OldPhysics;
var bool			StartedShutdown;

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

