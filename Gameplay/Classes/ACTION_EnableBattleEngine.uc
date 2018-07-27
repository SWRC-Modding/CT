class ACTION_EnableBattleEngine extends ScriptedAction;

var(Action) bool onOff;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.EnableBattleEngine( onOff );
	return false;	
}


defaultproperties
{
     ActionString="enable battle engine"
}

