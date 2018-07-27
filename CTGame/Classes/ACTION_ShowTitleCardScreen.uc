class ACTION_ShowTitleCardScreen extends ScriptedAction;

var(Action)	autoload string	MenuClass;
var(Action)			 string	Args;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;
	
	PC.ShowTitleCardMenu( MenuClass, Args );

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	if ( Caps(StartOfTag) == Left(Caps(MenuClass), Len(StartOfTag) ) )
	{
		return true;
	}
	else if ( Caps(StartOfTag) == Left(Caps(Args), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     MenuClass="XInterfaceCTMenus.CTTitleCardMenu"
     ActionString="Show Title Card Screen"
}

