//=============================================================================
// A camera, used in UnrealEd.
//=============================================================================
class Camera extends PlayerController
	native;

// Sprite.
#exec Texture Import File=Textures\S_Camera.pcx Name=S_Camera Mips=Off MASKED=1


defaultproperties
{
     LightBrightness=100
     LightRadius=16
     Location=(X=-500,Y=-300,Z=300)
     Texture=Texture'Engine.S_Camera'
     CollisionRadius=16
     CollisionHeight=39
     bDirectional=True
}

