class VisionModeEnhance extends VisionMode
	native;

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
     BloomOveride=True
     BloomOverideValue=1
     BloomOverideFilter=100
     BlurPasses=1
     ExtractGlow=False
     FlashlightOn=False
}

