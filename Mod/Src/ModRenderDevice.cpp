#include "ModRenderDevice.h"
#include "Engine.h"
#include "Editor.h"

/*
 * D3D8 declarations to avoid including d3d8.h which is not available with newer SDKs
 */

#pragma pack(push, 4)

struct IDirect3DSurface8;
struct IDirect3DTexture8;
struct D3DPRESENT_PARAMETERS;

enum D3DFORMAT{
	D3DFMT_A8R8G8B8    = 21,
	D3DFMT_V8U8        = 60,
	D3DFMT_L6V5U5      = 61,
	D3DFMT_X8L8V8U8    = 62,

	D3DFMT_FORCE_DWORD = 0x7fffffff
};

typedef DWORD D3DRESOURCETYPE;
typedef DWORD D3DPOOL;
typedef DWORD D3DMULTISAMPLE_TYPE;
typedef DWORD D3DDEVTYPE;

struct D3DSURFACE_DESC{
	D3DFORMAT           Format;
	D3DRESOURCETYPE     Type;
	DWORD               Usage;
	D3DPOOL             Pool;
	UINT                Size;
	D3DMULTISAMPLE_TYPE MultiSampleType;
	UINT                Width;
	UINT                Height;
};

struct D3DLOCKED_RECT{
	INT   Pitch;
	void* pBits;
};

#pragma pack(pop)

// Virtual table indices of different d3d functions that we are trying to hook
enum{
	// D3D8
	D3DVTIdx_D3D8CreateDevice      = 15,
	// D3DDevice
	D3DVTIdx_DeviceCreateTexture   = 20,
	D3DVTIdx_DeviceGetRenderTarget = 32,
	// D3DSurface
	D3DVTIdx_SurfaceGetDesc        = 8,
	D3DVTIdx_SurfaceLockRect       = 9,
	D3DVTIdx_SurfaceUnlockRect     = 10,
	// D3DTexture
	D3DVTIdx_TextureGetLevelDesc   = 14,
	D3DVTIdx_TextureLockRect       = 16,
	D3DVTIdx_TextureUnlockRect     = 17
};

static HRESULT D3DDeviceGetRenderTarget(IDirect3DDevice8* Self, IDirect3DSurface8** ppRenderTarget)
{
	void** VTable = *reinterpret_cast<void***>(Self);
	return reinterpret_cast<HRESULT(__stdcall*)(IDirect3DDevice8*, IDirect3DSurface8**)>(
		VTable[D3DVTIdx_DeviceGetRenderTarget])(Self, ppRenderTarget);
}

static HRESULT D3DSurfaceGetDesc(IDirect3DSurface8* Self, D3DSURFACE_DESC *pDesc)
{
	void** VTable = *reinterpret_cast<void***>(Self);
	return reinterpret_cast<HRESULT(__stdcall*)(IDirect3DSurface8*, D3DSURFACE_DESC*)>(
		VTable[D3DVTIdx_SurfaceGetDesc])(Self, pDesc);
}

static HRESULT D3DSurfaceLockRect(IDirect3DSurface8* Self, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	void** VTable = *reinterpret_cast<void***>(Self);
	return reinterpret_cast<HRESULT(__stdcall*)(IDirect3DSurface8*, D3DLOCKED_RECT*, const RECT*, DWORD)>(
		VTable[D3DVTIdx_SurfaceLockRect])(Self, pLockedRect, pRect, Flags);
}

static HRESULT D3DSurfaceUnlockRect(IDirect3DSurface8* Self)
{
	void** VTable = *reinterpret_cast<void***>(Self);
	return reinterpret_cast<HRESULT(__stdcall*)(IDirect3DSurface8*)>(
		VTable[D3DVTIdx_SurfaceUnlockRect])(Self);
}

