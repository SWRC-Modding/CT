class StateFlyingAttack extends StateAbstractAttack
	native
	notplaceable;

var		Pawn				Victim;
var()	float				KamikazeThreshold;
var()	float				PostFirePauseTime;
var()	float				MinDodgeDist;
var()	float				AttachDist;
var()	float				DrillTime;
var()	class<DamageType>	DrillDamageType;
var		bool				CanKamikaze;
var		bool				CanHeadAttach;
	

defaultproperties
{
     PostFirePauseTime=1
}

