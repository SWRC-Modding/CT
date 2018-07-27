class ACTION_SetBattleMusicMulti extends ScriptedAction;

var(Action) Sound multiSound;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.SetBattleMusicMulti(multiSound);
	return false;	
}


defaultproperties
{
     ActionString="set battle music multi"
}

