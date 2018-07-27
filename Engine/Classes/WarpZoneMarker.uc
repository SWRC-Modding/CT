//=============================================================================
// WarpZoneMarker.
//=============================================================================
class WarpZoneMarker extends NavigationPoint
	native;

var WarpZoneInfo markedWarpZone;

// AI related
//var Actor TriggerActor;		//used to tell AI how to trigger me
//var Actor TriggerActor2;

function PostBeginPlay()
{
	//if ( markedWarpZone.numDestinations > 1 )
	//	FindTriggerActor();
	Super.PostBeginPlay();
}

/*
function FindTriggerActor()
{
	local ZoneTrigger Z;
	ForEach AllActors(class 'ZoneTrigger', Z)
		if ( Z.Event == markedWarpZone.ZoneTag)
		{
			TriggerActor = Z;
			return;
		} 
}
*/


defaultproperties
{
     bRemoveMeOnRebuild=True
     bCollideWhenPlacing=False
     bHiddenEd=True
}

