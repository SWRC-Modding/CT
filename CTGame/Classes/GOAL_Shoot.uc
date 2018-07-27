class GOAL_Shoot extends GoalObject
	native;

// The GOAL will remain active as long as Time > 0 && ( NumShots > 0 || NumShots = -1 )
var float	Time;		// How long to fire
var int		NumShots;	// Number of shots to fire.

defaultproperties
{
     Time=5
     NumShots=-1
}