static HRESULT D3DTextureGetLevelDesc(IDirect3DTexture8* Self, UINT Level, D3DSURFACE_DESC* pDesc)
{
	void** VTable = *reinterpret_cast<void***>(Self);
	return reinterpret_cast<HRESULT(__stdcall*)(IDirect3DTexture8*, UINT, D3DSURFACE_DESC*)>(
		VTable[D3DVTIdx_TextureGetLevelDesc])(Self, Level, pDesc);
}

/*
 * Helper function that only patches a vtable if it wasn't done already.
 * This is needed because the editor creates multiple render devices which in turn create new D3D8 devices that need to be patched again.
 * However, we have no way of knowing about that in e.g. a CreateTexture call and thus we have to check each time whether the function at the
 * vtable index is already the override function and if not patch it.
 */
template<typename F>
static void MaybePatchVTable(F* OutFunc, void* Object, INT Index, F NewFunc)
{
	if((*reinterpret_cast<void***>(Object))[Index] == NewFunc)
	{
		checkSlow(OutFunc);

		return;
	}

	*OutFunc = static_cast<F>(PatchVTable(Object, Index, NewFunc));
}

/*
 * L6V5U5 to X8L8V8U8 texture format conversion.
 *
 * L6V5U5 is not supported on newer hardware and either makes the game and editor crash or the driver converts it directly to X8L8V8U8 which results in visual artifacts.
 * That's why we detect if a texture is created with the L6V5U5 format and perform a proper conversion to X8L8V8U8 or if that is not available V8U8 and if not even that one works ARGB8.
 */

// Information about the current texture returned by CreateTexture and used by Lock/UnlockRect
static IDirect3DTexture8* CurrentD3D8Texture;
static D3DFORMAT          CurrentD3D8TextureSourceFormat;
static D3DFORMAT          CurrentD3D8TextureTargetFormat;
static UINT               CurrentD3D8TextureNumMipLevels;
static UINT               CurrentD3D8MipLevelWidth;
static UINT               CurrentD3D8MipLevelHeight;
static void*              CurrentD3D8MipLevelPixels;

// Conversion from one bumpmap format to another

/*
 * L6V5U5 format conversion
 */

static void ConvertL6V5U5ToX8L8V8U8(void* Dest, const void* Src, INT Width, INT Height)
{
	INT NumPixels = Width * Height;

	for(INT i = 0; i < NumPixels; ++i)
	{
		const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(Src) + i;
		FX8L8V8U8Pixel* P2 = static_cast<FX8L8V8U8Pixel*>(Dest) + i;

		P2->V = Map5BitSignedTo8BitSigned(P1->V);
		P2->U = Map5BitSignedTo8BitSigned(P1->U);
		P2->L = Map6BitUnsignedTo8BitUnsigned(P1->L);
		P2->X = P2->L; // L6V5U5 only has one luminance value so we can just reuse it
	}
}

static void ConvertL6V5U5ToV8U8(void* Dest, const void* Src, INT Width, INT Height)
{
	INT NumPixels = Width * Height;

	for(INT i = 0; i < NumPixels; ++i)
	{
		const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(Src) + i;
		FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Dest) + i;

		P2->V = Map5BitSignedTo8BitSigned(P1->V);
		P2->U = Map5BitSignedTo8BitSigned(P1->U);
		// No luminance
	}
}

/*
 * X8L8V8U8 format conversion
 */

static void ConvertX8L8V8U8ToV8U8(void* Dest, const void* Src, INT Width, INT Height)
{
	INT NumPixels = Width * Height;

	for(INT i = 0; i < NumPixels; ++i)
	{
		const FX8L8V8U8Pixel* P1 = static_cast<const FX8L8V8U8Pixel*>(Src) + i;
		FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Dest) + i;

		P2->V = Map5BitSignedTo8BitSigned(P1->V);
		P2->U = Map5BitSignedTo8BitSigned(P1->U);
		// No luminance
	}
}

/*
 * LockRect
 */

#define D3DTEXTURE_LOCKRECT(name) HRESULT __stdcall name(IDirect3DTexture8* D3DTexture, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
typedef D3DTEXTURE_LOCKRECT(D3DTextureLockRectFunc);

