class StateFreakOut extends StateObject
	native
	notplaceable;

var()	float				Radius;
var()	float				Time;
var		float				EndTime;
var		class<DamageType>	DeathDamageType;

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.R = 255;
	DebugColor.B = 255;
	return DebugColor;
}


defaultproperties
{
     Radius=512
     Time=5
     DeathDamageType=Class'Engine.DamageThrown'
}

