class ACTION_FailMission extends ScriptedAction;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	PC.bGodMode = false;
	PC.Pawn.BleedOut();

	return false;	
}



defaultproperties
{
}

