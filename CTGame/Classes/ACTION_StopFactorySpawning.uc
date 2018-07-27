class ACTION_StopFactorySpawning extends ScriptedAction;

var(Action) name FactoryTag;
var(Action) bool PermanentlyDisableFactory;
var(Action) bool DestroyFactoryOffspring;

function bool InitActionFor(ScriptedController C)
{
	local Factory F;
	local int i;
	
	if ( FactoryTag != '' )
	{
		ForEach C.AllActors(class'Factory',F,FactoryTag)
		{
			if( DestroyFactoryOffspring )
			{
				for( i = 0; i < F.SpawnedActors.length; i++ )
				{					
					F.SpawnedActors[i].Destroy();					
				}

				F.SpawnedActors.length = 0;
			}

			if( PermanentlyDisableFactory )
				F.GotoState('');
			else
				F.GotoState('Offline');			
		}
	}
	return false;	
}


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(FactoryTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}

defaultproperties
{
}

