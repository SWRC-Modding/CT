class ACTION_ShowMenuScreen extends ScriptedAction;

var(Action)	autoload	string			MenuClass;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	PC.ShowMenu(MenuClass);

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	if ( Caps(StartOfTag) == Left(Caps(MenuClass), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Show Menu Screen"
}

