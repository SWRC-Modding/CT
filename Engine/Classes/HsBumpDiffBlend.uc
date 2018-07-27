class HsBumpDiffBlend extends HardwareShaderWrapper
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Texture			DiffuseTexture;
var() Texture			BlendTexture;
var() Texture			NormalMap;
var() float				DiffUVScale;
var() float				BlendUVScale;
var() float				BumpUVScale;



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );

}

defaultproperties
{
     DiffUVScale=1
     BlendUVScale=1
     BumpUVScale=1
     ShaderImplementation=HardwareShader'HardwareShaders.Bump.DOT3DiffBlend'
}

