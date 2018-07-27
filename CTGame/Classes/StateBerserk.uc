class StateBerserk extends StateAbstractAttack
	native
	notplaceable;

var() float BerserkThreshold;			// the fraction of your max health that below which you will berserk
var() float	MaxBerserkRange;			// character will only berserk if within this distance from their enemy
var() float BerserkDamageFactor;
	

defaultproperties
{
     BerserkThreshold=0.5
     MaxBerserkRange=1000
     BerserkDamageFactor=0.15
}