static D3DTextureLockRectFunc* D3DTextureLockRect = NULL;

static D3DTEXTURE_LOCKRECT(D3DTextureLockRectOverride)
{
	if(D3DTexture != CurrentD3D8Texture)
		return D3DTextureLockRect(D3DTexture, Level, pLockedRect, pRect, Flags);

	D3DSURFACE_DESC SurfaceDesc;
	HRESULT GetLevelDescResult = D3DTextureGetLevelDesc(D3DTexture, Level, &SurfaceDesc);

	if(FAILED(GetLevelDescResult))
		return GetLevelDescResult;

	CurrentD3D8MipLevelWidth = SurfaceDesc.Width;
	CurrentD3D8MipLevelHeight = SurfaceDesc.Height;
	check(CurrentD3D8MipLevelPixels == NULL);
	CurrentD3D8MipLevelPixels = appMalloc(SurfaceDesc.Size);

	INT BytesPerPixel = 0;

	switch(CurrentD3D8TextureSourceFormat)
	{
	case D3DFMT_A8R8G8B8:
		BytesPerPixel = sizeof(FColor);
		break;
	case D3DFMT_V8U8:
		BytesPerPixel = sizeof(FV8U8Pixel);
		break;
	case D3DFMT_L6V5U5:
		BytesPerPixel = sizeof(FL6V5U5Pixel);
		break;
	case D3DFMT_X8L8V8U8:
		BytesPerPixel = sizeof(FX8L8V8U8Pixel);
		break;
	default:
		appErrorf("Unexpected texture format (%i)", CurrentD3D8TextureSourceFormat);
	}

	pLockedRect->Pitch = CurrentD3D8MipLevelWidth * BytesPerPixel;
	pLockedRect->pBits = CurrentD3D8MipLevelPixels;

	return S_OK;
}

/*
 * UnlockRect
 */

#define D3DTEXTURE_UNLOCKRECT(name) HRESULT __stdcall name(IDirect3DTexture8* D3DTexture, UINT Level)
typedef D3DTEXTURE_UNLOCKRECT(D3DTextureUnlockRectFunc);

static D3DTextureUnlockRectFunc* D3DTextureUnlockRect = NULL;

static D3DTEXTURE_UNLOCKRECT(D3DTextureUnlockRectOverride)
{
	if(D3DTexture != CurrentD3D8Texture)
		return D3DTextureUnlockRect(D3DTexture, Level);

	D3DLOCKED_RECT LockedRect;
	HRESULT Result = D3DTextureLockRect(D3DTexture, Level, &LockedRect, NULL, 0);

	if(SUCCEEDED(Result))
	{
		if(CurrentD3D8TextureSourceFormat == D3DFMT_V8U8)
		{
			ConvertV8U8ToBGRA8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
		}
		else if(CurrentD3D8TextureSourceFormat == D3DFMT_L6V5U5)
		{
			if(CurrentD3D8TextureTargetFormat == D3DFMT_V8U8)
				ConvertL6V5U5ToV8U8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
			else if(CurrentD3D8TextureTargetFormat == D3DFMT_X8L8V8U8)
				ConvertL6V5U5ToX8L8V8U8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
			else if(CurrentD3D8TextureTargetFormat == D3DFMT_A8R8G8B8)
				ConvertL6V5U5ToBGRA8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
		}
		else if(CurrentD3D8TextureSourceFormat == D3DFMT_X8L8V8U8)
		{
			if(CurrentD3D8TextureTargetFormat == D3DFMT_V8U8)
				ConvertX8L8V8U8ToV8U8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
			else if(CurrentD3D8TextureTargetFormat == D3DFMT_A8R8G8B8)
				ConvertX8L8V8U8ToBGRA8(LockedRect.pBits, CurrentD3D8MipLevelPixels, CurrentD3D8MipLevelWidth, CurrentD3D8MipLevelHeight);
		}

		Result = D3DTextureUnlockRect(D3DTexture, Level);
	}

	appFree(CurrentD3D8MipLevelPixels);
	CurrentD3D8MipLevelPixels = NULL;

	if(Level == CurrentD3D8TextureNumMipLevels - 1) // This is the last mip level which means the current texture is fully converted and should be set to NULL
		CurrentD3D8Texture = NULL;

	return Result;
}

