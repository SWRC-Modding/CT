/*=============================================================================
	UnViewport.h: Unreal viewport object.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UCanvas.
-----------------------------------------------------------------------------*/

//
// Blitting types.
//
enum EViewportBlitFlags{
	// Bitflags.
	BLIT_Fullscreen     = 0x0001, // Fullscreen.
	BLIT_Temporary      = 0x0002, // Temporary viewport.
	BLIT_DibSection     = 0x0004, // Create a DibSection for windowed rendering.
	BLIT_DirectDraw     = 0x0008, // Create Direct3D along with DirectDraw.
	BLIT_Direct3D       = 0x0010, // Create Direct3D along with DirectDraw.
	BLIT_NoWindowChange = 0x0020, // Don't resize existing window frame.
	BLIT_NoWindowFrame  = 0x0040, // Turn off the window frame.
	BLIT_OpenGL			= 0x0080, // OpenGL rendering.
	BLIT_HardwarePaint  = 0x0100, // Window should be repainted in hardware when drawn.

	// Special.
	BLIT_ParameterFlags	= BLIT_NoWindowChange, // Only parameters to ResizeViewport, not permanent flags.
};

//
// A drawing canvas.
//
class ENGINE_API UCanvas : public UObject{
	DECLARE_CLASS(UCanvas,UObject,CLASS_Transient,Engine)
	NO_DEFAULT_CONSTRUCTOR(UCanvas)
public:
	// Variables.
	UFont*       Font;
	FLOAT        SpaceX, SpaceY;
	FLOAT        OrgX, OrgY;
	FLOAT        ClipX, ClipY;
	FLOAT        CurX, CurY;
	FLOAT        Z;
	BYTE         Style;
	FLOAT        CurYL;
	FColor       Color;
	BITFIELD     bCenter:1;
	BITFIELD     bNoSmooth:1;
	INT          SizeX, SizeY;
	FPlane       ColorModulate;
	UFont*       TinyFont;
	UFont*       SmallFont;
	UFont*       MedFont;
	UViewport*   Viewport;
	FCanvasUtil* pCanvasUtil;

	// Overrides
	virtual void Serialize(FArchive& Ar);
	virtual void Destroy();

	// UCanvas interface.
	void Flush();
	void DrawActor(AActor* Actor, UBOOL Wireframe, UBOOL ClearZ, FLOAT DisplayFOV);
	void DrawScreenActor(AActor* Actor, UBOOL Wireframe, UBOOL ClearZ, FLOAT DisplayFOV);
	void SetScreenLight(INT Index, const FVector& Position, FColor Color, FLOAT Radius);
	void Reset();

	// Virtual functions
	virtual void Init(UViewport* InViewport);
	virtual void Update();
	virtual void DrawTile(UMaterial*, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT Z, const FPlane& Color, const FPlane& Fog);
	virtual void DrawRotatedTile(FLOAT, FLOAT, FLOAT, UMaterial*, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT Z, const FPlane& Color, const FPlane& Fog);
	virtual void DrawRotatedTile(FLOAT Angle, UMaterial* Material, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLOAT Z, const FPlane& Color, const FPlane& Fog);
	virtual void DrawCubemap(UMaterial* Material, FLOAT, FLOAT, FLOAT, FLOAT);
	virtual void DrawIcon(UMaterial* Material, FLOAT ScreenX, FLOAT ScreenY, FLOAT XSize, FLOAT YSize, FLOAT Z, const FPlane& Color, const FPlane& Fog);
	virtual void DrawPattern(UMaterial* Material, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Scale, FLOAT OrgX, FLOAT OrgY, FLOAT Z, const FPlane& Color, const FPlane& Fog);
	virtual void VARARGS WrappedStrLenf(UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT&, INT&, const TCHAR*, ...);
	virtual void VARARGS WrappedStrLenf(UFont* Font, INT& XL, INT& YL, const TCHAR* Fmt, ...);
	virtual void VARARGS WrappedPrintf(UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Fmt, ...);
	virtual void VARARGS WrappedPrintf(UFont* Font, UBOOL Center, const TCHAR* Fmt, ...);
	virtual void VARARGS WrappedIconPrintf(UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TArray<UTexture*>&, TCHAR, const TCHAR* Fmt, ...);
	virtual void WrapStringToArray(const TCHAR* Text, TArray<FString>* OutArray, FLOAT Width, UFont* Font = NULL, TCHAR EOL = '\n');
	virtual void ClippedStrLen(UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Text);
	virtual void ClippedPrint(UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text);
	virtual void DrawTileStretched(UMaterial* Material, FLOAT Left, FLOAT Top, FLOAT AWidth, FLOAT AHeight);
	virtual void DrawTileScaled(UMaterial* Material, FLOAT Left, FLOAT Top, FLOAT NewXScale, FLOAT NewYScale);
	virtual void DrawTileBound(UMaterial* Material, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height);
	virtual void DrawTileJustified(UMaterial* Material, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height, BYTE Justification);
	virtual void DrawTileScaleBound(UMaterial* Material, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height);
	virtual void VARARGS DrawTextJustified(BYTE Justification, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, const TCHAR* Fmt, ...);
	virtual void SetClip(INT X, INT Y, INT XL, INT YL);

private:
    // Style is probably one of ERenderStyle
	void WrappedPrint(INT Style, INT& XL, INT& YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text);
	void WrappedIconPrint(INT Style, INT& XL, INT& YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TArray<UTexture*>&, TCHAR, const TCHAR* Text);
};

