class GOAL_ChangePhysics extends GoalObject
	native;

struct PhysicsData
{
	var Pawn.EPhysics	Physics;
	var Pawn.EIdleState	IdleState;
	var vector			Acceleration;
	var vector			Velocity;
	var bool			ShouldCrouch;
};

var PhysicsData		NewPhysicsData;
//var PhysicsData		OldPhysicsData;
var int				ChangeFlags;


defaultproperties
{
}

