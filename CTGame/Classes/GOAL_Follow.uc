class GOAL_Follow extends GoalObject
	native;

//Task descriptor variables
var bool SucceedOnApproach;
var bool MatchSpeed;
var bool Formation;
var bool bNoPath;
var bool bAnchor;
var bool bFollowAnchor;
var bool bCanMoveDirect;
var bool bCheatedCollision;
var bool bMovedDirectLastTime;

var float LastCanMoveToCheck;
var float FollowOffsetForward;
var float FollowOffsetRight;
var Actor FollowTarget;
var float MinimumDistance;
var float MaximumDistance;
var vector PreviousLocation;
var float StuckTime;

//Instance/temporally dependant variables
//var bool PathIsDirect;
//var bool PathIsSimple;


defaultproperties
{
     MatchSpeed=True
     FollowOffsetForward=512
     FollowOffsetRight=256
     MinimumDistance=250
     MaximumDistance=700
}

