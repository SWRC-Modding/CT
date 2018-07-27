class VisionModeDistort extends VisionMode
	native;

var() float BumpSize;
var() float BumpScaleX;
var() float BumpScaleY;
var() float PanSpeedX;
var() float PanSpeedY;
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
     BumpSize=0.12
     BumpScaleX=1
     BumpScaleY=1
     VisionShader=HardwareShader'FrameFX.VisionShaders.VisionShaderIncap'
     BloomOveride=True
     BloomOverideValue=255
}

