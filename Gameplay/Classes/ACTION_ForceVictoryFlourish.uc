class ACTION_ForceVictoryFlourish extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.ForceBattleFlourish();
	return false;	
}


defaultproperties
{
     ActionString="Force Victory Flourish"
}

