class StateIdleSquad extends StateObject
	native
	notplaceable;

//Tether functionality
var float ActivateDistance;
var float ReleaseDistance;
var float TetherUtility;
var Pawn TetherTarget;
var float LastCoverCheck;
var Actor UsefulActor;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.G = 255;
	return DebugColor;
}


defaultproperties
{
     ActivateDistance=2560
     ReleaseDistance=1536
     TetherUtility=0.37
}