/*
 * CreateTexture
 */

#define D3DDEVICE_CREATETEXTURE(name) HRESULT __stdcall name(IDirect3DDevice8* D3DDevice, \
                                                             UINT Width, \
                                                             UINT Height, \
                                                             UINT Levels, \
                                                             DWORD Usage, \
                                                             D3DFORMAT Format, \
                                                             D3DPOOL Pool, \
                                                             IDirect3DTexture8** ppTexture)
typedef D3DDEVICE_CREATETEXTURE(D3DDeviceCreateTextureFunc);

static D3DDeviceCreateTextureFunc* D3DDeviceCreateTexture = NULL;

static D3DDEVICE_CREATETEXTURE(D3DDeviceCreateTextureOverride)
{
	// X8L8V8U8 is used as the first fallback format because no information is lost in the conversion
	D3DFORMAT FallbackFormat = Format == D3DFMT_L6V5U5 ? D3DFMT_X8L8V8U8 : Format;
	HRESULT Result = D3DDeviceCreateTexture(D3DDevice,
	                                        Width,
	                                        Height,
	                                        Levels,
	                                        Usage,
	                                        FallbackFormat,
	                                        Pool,
	                                        ppTexture);

	if(SUCCEEDED(Result)  && FallbackFormat == Format)
		return Result; // No fallback format was needed so just return

	if(FAILED(Result) && Format < D3DFMT_V8U8 && Format > D3DFMT_X8L8V8U8) // CreateTexture failed with a non-bumpmap format
		appErrorf("CreateTexture failed (Format: %i)", Format);            // Should never happen but this is a better error than the engine produces

	if(FAILED(Result)) // If X8L8V8U8 is not supported V8U8 might still be so try that. Visually the same except for missing luminance
	{
		FallbackFormat = D3DFMT_V8U8;
		Result = D3DDeviceCreateTexture(D3DDevice,
		                                Width,
		                                Height,
		                                Levels,
		                                Usage,
		                                FallbackFormat,
		                                Pool,
		                                ppTexture);
	}

	if(FAILED(Result)) // If no bumpmap format is available we fall back to ARGB. Looks fine visually and should always be supported
	{
		FallbackFormat = D3DFMT_A8R8G8B8;
		Result = D3DDeviceCreateTexture(D3DDevice,
		                                Width,
		                                Height,
		                                Levels,
		                                Usage,
		                                FallbackFormat,
		                                Pool,
		                                ppTexture);
	}

	if(FAILED(Result))
		appErrorf("CreateTexture failed even with fallback format (Format: %i)", FallbackFormat);

	// Patching vtables if it wasn't done already

	MaybePatchVTable(&D3DTextureLockRect, *ppTexture, D3DVTIdx_TextureLockRect, D3DTextureLockRectOverride);
	MaybePatchVTable(&D3DTextureUnlockRect, *ppTexture, D3DVTIdx_TextureUnlockRect, D3DTextureUnlockRectOverride);

	// Updating the current texture with the newly created one
	CurrentD3D8Texture = *ppTexture;
	CurrentD3D8TextureSourceFormat = Format;
	CurrentD3D8TextureTargetFormat = FallbackFormat;
	CurrentD3D8TextureNumMipLevels = Levels;

	return Result;
}

/*
 * CreateDevice
 */

#define D3D_CREATEDEVICE(name) HRESULT __stdcall name(IDirect3D8* D3D8, \
                                                      UINT Adapter, \
                                                      D3DDEVTYPE DeviceType, \
                                                      HWND hFocusWindow, \
                                                      DWORD BehaviorFlags, \
                                                      D3DPRESENT_PARAMETERS* pPresentationParameters, \
                                                      IDirect3DDevice8** ppReturnedDeviceInterface)
