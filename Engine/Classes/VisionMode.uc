class VisionMode extends RenderedMaterial
	native;
var(Shaders) HardwareShader	VisionShader;
var(Shaders) HardwareShader	OrganicShader;
var(Shaders) HardwareShader	MechanicalShader;
var(Shaders) HardwareShader	StaticShader;
var(Shaders) HardwareShader	TargetedOrganicShader;
var(Shaders) HardwareShader	TargetedMechanicalShader;
var()		bool			DrawRegularWorld;
var(Bloom)	bool			BloomOveride;
var(Bloom)	byte			BloomOverideValue;
var(Bloom)	byte			BloomOverideFilter;
var()		byte			MotionBlur;
var()		byte			Noise;
var()		int				BlurPasses;
var()		bool			ExtractGlow;
var(Fog)	bool			OverideFog;
var(Fog)	color			DistanceFogColor;
var(Fog)	float			DistanceFogStart;
var(Fog)	float			DistanceFogEnd;
var()		float			FadeInTime;
var()		float			FadeOutTime;
var()		bool			FlashlightOn;
var()		I3DL2Listener	EAXEffect;
var()		I3DL2Listener	EAXEffectXbox;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// Make sure derived classes implement the setup funcion
	virtual UHardwareShader* SetVisionMode( UViewport* Viewport, UProxyBitmapMaterial* RenderedWorldTexture, UProxyBitmapMaterial* BlurTargetTexture, int TargetSizeX, int TargetSizeY, float Fade=1.f ) { return VisionShader; }
	virtual UHardwareShader* GetSkelMeshShader( UViewport* Viewport, AActor* Actor, float Fade=1.f ) { return NULL; }
	virtual UHardwareShader* GetStaticMeshShader( UViewport* Viewport, AActor* Actor, float Fade=1.f ) { return NULL; }

	// TimR: Use first texture as the default size for the browsers
	virtual INT MaterialUSize() 
	{ 
		if( VisionShader && VisionShader->Textures[0] )
			return VisionShader->Textures[0]->MaterialUSize();

		return 0; 
	}
	virtual INT MaterialVSize() 
	{ 
		if( VisionShader && VisionShader->Textures[0] )
			return VisionShader->Textures[0]->MaterialUSize();

		return 0; 
	}
	virtual UBOOL RequiresSorting()
	{
		if( VisionShader )
			return VisionShader->RequiresSorting();

		return 0; 
	}


}

defaultproperties
{
     VisionShader=HardwareShader'FrameFX.VisionShaders.VisionShaderNormal'
     DrawRegularWorld=True
     BlurPasses=4
     ExtractGlow=True
     FlashlightOn=True
}

