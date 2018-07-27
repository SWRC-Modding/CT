class VisionModeSniper extends VisionMode
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// Make sure to implement this function in UnVisionModes.cpp
	virtual UHardwareShader* SetVisionMode( UViewport* Viewport, UProxyBitmapMaterial* RenderedWorldTexture, UProxyBitmapMaterial* BlurTargetTexture, int TargetSizeX, int TargetSizeY, float Fade/*=1.f*/ );
	virtual UHardwareShader* GetSkelMeshShader( UViewport* Viewport, AActor* Actor, float Fade/*=1.f*/ );

}

defaultproperties
{
     VisionShader=HardwareShader'FrameFX.VisionShaders.VisionShaderSniper'
     BloomOveride=True
     BloomOverideValue=1
     BloomOverideFilter=200
     BlurPasses=2
     ExtractGlow=False
}

