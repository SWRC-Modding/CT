/*=============================================================================
 Door.
 Used to mark a door on the Navigation network (a door is a mover that may act
 as an obstruction).
=============================================================================
*/
class Door extends NavigationPoint
	placeable
	native;

#exec Texture Import File=Textures\Door.pcx Name=S_Door Mips=Off MASKED=1

//var() name DoorTag;				// tag of mover associated with this node
var()	 Mover MyDoor;
var() name DoorTrigger;			// recommended trigger to use (if door is triggerable)
var actor RecommendedTrigger;
var() bool bInitiallyClosed;	// if true, means that the initial position of the mover blocks navigation
var() bool bBlockedWhenClosed;	// don't even try to go through this path if door is closed
var bool bDoorOpen;

function PostBeginPlay()
{
	local vector Dist;

	if ( DoorTrigger != '' )
	{
		ForEach AllActors(class'Actor', RecommendedTrigger, DoorTrigger )
			break;
		// ignore recommended trigger if door is within its radius
		// (DoorTrigger shouldn't have been set)
		if ( RecommendedTrigger != None )
		{
			Dist = Location - RecommendedTrigger.Location;
			if ( abs(Dist.Z) < RecommendedTrigger.CollisionHeight )
			{
				Dist.Z = 0;
				if ( VSize(Dist) < RecommendedTrigger.CollisionRadius )
					RecommendedTrigger = None;
			}
		}		
	}
	if (bBlockedWhenClosed)
		SetBlocked( bInitiallyClosed );
	bDoorOpen = !bInitiallyClosed;
	Super.PostBeginPlay();
}

function MoverOpened()
{
	if (bBlockedWhenClosed)
		SetBlocked( !bInitiallyClosed  );
	bDoorOpen = bInitiallyClosed;
}

function MoverClosed()
{
	if (bBlockedWhenClosed)
		SetBlocked( bInitiallyClosed );
	bDoorOpen = !bInitiallyClosed;
}

function bool ProceedWithMove(Pawn Other)
{
	if ( MyDoor.bDamageTriggered && (Other.Controller.Focus == MyDoor) )
		Other.Controller.StopFiring();

	if ( bDoorOpen || !MyDoor.bDamageTriggered )
		return true;

	// door still needs to be shot
	Other.SetRotation(rotator(MyDoor.Location - Other.Location));
	Other.Controller.Focus = MyDoor;
	Other.ShootSpecial(MyDoor);
	MyDoor.Trigger(Other,Other);
	Other.Controller.WaitForMover(MyDoor);
	return false;
}

event bool SuggestMovePreparation(Pawn Other)
{
	if ( bDoorOpen )
		return false;
	if ( MyDoor.bOpening || MyDoor.bDelaying )
	{
		Other.Controller.WaitForMover(MyDoor);
		return true;
	}
	if ( MyDoor.bDamageTriggered )
	{
		// handle shootable doors
		Other.SetRotation(rotator(MyDoor.Location - Other.Location));
		Other.Controller.Focus = MyDoor;
		Other.ShootSpecial(MyDoor);
		MyDoor.Trigger(Other,Other);
		Other.Controller.WaitForMover(MyDoor);
		return true;
	}

	return false;
}


defaultproperties
{
     bInitiallyClosed=True
     ExtraCost=100
     bSpecialMove=True
     RemoteRole=ROLE_None
     Texture=Texture'Engine.S_Door'
}

