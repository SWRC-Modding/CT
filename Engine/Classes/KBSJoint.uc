//=============================================================================
// The Ball-and-Socket joint class.
//=============================================================================

#exec Texture Import File=Textures\S_KBSJoint.pcx Name=S_KBSJoint Mips=Off MASKED=1

class KBSJoint extends KConstraint
    native
    placeable;


defaultproperties
{
     Texture=Texture'Engine.S_KBSJoint'
}

