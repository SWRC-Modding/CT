class StateRetreat extends StateObject
	native
	notplaceable;

var NavigationPoint RetreatGoal;
var float			HealthThreshold;
var float			DamageThreshold;
var range			EnemyDist;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.B = 255;
	DebugColor.G = 255;
	return DebugColor;
}


defaultproperties
{
     HealthThreshold=0.2
     EnemyDist=(Min=500,Max=2000)
}

