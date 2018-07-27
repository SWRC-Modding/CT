class VisionModeHit extends VisionMode
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
     VisionShader=HardwareShader'FrameFX.VisionShaders.VisionShaderHit'
     BloomOveride=True
     BloomOverideValue=255
     BloomOverideFilter=1
     ExtractGlow=False
}

