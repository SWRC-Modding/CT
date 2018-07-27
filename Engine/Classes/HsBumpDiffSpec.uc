class HsBumpDiffSpec extends HardwareShaderWrapper
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Texture			DiffuseTexture;
var() Texture			NormalMap;
var() float				DiffUVScale;
var() float				BumpUVScale;
var(Specular) byte		Specularity;
var(Specular) color		SpecularTint;



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );

}

defaultproperties
{
     DiffUVScale=1
     BumpUVScale=1
     Specularity=100
     ShaderImplementation=HardwareShader'HardwareShaders.Bump.DOT3DiffSpec'
}

