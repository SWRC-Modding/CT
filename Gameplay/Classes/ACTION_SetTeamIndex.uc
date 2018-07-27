class ACTION_SetTeamIndex extends ScriptedAction;

var(Action) int  NewTeamIndex;
var(Action) name PawnTag;

function bool InitActionFor(ScriptedController C)
{
	local Pawn P;
	if ( PawnTag != '' )
	{
		ForEach C.AllActors(class'Pawn',P,PawnTag)
			P.TeamIndex = NewTeamIndex;
	}
	else
		C.GetInstigator().TeamIndex = NewTeamIndex;
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(PawnTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

defaultproperties
{
}

