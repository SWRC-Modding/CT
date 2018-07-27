class ACTION_MoveToPlayer extends LatentScriptedAction;

var(Action) bool WalkToPoint;
var(Action) float Tolerance;

function bool InitActionFor(ScriptedController C)
{
	local Pawn Player;

	C.Pawn.ShouldCrouch( false ); //REVISIT JAH: these should call Bot functions
	if( C.Pawn.Physics != PHYS_Flying )
		C.Pawn.SetWalking( WalkToPoint );
	Player = C.GetMyPlayer();

	//C.ScriptedFocus = Player;

	if (Player != None && CTBot(C) != None)
		CTBot(C).AddScriptedGotoGoal(Player, Tolerance);
	return Super.InitActionFor( C );;	
}

function bool MoveToGoal()
{
	return true;
}

function Actor GetMoveTargetFor(ScriptedController C)
{
	return C.GetMyPlayer();
}


defaultproperties
{
     ActionString="Move to player"
     bValidForTrigger=False
}

