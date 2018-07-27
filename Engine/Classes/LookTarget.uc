//=============================================================================
// LookTarget
//
// A convenience actor that you can point a matinee camera at.
//
// Isn't bStatic so you can attach these to movers and such.
//
//=============================================================================
class LookTarget extends KeyPoint
	native;

// Sprite.
#exec Texture Import File=Textures\LookTarget.pcx Name=S_LookTarget Mips=Off MASKED=1


defaultproperties
{
     bStatic=False
     bNoDelete=True
     Texture=Texture'Engine.S_LookTarget'
}

