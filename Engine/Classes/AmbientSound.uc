//=============================================================================
// Ambient sound, sits there and emits its sound.  This class is no different 
// than placing any other actor in a level and setting its ambient sound.
//=============================================================================
class AmbientSound extends Keypoint;

// Import the sprite.
#exec Texture Import File=Textures\Ambient.pcx Name=S_Ambient Mips=Off MASKED=1


defaultproperties
{
     RemoteRole=ROLE_None
     Texture=Texture'Engine.S_Ambient'
}

