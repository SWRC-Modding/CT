class ACTION_AttachToTurret extends ScriptedAction;

var(Action) name DestinationTag;	// tag of turret - if none, then use the ScriptedSequence
var(Action) bool Attach;

function bool InitActionFor(ScriptedController C)
{
	local Turret Turret;
	if ( DestinationTag != '' )
	{
		ForEach C.AllActors(class'Turret',Turret,DestinationTag)
			break;

		if ( Turret != None )
		{
			C.Machine = Turret;
			if( Attach )
			{
				if( Turret.IsAvailableFor( C.Pawn ) )
					CTBot(C).ForceState(class'StateTurretAttack',true);
			}
			else
				CTBot(C).UnForceState();
		}
	}

	return false;	
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString@DestinationTag$" // "$Comment;
	return ActionString@DestinationTag;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(DestinationTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     Attach=True
     ActionString="AttachToTurret"
     bValidForTrigger=False
}