typedef D3D_CREATEDEVICE(D3D8CreateDeviceFunc);

static D3D8CreateDeviceFunc* D3D8CreateDevice = NULL;

static D3D_CREATEDEVICE(D3D8CreateDeviceOverride)
{
	HRESULT Result = D3D8CreateDevice(D3D8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if(SUCCEEDED(Result))
		MaybePatchVTable(&D3DDeviceCreateTexture, *ppReturnedDeviceInterface, D3DVTIdx_DeviceCreateTexture, D3DDeviceCreateTextureOverride);

	return Result;
}

/*
 * FModRenderInterface
 */

FSelectionRenderInterface::FSelectionRenderInterface(UModRenderDevice* InRenDev)
{
	RenDev = InRenDev;

	HitStack.SetNoShrink(true);
	AllHitData.SetNoShrink(true);
}

/*
 * Converts a color value read from the frame buffer to an index that is stored in OutIndex.
 * If this function returns true, it means that a 'preferred' selection type was found which should be used if there are more than one possible selections.
 */
bool FSelectionRenderInterface::ProcessHitColor(FColor HitColor, INT* OutIndex)
{
	INT Index = HitColor.R | HitColor.G << 8 | HitColor.B << 16;

	if(Index >= 0 && Index < AllHitData.Num() - (INT)sizeof(HHitProxy))
	{
		FHitProxyInfo* Info = reinterpret_cast<FHitProxyInfo*>(&AllHitData[Index]);
		HHitProxy* HitProxy = reinterpret_cast<HHitProxy*>(&AllHitData[Index + sizeof(FHitProxyInfo)]);
		AActor* HitActor = HitProxy->GetActor();

		*OutIndex = Index;

		// Checking for preferred selection types (the ones that are harder to hit, like brushes)
		if(Info->IsPreferred || (HitActor && (HitActor->DrawType == DT_Brush || HitActor->DrawType == DT_AntiPortal)))
			return true;
	}

	return false;
}

void FSelectionRenderInterface::ProcessHit(INT HitProxyIndex)
{
	if(HitProxyIndex >= 0 && HitProxyIndex < AllHitData.Num() - (INT)sizeof(HHitProxy))
	{
		INT ParentIndices[32];
		INT NumParents = 0;

		// Collecting all parents of the successful hit
		for(INT ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitProxyIndex])->ParentIndex;
			ParentIndex != INDEX_NONE;
			ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[ParentIndex])->ParentIndex)
		{

			ParentIndices[NumParents] = ParentIndex;
			++NumParents;
			checkSlow(NumParents <= ARRAY_COUNT(ParentIndices));
		}

		INT HitCount = 0;

		// Copying hit hierarchy to HitData
		for(INT i = 0; i < NumParents; ++i)
		{
			HHitProxy* Parent = reinterpret_cast<HHitProxy*>(&AllHitData[ParentIndices[i] + sizeof(FHitProxyInfo)]);

			appMemcpy(HitData + HitCount, Parent, Parent->Size);

			HitCount += Parent->Size;
		}

		HHitProxy* Hit = reinterpret_cast<HHitProxy*>(&AllHitData[HitProxyIndex + sizeof(FHitProxyInfo)]);

		appMemcpy(HitData + HitCount, Hit, Hit->Size);
		*HitSize = HitCount + Hit->Size;
	}

	HitStack.Empty();
	AllHitData.Empty();
	HitData = NULL;
}

