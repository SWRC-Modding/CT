class ShadowBitmapMaterial extends BitmapMaterial
	native;

#exec Texture Import file=Textures\blobshadow.dds Name=BlobTexture Mips=On UCLAMPMODE=CLAMP VCLAMPMODE=CLAMP

var const transient int	TextureInterfaces;

var Actor	ShadowActor;
var vector	LightDirection;
var float	LightDistance,
			LightFOV;
var bool	Dirty,
			Invalid,
			bBlobShadow;
var float   CullDistance;
var byte	ShadowDarkness;
var bool    bReady;
var float	LastUpdateTime;

var() BitmapMaterial	BlobShadow;
var() HardwareShader	HSMip;

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

//
//	Default properties
//



cpptext
{
	virtual void Destroy();

	virtual FBaseTexture* GetRenderInterface();
	virtual UBitmapMaterial* Get(FTime Time,UViewport* Viewport);

	void ClearShadowBitmap(FRenderTarget* Target,FRenderInterface* RI,
		FColor ClearColor);

    void BlurTarget( UViewport* Viewport, FRenderInterface* RI, FAuxRenderTarget* SourceRenderTarget, FAuxRenderTarget* DestRenderTarget );

	void UpdateShadowTexture( UViewport* Viewport );

}

defaultproperties
{
     Dirty=True
     ShadowDarkness=255
     BlobShadow=Texture'Engine.BlobTexture'
     HSMip=HardwareShader'Shadow.Blur.Normal'
     Format=TEXF_L8
     UClampMode=TC_Clamp
     VClampMode=TC_Clamp
     UBits=7
     VBits=7
     USize=128
     VSize=128
     UClamp=128
     VClamp=128
}

