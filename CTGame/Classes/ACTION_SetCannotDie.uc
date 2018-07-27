class ACTION_SetCannotDie extends ScriptedAction;

var(Action)		name			PawnTag;
var(Action)		bool			bCannotDie;

function bool InitActionFor(ScriptedController C)
{
	local Pawn Pawn;

	ForEach C.AllActors(class'Pawn', Pawn, PawnTag)
	{
		Pawn.bCannotDie = bCannotDie;
	}
	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(PawnTag);
	if ( Caps(PawnTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
}

