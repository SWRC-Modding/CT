class DeathEffect extends Object
	hidecategories(Object)
	editinlinenew
	abstract;

struct DeathEffectInfo
{
	var() class<DamageType> DamageType;
	var() float				DamageAmount;
	var() vector			HitLocation;
	var() vector			Momentum;	
	var() name				HitBone;	
};


function DoDeathEffect( Pawn Target, DeathEffectInfo DeathInfo );



defaultproperties
{
}

