class ACTION_BlinkHUDHealth extends ScriptedAction;

var(Action)		float			BlinkTime;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	PC.BlinkHUDHealth( BlinkTime );

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     BlinkTime=5
     ActionString="Blink HUD Health"
}

