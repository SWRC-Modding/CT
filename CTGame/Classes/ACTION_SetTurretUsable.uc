class ACTION_SetTurretUsable extends ScriptedAction;

var(Action)		name			TurretTag;
var(Action)		bool			IsPlayerUsable;

function bool InitActionFor(ScriptedController C)
{
	local Turret Turret;

	if (TurretTag != 'None')
	{
		ForEach C.AllActors(class'Turret', Turret, TurretTag)
		{	
			Turret.bPlayerUsable = IsPlayerUsable;
		}
	}

	return false;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(TurretTag);
	if ( Caps(TurretTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     IsPlayerUsable=True
}

