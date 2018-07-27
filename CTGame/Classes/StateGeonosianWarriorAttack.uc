class StateGeonosianWarriorAttack extends StateAbstractAttack
	native
	notplaceable;

var() static float		LungeDist;
var() static float		JumpDist;
var() static float		OffensiveJumpProbability;
var() static float		DefensiveJumpProbability;
var() static float		DefensiveJumpProbabilityForPlayer;
var bool				IsMeleeAttacking;
var bool				MustDoDefensiveJump;



defaultproperties
{
     LungeDist=800
     JumpDist=3000
     OffensiveJumpProbability=0.5
     DefensiveJumpProbability=0.8
     DefensiveJumpProbabilityForPlayer=0.8
}

