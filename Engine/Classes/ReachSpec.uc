//=============================================================================
// ReachSpec.
//
// A Reachspec describes the reachability requirements between two NavigationPoints
//
//=============================================================================
class ReachSpec extends Object
	native;

var	int		Distance; 
var	const NavigationPoint	Start;		// navigationpoint at start of this path
var	const NavigationPoint	End;		// navigationpoint at endpoint of this path (next waypoint or goal)
var	int		reachFlags;			// see EReachSpecFlags definition in UnPath.h
var	bool	bPruned;
var	const bool	bForced;
var() bool	bJump;
var() bool	bPerch;
var() bool	bFly;

defaultproperties
{
}

