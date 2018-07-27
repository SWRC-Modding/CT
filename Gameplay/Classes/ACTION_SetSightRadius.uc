class ACTION_SetSightRadius extends ScriptedAction;

var(Action)		name			AffectPawnsWithTag;
var(Action)		class<Pawn>		AffectPawnsOfClass;
var(Action)		bool			AffectAttachedPawn;
var(Action)		float			NewSightRadius;
var(Action)		bool			ResetToDefaults;

function bool InitActionFor(ScriptedController C)
{
	local Pawn P;

	ForEach C.AllActors(class'Pawn',P)
	{
		if( ( AffectPawnsWithTag != '' && P.Tag == AffectPawnsWithTag )	||
			( AffectPawnsOfClass != None && P.IsA( AffectPawnsOfClass.Name ) ) )
		{
			if( ResetToDefaults )
				P.SightRadius = P.default.SightRadius;
			else
				P.SightRadius = NewSightRadius;				
		}
	}

	if( AffectAttachedPawn && C.Pawn != None )
	{
		if( ResetToDefaults )
			C.Pawn.SightRadius = C.Pawn.default.SightRadius;
		else
			C.Pawn.SightRadius = NewSightRadius;				
	}

	return false;	
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AffectPawnsWithTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     ActionString="Set Sight Radius"
}

