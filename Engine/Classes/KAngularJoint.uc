#exec Texture Import File=Textures\S_KBSJoint.pcx Name=S_KBSJoint Mips=Off MASKED=1

class KAngularJoint extends KConstraint
    native
    placeable;

var(KarmaConstraint) bool FreeOneAxis;
var(KarmaConstraint) vector FreeAxis;


defaultproperties
{
     Texture=Texture'Engine.S_KBSJoint'
}

