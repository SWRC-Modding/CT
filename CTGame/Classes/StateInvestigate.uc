class StateInvestigate extends StateObject
	native
	notplaceable;

var Stimulus CurrentStimulus;
var float CurrentStimulusPriority;
var bool  bInvestigating;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.R = 255;
	DebugColor.G = 128;
	return DebugColor;
}


defaultproperties
{
}

