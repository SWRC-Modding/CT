class VisionModeScavDrill extends VisionMode
	native;

var() float MaxBumpSize;
var() float MaxSpinSpeed;
var() float MinTextureScale;
var() float MaxTextureScale;
var() HardwareShader AltVisionShader;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// Make sure to implement this function in UnVisionModes.cpp
	virtual UHardwareShader* SetVisionMode( UViewport* Viewport, UProxyBitmapMaterial* RenderedWorldTexture, UProxyBitmapMaterial* BlurTargetTexture, int TargetSizeX, int TargetSizeY, float Fade/*=1.f*/ );

}

defaultproperties
{
     MaxBumpSize=0.2
     MaxSpinSpeed=100
     MinTextureScale=1
     MaxTextureScale=2
     VisionShader=HardwareShader'FrameFX.VisionShaders.VisionShaderScavDrill'
     BloomOverideValue=255
}

