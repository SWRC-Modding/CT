class ACTION_ForceBattleMusic extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.ForceBattleMusic();
	return false;	
}


defaultproperties
{
     ActionString="force battle zone"
}

