class StateTrandoshanAttack extends StateAbstractAttack	
	native
	notplaceable;

var() range	CoverHideTime;
var() float	MinStepInterval;

var() float	AllySearchRadius;
var() name	AttackGesture;
var() name	RetreatGesture;
var() name	ChangeCoverGesture;
var() name	HoldGesture;

var() bool	FavorDefensiveCover;


defaultproperties
{
     CoverHideTime=(Min=0.5,Max=2)
     MinStepInterval=10
     AllySearchRadius=1000
}

