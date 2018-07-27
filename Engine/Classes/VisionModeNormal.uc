class VisionModeNormal extends VisionMode
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
     FlashlightOn=False
}

