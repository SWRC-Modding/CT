class StatePerched extends StateShutdown
	native
	notplaceable;

var		Perch			MyPerch;
var		float			NextDetachTime;
var()	float			DetachDelayTime;
var		Actor			StimInstigator;

function color GetDebugColor()
{
	local color DebugColor;	
	DebugColor.R = 255;
	DebugColor.B = 255;
	return DebugColor;
}


defaultproperties
{
     DetachDelayTime=3
}

