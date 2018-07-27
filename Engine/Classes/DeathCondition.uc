class DeathCondition extends Object
	dependsOn(DeathEffect)
	hidecategories(Object)
	editinlinenew
	abstract;


var() float Probability;

function bool IsValid( Pawn Target, DeathEffect.DeathEffectInfo DeathInfo )
{
	return ( frand() <= Probability );
}


defaultproperties
{
     Probability=1
}

