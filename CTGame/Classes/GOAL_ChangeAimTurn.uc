class GOAL_ChangeAimTurn extends GoalObject
	native;

enum EAimTurnGoal
{
	ATG_Focus,
	ATG_FocalPoint,
	ATG_FocalDir
};

var Actor Focus;
var vector FocalPoint;
var rotator FocalDir;
var EAimTurnGoal AimTurnGoal;

var bool EnableAim;
var bool EnableTurn;
var bool TurnExact;
var bool EnableHead;


defaultproperties
{
     EnableAim=True
     EnableTurn=True
}

