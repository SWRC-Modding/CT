class StateMarkerAttack extends StateAbstractAttack
	native
	notplaceable;

var MarkerWeapon Marker;
var float ShotInterval;
var float LastWeaponChange;
var int NumKills;
var	int WeaponGroup;
var Rotator StartRotation;
var vector StartLocation;
var bool bNormalRifle;


defaultproperties
{
     ShotInterval=1.5
     Enabled=False
}