void FSelectionRenderInterface::PushHit(const BYTE* Data, INT Count)
{
	const TCHAR* Name = reinterpret_cast<const HHitProxy*>(Data)->GetName();
	UBOOL IsPreferredSelection = appStricmp(Name, "HGizmoAxis") == 0 ||
	                             appStricmp(Name, "HBrushVertex") == 0 ||
	                             appStricmp(Name, "HActorVertex") == 0 ||
	                             appStricmp(Name, "HGlobalPivot") == 0;
	FHitProxyInfo Info(HitStack.Num() > 0 ? HitStack.Last() : INDEX_NONE, IsPreferredSelection);
	INT HitDataIndex = AllHitData.Add(sizeof(FHitProxyInfo) + Count, false);

	appMemcpy(&AllHitData[HitDataIndex], &Info, sizeof(FHitProxyInfo));
	appMemcpy(&AllHitData[HitDataIndex + sizeof(FHitProxyInfo)], Data, Count);
	HitStack.AddItem(HitDataIndex);
}

void FSelectionRenderInterface::PopHit(INT Count, UBOOL Force)
{
	HitStack.Pop();
}

void FSelectionRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses)
{
	CurrentTexture = NULL;
	Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);

	// Checking whether the current material has a texture with an alpha channel somewhere down the hierarchy.
	// If it has, it is used to draw the selection buffer for more accuracy. If not, the polygons are completly filled.
	while(Material)
	{
		if(Material->IsA<UBitmapMaterial>())
		{
			UTexture* Tmp = Cast<UTexture>(Material);

			if(Tmp && Tmp->bAlphaTexture)
				CurrentTexture = static_cast<UBitmapMaterial*>(Material);
			else
				CurrentTexture = NULL;

			break;
		}
		else if(Material->IsA<UShader>())
		{
			Material = static_cast<UShader*>(Material)->Diffuse;
		}
		else
		{
			Material = NULL;
		}
	}
}

void FSelectionRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex)
{
	UHardwareShader* Shader;

	DECLARE_STATIC_UOBJECT(UHardwareShader, SolidSelectionShader, {
		// Initialize shader used for selection in the editor
		SolidSelectionShader->VertexShaderText = "vs.1.1\n"
		                                         "m4x4 r0, v0, c0\n"
		                                         "mov oPos, r0\n";
		SolidSelectionShader->PixelShaderText = "ps.1.1\n"
		                                        "mov r0, c0\n";
		SolidSelectionShader->VSConstants[0].Type = EVC_ObjectToScreenMatrix;
		SolidSelectionShader->PSConstants[0].Type = EVC_MaterialDefined;
		SolidSelectionShader->ZTest = 1;
		SolidSelectionShader->ZWrite = 1;
	});
	DECLARE_STATIC_UOBJECT(UHardwareShader, AlphaSelectionShader, {
		// Initialize shader used for selection of objects with alpha channel in the editor
		AlphaSelectionShader->VertexShaderText = "vs.1.1\n"
		                                         "m4x4 r0, v0, c0\n"
		                                         "mov oPos, r0\n"
		                                         "mov oT0, v1\n";
		AlphaSelectionShader->PixelShaderText = "ps.1.1\n"
		                                        "tex t0\n"
		                                        "mov r0, c0\n"
		                                        "mad r0, t0, c1, r0\n";
		AlphaSelectionShader->StreamMapping.AddItem(FVF_Position);
		AlphaSelectionShader->StreamMapping.AddItem(FVF_TexCoord0);
		AlphaSelectionShader->VSConstants[0].Type = EVC_ObjectToScreenMatrix;
		AlphaSelectionShader->PSConstants[0].Type = EVC_MaterialDefined;
		AlphaSelectionShader->PSConstants[1].Type = EVC_MaterialDefined;
		AlphaSelectionShader->PSConstants[1].Value = FPlane(0.0f, 0.0f, 0.0f, 1.0f);
		AlphaSelectionShader->ZTest = 1;
		AlphaSelectionShader->ZWrite = 1;
		AlphaSelectionShader->AlphaTest = 1;
	});

	if(HitStack.Num() > 0)
	{
		INT            HitDataIndex = HitStack.Last();
		FHitProxyInfo* Info         = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndex]);
		HHitProxy*     HitProxy     = reinterpret_cast<HHitProxy*>(Info + 1);
		AActor*        HitActor     = HitProxy->GetActor();

		// Sprites are drawn with alpha regardless of whether UTexture::bAlphaTexture is set or not so we need to check for it
		if(HitActor && (HitActor->DrawType == DT_Sprite || HitActor->DrawType == DT_Particle))
		{
			Shader = AlphaSelectionShader;
			Shader->Textures[0] = Cast<UBitmapMaterial>(HitActor->Texture);
		}
		else
		{
			const bool IsTextureBrowserMaterial = appStricmp(HitProxy->GetName(), "HBrowserMaterial") == 0;

			// Alpha is ignored in the texture browser since there it should be possible to click anywhere on a texture to select it
			if(CurrentTexture && !IsTextureBrowserMaterial)
			{
				Shader = AlphaSelectionShader;
				Shader->Textures[0] = CurrentTexture;
			}
			else
			{
				/*
				 * There's a bug in the texture browser where clicking on the name of a material will not select the material under the cursor but the one after it.
				 * Since it can't be fixed at the source (UnrealEd.exe) here's a workaround that checks if a material name is rendered in which case the previous
				 * hit data index is used which causes the correct material to be selected.
				 */
				if(IsTextureBrowserMaterial)
				{
					static INT PrevHitDataIndex = 0;

					// NumPrimitives > 2 means the material name is being drawn. Everything else in the texture browser is drawn as a single quad (2 tris)
					if(NumPrimitives > 2)
						HitDataIndex = PrevHitDataIndex; // use the previous index for the current selection
					else
						PrevHitDataIndex = HitDataIndex;
				}

				Shader = SolidSelectionShader;
				Shader->ZTest = !(HitActor && (HitActor->DrawType == DT_Brush || HitActor->DrawType == DT_AntiPortal)); // Disable ZTest for brushes since they are rendered on top of everything else
			}
		}

		// Convert index to shader color

		checkSlow(RenDev->LockedViewport->ColorBytes == 3 || RenDev->LockedViewport->ColorBytes == 4);

		Shader->PSConstants[0].Value.X = static_cast<BYTE>(HitDataIndex) / 255.0f;
		Shader->PSConstants[0].Value.Y = static_cast<BYTE>(HitDataIndex >> 8) / 255.0f;
		Shader->PSConstants[0].Value.Z = static_cast<BYTE>(HitDataIndex >> 16) / 255.0f;
	}
	else
	{
		Shader = SolidSelectionShader;
		Shader->PSConstants[0].Value = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
	}

	Impl->SetHardwareShaderMaterial(Shader, NULL, NULL);
	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
}

