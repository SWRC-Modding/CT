class HsWorldDistortion extends HardwareShaderWrapper
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Color				ColorAdd;
var() Color				ColorVariation;
var() float				WorldDistortion;
var() float				WobbleScale;
var() float				WobbleSpeed;
var() float				WobbleAmplitute;
var() bool				VortexEffect;
var() byte				Alpha;



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );
	virtual UBOOL RequiresSorting()
	{
		return 1;
	}

}

defaultproperties
{
     WorldDistortion=0.0005
     WobbleScale=0.06
     WobbleSpeed=5
     WobbleAmplitute=4
     Alpha=255
     ShaderImplementation=HardwareShader'FrameFX.FBFX.WorldDistortion'
}

