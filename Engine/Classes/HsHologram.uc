class HsHologram extends HardwareShaderWrapper
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Texture		DiffuseTexture;
var() color			HologramColor;
var() bool			UseMarkerColorInstead;



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );

}

defaultproperties
{
     HologramColor=(B=255,G=206,R=122,A=50)
     ShaderImplementation=HardwareShader'HardwareShaders.Hologram.DynamicHologram'
}