/*-----------------------------------------------------------------------------
	FHitCause.
-----------------------------------------------------------------------------*/

//
// Information about a hit-test cause.
//
struct ENGINE_API FHitCause
{
	class FHitObserver* Observer;
	UViewport* Viewport;
	DWORD Buttons;
	FLOAT MouseX;
	FLOAT MouseY;
	FHitCause( FHitObserver* InObserver, UViewport* InViewport, DWORD InButtons, FLOAT InMouseX, FLOAT InMouseY )
	: Observer( InObserver ), Viewport( InViewport ), Buttons( InButtons ), MouseX( InMouseX ), MouseY( InMouseY )
	{}
};

/*-----------------------------------------------------------------------------
	FHitObserver.
-----------------------------------------------------------------------------*/

//
// Hit observer class for receiving hit-test notification.
//
class ENGINE_API FHitObserver
{
public:
	virtual void Click( const struct FHitCause& Cause, const struct HHitProxy& Hit ) {}
};

/*-----------------------------------------------------------------------------
	UViewport.
-----------------------------------------------------------------------------*/

// Information for rendering the viewport (detail level settings).
enum ERenderType
{
	REN_None			= 0,	// Hide completely.
	REN_Wire			= 1,	// Wireframe of EdPolys.
	REN_Zones			= 2,	// Show zones and zone portals.
	REN_Polys			= 3,	// Flat-shaded Bsp.
	REN_PolyCuts		= 4,	// Flat-shaded Bsp with normals displayed.
	REN_DynLight		= 5,	// Illuminated texture mapping.
	REN_PlainTex		= 6,	// Plain texture mapping.
	REN_OrthXY			= 13,	// Orthogonal overhead (XY) view.
	REN_OrthXZ			= 14,	// Orthogonal XZ view.
	REN_OrthYZ			= 15,	// Orthogonal YZ view.
	REN_TexView			= 16,	// Viewing a texture (no actor).
	REN_TexBrowser		= 17,	// Viewing a texture browser (no actor).
	REN_MeshView		= 18,	// Viewing a mesh.
	REN_MAX				= 20
};

// ShowFlags for viewport.
enum EViewportShowFlags
{
	SHOW_Frame     		= 0x00000001, 	// Show world bounding cube.
	SHOW_ActorRadii		= 0x00000002, 	// Show actor collision radii.
	SHOW_Backdrop  		= 0x00000004, 	// Show background scene.
	SHOW_Actors    		= 0x00000008,	// Show actors.
	SHOW_Coords    		= 0x00000010,	// Show brush/actor coords.
	SHOW_ActorIcons		= 0x00000020,	// Show actors as icons.
	SHOW_Brush			= 0x00000040,	// Show the active brush.
	SHOW_StandardView	= 0x00000080,	// Viewport is a standard view.
	SHOW_Menu			= 0x00000100,	// Show menu on viewport.
	SHOW_ChildWindow	= 0x00000200,	// Show as true child window.
	SHOW_MovingBrushes	= 0x00000400,	// Show moving brushes.
	SHOW_PlayerCtrl		= 0x00000800,	// Player controls are on.
	SHOW_Paths          = 0x00001000,   // Show paths.
	SHOW_NoButtons		= 0x00002000,	// No menu/view buttons.
	SHOW_RealTime		= 0x00004000,	// Update window in realtime.
};