/*
 * ModRenderDevice
 */

UBOOL UModRenderDevice::Init()
{
	// Initialize fixes. This might not be the best place but idk where else to put it...
	InitSWRCFix();
	bEnableSelectionFix = USWRCFix::Instance->EnableEditorSelectionFix;

	UBOOL Result = Super::Init();

	if(Result)
		MaybePatchVTable(&D3D8CreateDevice, Direct3D8, D3DVTIdx_D3D8CreateDevice, D3D8CreateDeviceOverride);

	return Result;
}

UBOOL UModRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if(GIsEditor)
	{
		if(ParseCommand(&Cmd, "DEBUGSELECT"))
		{
			bDebugSelectionBuffer = !bDebugSelectionBuffer;

			return 1;
		}
		else if(ParseCommand(&Cmd, "FIXSELECT"))
		{
			bEnableSelectionFix = !bEnableSelectionFix;

			debugf("Selection fix %s", bEnableSelectionFix ? "enabled" : "disabled");

			return 1;
		}
	}

	return Super::Exec(Cmd, Ar);
}

FRenderInterface* UModRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize)
{
	if(!USWRCFix::RenderingReady) // Return NULL (meaning no rendering) until ready to avoid caching all textures at startup, causing a high memory load
		return NULL;

	FRenderInterface* RI = Super::Lock(Viewport, HitData, HitSize);

	if(bEnableSelectionFix && GIsEditor && RI && HitData && CastChecked<UEditorEngine>(GEngine)->Mode != EM_EyeDropper)
	{
		LockedViewport = Viewport;
		RI->EnableFog(0); // No fog in the selection buffer or else there will be wrong color values.
		SelectionRI.Impl = RI;
		SelectionRI.HitData = HitData;
		SelectionRI.HitSize = HitSize;
		// Storing color values
		C_ActorArrow = GEngine->C_ActorArrow;
		GEngine->C_ActorArrow = FColor(0x00000000);

		return &SelectionRI;
	}

	return RI;
}

