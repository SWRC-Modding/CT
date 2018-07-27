class ACTION_ShowVisor extends ScriptedAction;

var(Action)	bool	bVisor;

function bool InitActionFor(ScriptedController C)
{
	/**** SBD - REMOVED
				This action is no longer necessary, and
				in fact causes problems because it changes
				a user setting.
	
	local PlayerController PC;

	ForEach C.AllActors(class'PlayerController', PC)
		break;

	if (PC != None)
		PC.bVisor = bVisor;
		
	******/

	return false;
}

defaultproperties
{
}

