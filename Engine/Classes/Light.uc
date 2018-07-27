//=============================================================================
// The light class.
//=============================================================================
class Light extends Actor
	placeable
	native;

#exec Texture Import File=Textures\S_Light.pcx  Name=S_Light Mips=Off MASKED=1

var (Corona)	float	MinCoronaSize;
var (Corona)	float	MaxCoronaSize;
var (Corona)	float	CoronaRotation;
var (Corona)	float	CoronaRotationOffset;
var (Corona)	bool	UseOwnFinalBlend;


defaultproperties
{
     MaxCoronaSize=1000
     LightType=LT_Steady
     LightBrightness=64
     LightRadius=1600
     LightSaturation=255
     LightPeriod=32
     bStatic=True
     bHidden=True
     bNoDelete=True
     bMovable=False
     Texture=Texture'Engine.S_Light'
     CollisionRadius=24
     CollisionHeight=24
}