void UModRenderDevice::Unlock(FRenderInterface* RI)
{
	if(RI == &SelectionRI)
	{
		checkSlow(LockedViewport);
		checkSlow(SelectionRI.HitData);
		checkSlow(SelectionRI.HitSize);

		IDirect3DSurface8* RenderTarget = NULL;

		D3DDeviceGetRenderTarget(Direct3DDevice8, &RenderTarget);

		checkSlow(RenderTarget != NULL);

		D3DSURFACE_DESC Desc;

		D3DSurfaceGetDesc(RenderTarget, &Desc);

		D3DLOCKED_RECT LockedRect;

		D3DSurfaceLockRect(RenderTarget, &LockedRect, NULL, 0);

		INT HitProxyIndex = INDEX_NONE;

		// The actual hit test location is offset by two pixels so that it is at the center of the cross cursor.
		INT HitX = LockedViewport->HitX + 2;
		INT HitY = LockedViewport->HitY + 2;
		INT PreferredHitProxyIndex = INDEX_NONE;
		FLOAT PreferredHitDist = 999999.0f;
		FLOAT HitDist = 999999.0f;
		DWORD* src = (DWORD*)LockedRect.pBits;
		src = (DWORD*)((BYTE*)src + HitX * 4 + (HitY - LockedViewport->HitYL) * LockedRect.Pitch);

		/*
		 * Hits are checked in a square area to make it easier to select stuff that is only a few pixels in size.
		 * Some hit types are preferred over others. E.g the gizmo axes or wireframe brushes. If there is more than one possible selection, the preferred ones will be used.
		 * If there is no preferred selection type, anything at the exact cursor position is also considered a preferred selection.
		 */
		for(INT Y = -LockedViewport->HitYL; Y < LockedViewport->HitYL + 1; Y++, src = (DWORD*)((BYTE*)src + LockedRect.Pitch))
		{
			for(INT X = -LockedViewport->HitXL; X < LockedViewport->HitXL + 1; X++)
			{
				if(src + X >= LockedRect.pBits)
				{
					FColor HitColor = FColor((src[X] >> 16) & 0xff, (src[X] >> 8) & 0xff, (src[X]) & 0xff);
					INT Index = INDEX_NONE;
					FLOAT Dist = FVector(X, Y, 0.0f).Size2D(); // Distance of the hit from the center of the hit area. The closer it is, the higher the priority over other hits.

					if(SelectionRI.ProcessHitColor(HitColor, &Index))
					{
						if(Dist < PreferredHitDist)
						{
							PreferredHitDist = Dist;
							PreferredHitProxyIndex = Index;
						}
					}
					else if(Index >= 0)
					{
						if(Dist < HitDist)
						{
							HitDist = Dist;
							HitProxyIndex = Index;
						}
					}
				}
			}

		if(PreferredHitProxyIndex >= 0)
			HitProxyIndex = PreferredHitProxyIndex;
		}

		D3DSurfaceUnlockRect(RenderTarget);
		Super::Unlock(SelectionRI.Impl);
		SelectionRI.ProcessHit(HitProxyIndex);

		// Restoring color values
		GEngine->C_ActorArrow = C_ActorArrow;

		if(bDebugSelectionBuffer)
		{
			LockedViewport->Present();
			appSleep(3.0f);
		}
	}
	else
	{
		Super::Unlock(RI);
	}

	LockedViewport = NULL;
}

IMPLEMENT_CLASS(UModRenderDevice)
