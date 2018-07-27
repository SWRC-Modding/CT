class DeathEffectContainer extends Object
	hidecategories(Object)
	abstract;

struct DeathEffectPair
{
	var() editinline DeathCondition		DeathCondition;
	var() editinline DeathEffect		DeathEffect;
};

var() editinline DeathEffect	DefaultDeathEffect;
var() array<DeathEffectPair>	DeathEffects;

static function DoDeathEffect( Pawn Target, class<DamageType> DamageType, float DamageAmount, vector HitLocation, vector Momentum, name HitBone )
{
	local int i;
	local DeathEffect DeathEffect;
	local DeathEffect.DeathEffectInfo DeathInfo;

	DeathInfo.DamageType = DamageType;
	DeathInfo.DamageAmount = DamageAmount;
	DeathInfo.HitLocation = HitLocation;
	DeathInfo.Momentum = Momentum;
	DeathInfo.HitBone = HitBone;

	DeathEffect = default.DefaultDeathEffect;

	for( i = 0; i < default.DeathEffects.length; i++ )
	{		
		if( default.DeathEffects[i].DeathCondition.IsValid( Target, DeathInfo ) )
		{
			DeathEffect = default.DeathEffects[i].DeathEffect;
			break;
		}		
	}

	if( DeathEffect != None )
		DeathEffect.DoDeathEffect( Target, DeathInfo );
}


defaultproperties
{
}

