//=============================================================================
// MarkerIconActor.
// An actor that is drawn using a static mesh(a mesh that never changes, and
// can be cached in video memory, resulting in a speed boost).
//=============================================================================

class MarkerIconActor extends Actor //maybe make it inherit from StaticMeshActor
	native
	placeable;

var Color CurrentColor;	// TimR: Color to draw the marker icon
var	float CurrentAlpha;	// TimR: Current Alpha for fading
var	float CurrentMorph;	// TimR: Current Morph factor


defaultproperties
{
     CurrentColor=(B=90,G=90,R=255,A=128)
     CurrentAlpha=1
     CurrentMorph=1
     DrawType=DT_StaticMesh
     bHidden=True
     bAcceptsProjectors=False
     bUnlit=True
     CollisionRadius=1
     CollisionHeight=1
     bEdShouldSnap=True
}

