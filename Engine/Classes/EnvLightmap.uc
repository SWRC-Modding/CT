class EnvLightmap extends Texture
	native
	noexport;

// Render interface for the texture
//var transient int	CubemapRenderInterface;

var() bool	SpecialLit;			// Only use lights that are special lit
var() bool	RemoveLightColor;	// Desaturate the lights in the scene (let vertex lighting effect color only)
var() bool	AddAmbientLight;	// Needed for DiffuseX2 but not for Specular+
var() bool	AmbientGrey;		// Start with 50% grey as base for ModulateX2 specular (BMT_Metal)
var() float SpecularPower;		// Specular power for the lighting
var() byte	SpecularBrightness;	// Needed for specular brightness
var() color	ColorAdd;			// For tinting lights
var() color	ColorMultiplyX2;	// For tinting lights
var transient int RenderTarget; // Render texture target


defaultproperties
{
     RemoveLightColor=True
     AddAmbientLight=True
     SpecularPower=1
     SpecularBrightness=255
     ColorMultiplyX2=(B=128,G=128,R=128,A=128)
}

