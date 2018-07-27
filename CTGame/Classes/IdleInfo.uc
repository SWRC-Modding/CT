//=============================================================================
// IdleInfo: this is a base class (and set of subclasses) which describe how
// often idle animations play and which animations can be played
//=============================================================================
class IdleInfo extends Object
	native
	abstract;


struct IdleAnimInfo {
	var() name  AnimationName;
	var() float RelativeWeight;
};


var() editinline Array<IdleAnimInfo> IdleAnimations;
var() float MinTimeUntilNextCheck;
var() float MaxTimeUntilNextCheck;


defaultproperties
{
     MinTimeUntilNextCheck=5
     MaxTimeUntilNextCheck=10
}

