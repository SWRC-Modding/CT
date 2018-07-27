class GOAL_FollowLink extends GoalObject
	native;

//we should be storing a link, ideally
//var Actor Goal;
var ReachSpec Link;
var Vector Destination;
var vector PreviousLocation;
var float StuckTime;
var bool bLastLink;
var bool bCheatedCollision;
var bool bOldBlockActors;
var bool bOldBlockPlayers;


defaultproperties
{
}

