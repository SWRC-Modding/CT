//=============================================================================
// RallyIconActor.
// An actor that is drawn using a static mesh(a mesh that never changes, and
// can be cached in video memory, resulting in a speed boost).
//=============================================================================

class IntangibleActor extends Actor
	native;


defaultproperties
{
     DrawType=DT_StaticMesh
     bAcceptsProjectors=False
     bBlockZeroExtentTraces=False
     bBlockNonZeroExtentTraces=False
     CollisionRadius=1
     CollisionHeight=1
     bEdShouldSnap=True
}

