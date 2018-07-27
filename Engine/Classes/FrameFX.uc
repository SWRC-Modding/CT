class FrameFX extends Object
	native
	editinlinenew
	collapsecategories;

var   byte				Blur;
var   byte				BlurMip;
var	  VisionMode		VisionMode;			// Current vision mode
var   float				VisionModeFade;
var	  float				LastFrameTime;
var	  bool				BloomEnabled;
var	  ZoneInfo			ViewZone;
var	  float				CurrentBloom;
var   float				CurrentBloomFilter;
var() HardwareShader	ShaderBlur;			// Shader to blur for bloom
var() HardwareShader	ShaderGlow;			// Shader for FB glow
var() HardwareShader	ShaderDraw;			// Shader for FB accumulate draw
var() HardwareShader	ShaderEngage;		// Shader for engage target
var() HardwareShader	ShaderShieldHit;	// Shader for shield hit, same as Enhanced shader
var() HardwareShader	ShaderShield;		// Shader for shields
var   VisionMode		DefaultVisionMode;	// Default vision mode to use when none other specified
var   array<HardwareShader>	ActorShaderFX;	// List of shaders to render the current actor with
var   bool				DrawRegularWorld;	// Whether current actor should render normally
var	  int				FSAA;				// Current FSAA setting

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
public:
	// --------- Member Variables --------- 
	// Static working targets (shared even in splitscreen)
	static const int MaxMips = 6;
	static FAuxRenderTarget*	MipTargets[MaxMips];
	static FAuxRenderTarget*	WorkingTarget;
	static FAuxRenderTarget*	AccumulatorTarget;
	static FAuxRenderTarget*	AntiAliasTarget;

	// Static Frame Grid variables
	static class FFrameGrid*	FrameGrid;
	
	// --------- UObject interface --------- 
	void Destroy();
	void Serialize( FArchive& Ar );

	// --------- UFrameFX interface --------- 
	// Access functions
	void SetVisionMode( UVisionMode* NewVisionMode ){ VisionMode = NewVisionMode; }
	void SetVisionModeFade( float NewFade )			{ VisionModeFade = NewFade; }

	// Render Target functions
	void InitRenderTargets( UViewport* Viewport, class FRenderInterface* RI, AZoneInfo* ViewZoneInfo, int FSAA );
	UBOOL CreateRenderTarget( FAuxRenderTarget** Target, UViewport* Viewport,FRenderInterface* RI, float Scale=1.f, bool bMatchBackBuffer=false );
	void FreeRenderTargets();
	void SetWorkingTarget( UViewport* Viewport, FRenderInterface* RI );
	void RestoreBackBuffer( UViewport* Viewport, FRenderInterface* RI );
	void MipWorkingTarget( UViewport* Viewport, FRenderInterface* RI );
	void BlurTarget(UViewport* Viewport,FRenderInterface* RI, FAuxRenderTarget* SourceRenderTarget, FAuxRenderTarget* DestRenderTarget );
	void ExtractGlow(UViewport* Viewport,FRenderInterface* RI, FAuxRenderTarget* SourceRenderTarget, FAuxRenderTarget* DestRenderTarget );
	void AccumulateWorkingTarget( UViewport* Viewport, FRenderInterface* RI );
	void DrawAccumulator( UViewport* Viewport, FRenderInterface* RI );
	void SetPannerInfo( UViewport* Viewport, class UTexMatrix * GlassTexture );
	void CopyBackBufferToAntiAliasTarget( FRenderInterface* RI );
	void SetupActorShaderFX( AActor* Actor, UViewport* Viewport );
	UHardwareShader* GetTacticalSkelMeshShader( UViewport* Viewport, AActor* Actor, float Fade=1.f );

}

defaultproperties
{
     BloomEnabled=True
     CurrentBloomFilter=255
     ShaderBlur=HardwareShader'FrameFX.FBFX.FBMip'
     ShaderGlow=HardwareShader'FrameFX.FBFX.FBGlow'
     ShaderDraw=HardwareShader'FrameFX.FBFX.FBDraw'
     ShaderEngage=HardwareShader'HardwareShaders.Hologram.EngageTarget'
     ShaderShieldHit=HardwareShader'HardwareShaders.Hologram.Enhanced'
     DefaultVisionMode=VisionModeNormal'FrameFX.VisionModes.VisionNormal'
     DrawRegularWorld=True
}

