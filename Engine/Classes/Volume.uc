//=============================================================================
// Volume:  a bounding volume
// touch() and untouch() notifications to the volume as actors enter or leave it
// enteredvolume() and leftvolume() notifications when center of actor enters the volume
// pawns with bIsPlayer==true  cause playerenteredvolume notifications instead of actorenteredvolume()
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Volume extends Brush
	native;

struct DecorationType
{
	var() StaticMesh	StaticMesh;
	var() range			Count;
	var() range			DrawScale;
	var() int			bAlign;
	var() int			bRandomPitch;
	var() int			bRandomYaw;
	var() int			bRandomRoll;
};

var() Actor AssociatedActor;			// this actor gets touch() and untouch notifications as the volume is entered or left
var() name AssociatedActorTag;		// Used by L.D. to specify tag of associated actor
var() int LocationPriority;
var() string LocationName;

var(List) array<DecorationType> DecoList;

native function bool Encompasses(Actor Other); // returns true if center of actor is within volume

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( AssociatedActorTag != '' )
		ForEach AllActors(class'Actor',AssociatedActor, AssociatedActorTag)
			break;
	if ( AssociatedActor != None )
	{
		GotoState('AssociatedTouch');
		InitialState = GetStateName();
	}
}

function DisplayDebug(Canvas Canvas, out float YL, out float YPos)
{
	Super.DisplayDebug(Canvas,YL,YPos);
	Canvas.DrawText("AssociatedActor "$AssociatedActor, false);
	YPos += YL;
	Canvas.SetPos(4,YPos);
}	

State AssociatedTouch
{
	event touch( Actor Other )
	{
		AssociatedActor.touch(Other);
	}

	event untouch( Actor Other )
	{
		AssociatedActor.untouch(Other);
	}

	function BeginState()
	{
		local Actor A;

		ForEach TouchingActors(class'Actor', A)
			Touch(A);
	}
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(AssociatedActorTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     LocationName="unspecified"
     bSkipActorPropertyReplication=True
     bCollideActors=True
}

