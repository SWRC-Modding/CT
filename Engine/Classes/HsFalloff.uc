class HsFalloff extends HardwareShaderWrapper
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Texture						DiffuseTexture;
var() Texture						FalloffGradient;
var() float							UPanRate;
var() float							VPanRate;
var() float							UOscilationRate;
var() float							VOscilationRate;
var() float							GradientBlendMode;
var() color							DiffuseTint;
var() color							GradientTint;
var() HardwareShader.ED3DBLEND		SrcBlend;
var() HardwareShader.ED3DBLEND		DestBlend;
var() bool							ZTest;
var() bool							ZWrite;



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );

}

defaultproperties
{
     GradientBlendMode=1
     DiffuseTint=(B=255,G=255,R=255,A=255)
     GradientTint=(B=255,G=255,R=255,A=255)
     SrcBlend=SRCCOLOR
     DestBlend=ONE
     ZTest=True
     ShaderImplementation=HardwareShader'HardwareShaders.Falloff.FalloffImpl'
}