// Mouse buttons and commands.
enum EMouseButtons
{
	MOUSE_Left			= 0x0001,		// Left mouse button.
	MOUSE_Right			= 0x0002,		// Right mouse button.
	MOUSE_Middle 		= 0x0004,		// Middle mouse button.
	MOUSE_FirstHit		= 0x0008,		// Sent when a mouse button is initially hit.
	MOUSE_LastRelease	= 0x0010,		// Sent when last mouse button is released.
	MOUSE_SetMode		= 0x0020,		// Called when a new viewport mode is first set.
	MOUSE_ExitMode		= 0x0040,		// Called when the existing mode is changed.
	MOUSE_Ctrl			= 0x0080,		// Ctrl is pressed.
	MOUSE_Shift			= 0x0100,		// Shift is pressed.
	MOUSE_Alt			= 0x0200,		// Alt is pressed.
	MOUSE_LeftDouble	= 0x0200,		// Left double click.
};

//
// A viewport object, which associates an actor (which defines
// most view parameters) with a Windows window.
//
class ENGINE_API UViewport : public UPlayer{
	DECLARE_ABSTRACT_CLASS(UViewport,UPlayer,CLASS_Transient,Engine)
	DECLARE_WITHIN(UClient)

	char Padding1[288];
	FRenderInterface* RI;
	char Padding2[64];

	/*// Referenced objects.
	class UCanvas*		 Canvas;	// Viewport's painting canvas.
	class UInput*		 Input;		// Input system.
	class URenderDevice* RenDev;	// Render device.

	// Normal variables.
	UObject*		MiscRes;		// Used in in modes like EM_TEXVIEW.
	FName			Group;			// Group for editing.
	FTime			LastUpdateTime;	// Time of last update.
	INT				SizeX, SizeY;   // Buffer X & Y resolutions.
	INT				ColorBytes;		// 1=256-color, 4=32-bit color.
	INT				FrameCount;		// Frame count, incremented when locked.
	DWORD			Caps;			// Capabilities (CC_).
	UBOOL			Current;		// If this is the current input viewport.
	UBOOL			Dragging;		// Dragging mouse.
	DWORD			RenderFlags;	// Render locking flags (only when locked).
	DWORD			ExtraPolyFlags;	// Additional poly flags associated with camera.

	// Level traveling.
	ETravelType		TravelType;
	FStringNoInit	TravelURL;
	UBOOL			bTravelItems;

	// Frame buffer info; only valid when locked.
	FTime			CurrentTime;	// Time when initially locked.
	BYTE*			ScreenPointer;	// Pointer to screen frame buffer, or NULL if none.
	INT				Stride;			// Stride in pixels.

	// Hit testing.
	UBOOL			HitTesting;		// Whether hit-testing.
	INT				HitX, HitY;		// Hit rectangle top left.
	INT				HitXL, HitYL;	// Hit size.
	TArray<INT>		HitSizes;		// Currently pushed hit sizes.

	// Saved-actor parameters.
	FLOAT SavedOrthoZoom, SavedFovAngle;
	INT SavedShowFlags, SavedRendMap, SavedMisc1, SavedMisc2;

	// Constructor.
	UViewport();

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// FArchive interface.
	void Serialize( const TCHAR* Data, EName MsgType );

	// UPlayer interface.
	void ReadInput( FLOAT DeltaSeconds );*/

