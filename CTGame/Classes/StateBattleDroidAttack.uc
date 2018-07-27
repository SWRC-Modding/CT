class StateBattleDroidAttack extends StateAbstractAttack
	native
	notplaceable;

var   float NextManuverTime;	// Timer for manuver time
var() float MaxManuverTime;		// Max time to execute a manuver
var() float MoveIntervalDist;	// How far to move at a time when closing with the player


defaultproperties
{
     MaxManuverTime=10
}

