//=============================================================================
// RallyIconActor.
// An actor that is drawn using a static mesh(a mesh that never changes, and
// can be cached in video memory, resulting in a speed boost).
//=============================================================================

class RallyIconActor extends IntangibleActor
	;


defaultproperties
{
     StaticMesh=StaticMesh'MarkerIcons.Commands.EngageIcon'
     bUnlit=True
     bAlignBottom=True
}

