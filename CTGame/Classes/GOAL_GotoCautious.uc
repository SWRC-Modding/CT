class GOAL_GotoCautious extends GoalObject
	native;

var Vector Destination;
var Vector ToleranceCheckLocation;
var NavigationPoint Goal;
var Array<NavigationPoint> CoverPts;
var int Index;
var float ToleranceSquared;
var bool bMatchOrientation;
var bool bPathComplete;


defaultproperties
{
}

