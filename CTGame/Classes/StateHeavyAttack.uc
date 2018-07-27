class StateHeavyAttack extends StateAbstractAttack
	native
	notplaceable;

var   float NextManuverTime;	// Timer for next manuver
var() float MaxManuverTime;		// Max time to execute a manuver
var() float MoveIntervalDist;	// How far to move at a time when closing with the player

var   float NextShieldTime;		// Next shield Time
var() float MinShieldTime;		// Min time between shields
var() float ShieldFactor;		// How much damage to absorb while crouched/shielding

var   bool	HatesDroids;


defaultproperties
{
}

