class StateAbstractAttack extends StateObject
	native
	notplaceable;

var()	static float		ReadyDistance;			//This is the distance we like to keep from the enemy
var()	static float		SafeDistance;			//This is the max distance we like to be from the enemy (except when sniping)

var		Vector		MoveModePosition;		//Current local destination of move-mode
var		Vector		LocalEnemyPos;			//The enemy position in local-space coordinates (updated by ComputeEssentials())
var		Vector		DirToEnemy;				//The direction to the enemy
var		float		DistToEnemy;			//Distance to enemy (updated by ComputeEssentials())
var		float		NextMoveModeTime;		//Next time in game seconds 
var		float		NextToggleAttackTime;	
var		GOAL_Follow FollowEnemyGoal;		//Kept separate from a goal stack, used for running concurrently w/ the rest of the code

var		bool		InOffensiveMode;		//Are we in offensive or defensive mode
var		bool		CanSeeEnemy;			//It the Bot->Enemy visible (updated by ComputeEssentials())
var		bool		ForceMoveModeDecision;	//Force a move mode switch
var		bool		MoveModeBegun;			// Whether BeginCycle has been called on current movemode
var		bool		FinishedMovement;		//Currently used by some movemodes only
var		bool		Melee;					//If we're currently using a melee weapon
var		bool		FallBackFromEnemy;		//if we should advance on the enemy

var		int			LastManuver;			//The last manuver we did

//NoExport variables must be at end
var noexport int CurMoveMode; 

function color GetDebugColor()
{
	local color DebugColor;
	DebugColor.R = 255;
	return DebugColor;
}



defaultproperties
{
     ReadyDistance=800
     SafeDistance=1200
}

