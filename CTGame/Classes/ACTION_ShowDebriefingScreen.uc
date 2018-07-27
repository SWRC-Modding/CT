class ACTION_ShowDebriefingScreen extends ScriptedAction;

var(Action)	autoload	string				MenuClass;
var(Action)		localized string	Title;
var(Action)		localized string	Text;
var(Action)		string				PicName;
var(Action)		string				NewLevel;
var(Action)     bool				bIsForHintText;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	ForEach C.AllActors(class'CTPlayer', PC)
		break;

	if ( PC == None )
		return false;

	if (!bIsForHintText || PC.bKeepHintMenusAwfulHack)
		PC.ShowMenu( MenuClass, Title, Text, PicName, NewLevel );
	
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	if ( ( Caps(StartOfTag) == Left( Caps(MenuClass), Len(StartOfTag) ) ) ||
	     ( Caps(StartOfTag) == Left( Caps(Title), Len(StartOfTag) ) ) ||
	     ( Caps(StartOfTag) == Left( Caps(Text), Len(StartOfTag) ) ) ||
	     ( Caps(StartOfTag) == Left( Caps(PicName), Len(StartOfTag) ) ) ||
	     ( Caps(StartOfTag) == Left( Caps(NewLevel), Len(StartOfTag) ) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     MenuClass="XInterfaceCTMenus.CTDebriefingInfo"
     ActionString="Show Menu Screen"
}