	// UViewport interface.
	virtual UBOOL Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData=NULL, INT* HitSize = NULL);
	virtual void Unlock(UBOOL Blit = 0);
	virtual void Present();
	virtual UBOOL SetDrag(UBOOL NewDrag);
	virtual UBOOL IsFullscreen() = 0;
	virtual UBOOL ResizeViewport(DWORD BlitType, INT X = INDEX_NONE, INT Y = INDEX_NONE, INT ColorBytes = INDEX_NONE) = 0;
	virtual void SetModeCursor() = 0;
	virtual void UpdateWindowFrame() = 0;
	virtual void OpenWindow(DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY) = 0;
	virtual void CloseWindow() = 0;
	virtual void UpdateInput(UBOOL Reset) = 0;
	virtual void* GetWindow() = 0;
	virtual void* GetServer();
	virtual void SetMouseCapture(UBOOL Capture, UBOOL Clip, UBOOL FocusOnly = 0) = 0;
	virtual void Repaint(UBOOL Blit) = 0;
	virtual void TryRenderDevice(const TCHAR* ClassName, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen){}
	virtual char* GetLocalizedKeyName(EInputKey Key) = 0;
	virtual void DisableWinKey();
	virtual void EndFullscreen();
	virtual void IgnoreInput(bool Ignore);
	virtual bool IgnoringInput();
	virtual void ProfileSetMarker(char*, INT, INT, INT, INT);
	virtual INT ProfileBeginEvent(char*, INT, INT, INT, INT);
	virtual INT ProfileEndEvent();
};

// Viewport hit-testing macros.
#define PUSH_HIT(frame,expr) if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PushHit(expr,sizeof(expr));
#define POP_HIT(frame)       if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PopHit(0);
#define POP_HIT_FORCE(frame) if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PopHit(1);

/*-----------------------------------------------------------------------------
	Base hit proxy.
-----------------------------------------------------------------------------*/

// Hit proxy declaration macro.
#define DECLARE_HIT_PROXY(cls,parent) \
	const TCHAR* GetName() const \
		{ return TEXT(#cls); } \
	UBOOL IsA( const TCHAR* Str ) const \
		{ return appStricmp(TEXT(#cls),Str)==0 || parent::IsA(Str); }

// Base class for detecting user-interface hits.
struct ENGINE_API HHitProxy
{
	union
	{
		mutable INT Size;
		HHitProxy* Parent;
	};
	const TCHAR* GetName() const
	{
		return TEXT("HHitProxy");
	}
	virtual UBOOL IsA( const TCHAR* Str ) const
	{
		return appStricmp(TEXT("HHitProxy"),Str)==0;
	}
	virtual void Click( const FHitCause& Cause )
	{
		Cause.Observer->Click( Cause, *this );
	}
};

/*-----------------------------------------------------------------------------
	UClient.
-----------------------------------------------------------------------------*/

//
// Client, responsible for tracking viewports.
//
class ENGINE_API UClient : public UObject
{
	DECLARE_ABSTRACT_CLASS(UClient,UObject,CLASS_Config,Engine)

	// Variables.
	UEngine*			Engine;
	TArray<UViewport*>	Viewports;
	INT					DrawCycles;

	// Configurable.
	BITFIELD	CaptureMouse;
	BITFIELD	CurvedSurfaces;
	BITFIELD	ScreenFlashes;
	BITFIELD	NoLighting;
	BITFIELD	Decals;
	BITFIELD	NoDynamicLights;
	BITFIELD    NoFractalAnim;
	INT			WindowedViewportX;
	INT			WindowedViewportY;
	INT			WindowedColorBits;
	INT			FullscreenViewportX;
	INT			FullscreenViewportY;
	INT			FullscreenColorBits;
	FLOAT		Brightness;
	FLOAT		MipFactor;
	INT			TextureLODSet[LODSET_MAX];
	FLOAT		MinDesiredFrameRate;
	INT			ParticleDensity;


	// Constructors.
	UClient();
	void StaticConstructor();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();
	void PostEditChange();

	// UClient interface.
	virtual void Init( UEngine* InEngine )=0;
	virtual void Flush( UBOOL AllowPrecache );
	virtual void ShowViewportWindows( DWORD ShowFlags, int DoShow )=0;
	virtual void EnableViewportWindows( DWORD ShowFlags, int DoEnable )=0;
	virtual void Tick()=0;
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog )=0;
	virtual class UViewport* NewViewport( const FName Name )=0;
	virtual void MakeCurrent( UViewport* NewViewport )=0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
