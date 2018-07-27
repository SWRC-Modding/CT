class ACTION_ShowPrompt extends ScriptedAction;

var(Action)		localized string	PromptText;
var(Action)		Array<string>		PromptButtonFuncs;
var(Action)		float				PromptTime;
var(Action)		bool				bFlash;
var(Action)		bool				bShowOnPC;
var(Action)		bool				bShowOnConsole;

function bool InitActionFor(ScriptedController C)
{
	local CTPlayer PC;

	if ( IsOnConsole() )
	{
		if ( !bShowOnConsole )
			return false;
	}
	else
	{
		if ( !bShowOnPC )
			return false;
	}

	if(C.Level.NetMode == NM_Standalone)
	{
		ForEach C.AllActors(class'CTPlayer', PC)
			break;

		if ( PC == None )
			return false;

		PC.ShowPrompt(PromptText, PromptButtonFuncs, PromptTime, bFlash, String(Name));
	}
	else
	{
		if(C.Instigator != None && C.Instigator.Controller != None)
		{
			ForEach C.AllActors(class'CTPlayer', PC)
			{
				if(PC == C.Instigator.Controller)
				{
					PC.ShowPrompt(PromptText, PromptButtonFuncs, PromptTime, bFlash, String(Name));
					break;
				}
			}
		}
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = PromptText;
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     PromptTime=5
     bShowOnPC=True
     bShowOnConsole=True
     ActionString="Show Prompt"
}

