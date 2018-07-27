class StateSpiderDroidAttack extends StateAbstractAttack
	native
	notplaceable;

var		bool	bCanBlock;
var		bool	bCanBlockNow;
var()	float	ShotRange;					// the facing ratio between us and our target within which we will try to shoot

var()	int		ProxDefNPCThreshold;		// The number of enemy NPCs that have to be near the SBD to initiate a prox defense
var()	float	ProxDefRange;				// The distance within which to search for NPCs
var()	float	ProxDefMinInterval;			// The min amount of time between defensive moves
var		float	NextProxDefTime;

var()	float	MinDefenseInterval;
var		float	NextDefenseTime;

var()	range	BlinkInterval;
var		float	NextBlinkTime;

var()	Accessory Scanner;


defaultproperties
{
     bCanBlock=True
     ProxDefNPCThreshold=2
     ProxDefRange=600
     ProxDefMinInterval=12
     MinDefenseInterval=10
     BlinkInterval=(Min=5,Max=7)
}

