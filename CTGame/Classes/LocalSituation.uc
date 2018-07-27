//=============================================================================
// Clone Troooper Weapon base class
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================

class LocalSituation extends Object
	native
	notplaceable;

//could pack these into bytes
var int TotalEnemies; //total number of enemies w/in situation distance
var int TotalAllies; //total number of allies w/in situation distance
var int EnemiesICanDetect; //total enemies I can detect (or remember)
var int AlliesNearby; //the number of enemies nearby (within ~10 meters)

var float LastUpdateTime;
var float ClosestEnemyDistSqr;
var Actor BestTarget;

//Actor ClosestThreat;


defaultproperties
{
}

