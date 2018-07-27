class ACTION_MakeInvulnerable extends ScriptedAction;

var(Action)		name				Tag;
var(Action)		bool				bMakeGod;

function bool InitActionFor(ScriptedController C)
{
	local Pawn CurrentPawn;
	local Prop CurrentProp;

	if(Tag != '')
	{
		ForEach C.AllActors(class'Pawn', CurrentPawn, Tag)
		{
			if( CurrentPawn.Controller != None )
				CurrentPawn.Controller.bGodMode = bMakeGod;
		}

		ForEach C.AllActors(class'Prop', CurrentProp, Tag)
		{
			if( bMakeGod )
				CurrentProp.GotoState('');
			else
				CurrentProp.GotoState('Invulnerable'); // The invulnerable state will hand off to the damageable state if appropriate
		}		
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(Tag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}




defaultproperties
{
     bMakeGod=True
     ActionString="Make Invulnerable"
}

