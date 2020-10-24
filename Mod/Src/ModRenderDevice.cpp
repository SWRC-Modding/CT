#include "../Inc/Mod.h"
#include "../../Editor/Inc/Editor.h"
#include <Windows.h>

// TODO: Tidy up. This is a mess!

/*
 * Helper function that only patches a vtable if it wasn't done already.
 * This is needed because the editor creates multiple render devices which in turn create new D3D8 devices that need to be patched again.
 * However, we have no way of knowing about that in e.g. a CreateTexture call and thus we have to check each time whether the function at the
 * vtable index is already the override function and if not patch it.
 */
template<typename F>
void MaybePatchVTable(F* OutFunc, void* Object, INT Index, F NewFunc){
	if((*reinterpret_cast<void***>(Object))[Index] == NewFunc){
		checkSlow(OutFunc);

		return;
	}

	*OutFunc = static_cast<F>(PatchVTable(Object, Index, NewFunc));
}

enum{
	// D3D8
	D3DVTableIndex_D3D8CreateDevice    = 15,
	// D3DDevice
	D3DVTableIndex_DeviceCreateTexture = 20,
	// D3DTexture
	D3DVTableIndex_TextureGetLevelDesc = 14,
	D3DVTableIndex_TextureLockRect     = 16,
	D3DVTableIndex_TextureUnlockRect   = 17
};

/*
 * Direct3D types and Functions which are used to patch the d3d object vtables.
 * We could simply include d3d8.h and inherit from the d3d interfaces but this way it is simpler and doesn't add any external dependencies.
 */

typedef void FD3D8;
typedef void FD3DDevice;
typedef void FD3DTexture;

enum ED3DFormat{
	D3DFormat_A8R8G8B8 = 21, // Fallback if none of the bumpmap formats are available.
	D3DFormat_V8U8     = 60,
	D3DFormat_L6V5U5   = 61,
	D3DFormat_X8L8V8U8 = 62
};

bool IsBumpmapFormat(ED3DFormat Format){
	return Format >= D3DFormat_V8U8 && Format <= D3DFormat_X8L8V8U8;
}

struct FD3DLockedRect{
	INT   Pitch;
	void* pBits;
};

/*
 * L6V5U5 to X8L8V8U8 texture format conversion.
 *
 * L6V5U5 is not supported on newer hardware and either makes the game and editor crash or the driver converts it directly to X8L8V8U8 which results in visual artifacts.
 * That's why we detect if a texture is created with the L6V5U5 format and perform a proper conversion to X8L8V8U8 or if that is not available V8U8 and if not even that one works ARGB.
 */

// Information about the current texture returned by CreateTexture and used by Lock/UnlockRect
static FD3DTexture* CurrentTexture;
static ED3DFormat   CurrentTextureSourceFormat;
static ED3DFormat   CurrentTextureTargetFormat;
static UINT         CurrentTextureNumMipLevels;
static UINT         CurrentMipLevelWidth;
static UINT         CurrentMipLevelHeight;
static void*        CurrentMipLevelPixels;

struct FV8U8Pixel{
	INT8 U;
	INT8 V;
};

struct FL6V5U5Pixel{
	INT16  U:5;
	INT16  V:5;
	UINT16 L:6;
};

struct FX8L8V8U8Pixel{
	INT8  U;
	INT8  V;
	UINT8 L;
	UINT8 X;
};

struct FA8R8G8B8Pixel{
	UINT8 B;
	UINT8 G;
	UINT8 R;
	UINT8 A;
};

static int GetBytesPerPixel(ED3DFormat Format){
	switch(Format){
	case D3DFormat_A8R8G8B8:
		return sizeof(FA8R8G8B8Pixel);
	case D3DFormat_V8U8:
		return sizeof(FV8U8Pixel);
	case D3DFormat_L6V5U5:
		return sizeof(FL6V5U5Pixel);
	case D3DFormat_X8L8V8U8:
		return sizeof(FX8L8V8U8Pixel);
	}

	appErrorf("Unknown texture format");

	return 0;
}

/*
 * Integer range mapping
 */

static UINT8 Map6BitUnsignedTo8BitUnsigned(UINT8 U6){
	return (UINT8)(U6 * 255 / 63);
}

static INT8 Map5BitSignedTo8BitSigned(INT8 S5){
	const int Min5 = -16;
	const int Max5 = 15;
	const int Range5 = Max5 - Min5;

	const int Min8 = -128;
	const int Max8 = 127;
	const int Range8 = Max8 - Min8;

	return (INT8)((S5 - Min5) * Range8 / Range5 + Min8);
}

static UINT8 Map8BitSignedTo8BitUnsigned(INT8 S8){
	return S8 + 128;
}

/*
 * V8U8 format conversion
 */

static void ConvertV8U8ToA8R8G8B8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FV8U8Pixel* P1 = static_cast<const FV8U8Pixel*>(In) + Index;
			FA8R8G8B8Pixel* P2 = static_cast<FA8R8G8B8Pixel*>(Out) + Index;

			P2->B = Map8BitSignedTo8BitUnsigned(P1->V);
			P2->G = Map8BitSignedTo8BitUnsigned(P1->U);
			P2->R = 0xFF;
			P2->A = 0xFF;
		}
	}
}

/*
 * L6V5U5 format conversion
 */

static void ConvertL6V5U5ToX8L8V8U8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(In) + Index;
			FX8L8V8U8Pixel* P2 = static_cast<FX8L8V8U8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			P2->L = Map6BitUnsignedTo8BitUnsigned(P1->L);
			P2->X = P2->L; // L6V5U5 only has one luminance value so we can just reuse it
		}
	}
}

static void ConvertL6V5U5ToV8U8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(In) + Index;
			FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			// No luminance
		}
	}
}

static void ConvertL6V5U5ToA8R8G8B8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(In) + Index;
			FA8R8G8B8Pixel* P2 = static_cast<FA8R8G8B8Pixel*>(Out) + Index;

			P2->B = Map8BitSignedTo8BitUnsigned(Map5BitSignedTo8BitSigned(P1->V));
			P2->G = Map8BitSignedTo8BitUnsigned(Map5BitSignedTo8BitSigned(P1->U));
			P2->R = Map6BitUnsignedTo8BitUnsigned(P1->L);
			P2->A = P2->R;
		}
	}
}

/*
 * X8L8V8U8 format conversion
 */

static void ConvertX8L8V8U8ToV8U8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FX8L8V8U8Pixel* P1 = static_cast<const FX8L8V8U8Pixel*>(In) + Index;
			FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			// No luminance
		}
	}
}

static void ConvertX8L8V8U8ToA8R8G8B8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FX8L8V8U8Pixel* P1 = static_cast<const FX8L8V8U8Pixel*>(In) + Index;
			FA8R8G8B8Pixel* P2 = static_cast<FA8R8G8B8Pixel*>(Out) + Index;

			P2->B = Map8BitSignedTo8BitUnsigned(P1->V);
			P2->G = Map8BitSignedTo8BitUnsigned(P1->U);
			P2->R = P1->L;
			P2->A = P1->X;
		}
	}
}

/*
 * GetLevelDesc
 *
 * Not overridden but used to get information about the size of the different mip levels.
 */

struct FD3DSurfaceDesc{
	ED3DFormat               Format;
	enum ED3DResourceType    Type;
	DWORD                    Usage;
	enum ED3DPool            Pool;
	UINT                     Size;
	enum ED3DMultiSampleType MultiSampleType;
	UINT                     Width;
	UINT                     Height;
};

typedef HRESULT(__stdcall*D3DTextureGetLevelDescFunc)(FD3DTexture*, UINT, FD3DSurfaceDesc*);

D3DTextureGetLevelDescFunc D3DTextureGetLevelDesc = NULL;

/*
 * LockRect
 */

typedef HRESULT(__stdcall*D3DTextureLockRectFunc)(FD3DTexture*, UINT, FD3DLockedRect*, const RECT*, DWORD);

D3DTextureLockRectFunc D3DTextureLockRect = NULL;

static HRESULT __stdcall D3DTextureLockRectOverride(FD3DTexture* D3DTexture, UINT Level, FD3DLockedRect* pLockedRect, const RECT* pRect, DWORD Flags){
	if(D3DTexture != CurrentTexture)
		return D3DTextureLockRect(D3DTexture, Level, pLockedRect, pRect, Flags);

	FD3DSurfaceDesc SurfaceDesc;
	HRESULT GetLevelDescResult = D3DTextureGetLevelDesc(D3DTexture, Level, &SurfaceDesc);

	if(FAILED(GetLevelDescResult))
		return GetLevelDescResult;

	CurrentMipLevelWidth = SurfaceDesc.Width;
	CurrentMipLevelHeight = SurfaceDesc.Height;
	CurrentMipLevelPixels = appMalloc(SurfaceDesc.Size);

	pLockedRect->Pitch = CurrentMipLevelWidth * GetBytesPerPixel(CurrentTextureSourceFormat);
	pLockedRect->pBits = CurrentMipLevelPixels;

	return S_OK;
}

/*
 * UnlockRect
 */

typedef HRESULT(__stdcall*D3DTextureUnlockRectFunc)(FD3DTexture*, UINT);

D3DTextureUnlockRectFunc D3DTextureUnlockRect = NULL;

static HRESULT __stdcall D3DTextureUnlockRectOverride(FD3DTexture* D3DTexture, UINT Level){
	if(D3DTexture != CurrentTexture)
		return D3DTextureUnlockRect(D3DTexture, Level);

	FD3DLockedRect LockedRect;
	HRESULT Result = D3DTextureLockRect(D3DTexture, Level, &LockedRect, NULL, 0);

	if(SUCCEEDED(Result)){
		if(CurrentTextureSourceFormat == D3DFormat_V8U8){
			ConvertV8U8ToA8R8G8B8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
		}else if(CurrentTextureSourceFormat == D3DFormat_L6V5U5){
			if(CurrentTextureTargetFormat == D3DFormat_V8U8)
				ConvertL6V5U5ToV8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFormat_X8L8V8U8)
				ConvertL6V5U5ToX8L8V8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFormat_A8R8G8B8)
				ConvertL6V5U5ToA8R8G8B8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
		}else if(CurrentTextureSourceFormat == D3DFormat_X8L8V8U8){
			if(CurrentTextureTargetFormat == D3DFormat_V8U8)
				ConvertX8L8V8U8ToV8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFormat_A8R8G8B8)
				ConvertX8L8V8U8ToA8R8G8B8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
		}

		Result = D3DTextureUnlockRect(D3DTexture, Level);
	}

	appFree(CurrentMipLevelPixels);
	CurrentMipLevelPixels = NULL;

	if(Level == CurrentTextureNumMipLevels - 1) // This is the last mip level which means the current texture is fully converted and should be set to NULL
		CurrentTexture = NULL;

	return Result;
}

/*
 * CreateTexture
 */

typedef HRESULT(__stdcall*D3DDeviceCreateTextureFunc)(FD3DDevice*, UINT, UINT, UINT, DWORD, ED3DFormat, enum ED3DPool, FD3DTexture**);

D3DDeviceCreateTextureFunc D3DDeviceCreateTexture = NULL;

static HRESULT __stdcall D3DDeviceCreateTextureOverride(FD3DDevice* D3DDevice,
														UINT Width,
														UINT Height,
														UINT Levels,
														DWORD Usage,
														ED3DFormat Format,
														enum ED3DPool Pool,
														FD3DTexture** ppTexture){
	// X8L8V8U8 is used as the first fallback format because no information is lost in the conversion
	ED3DFormat FallbackFormat = Format == D3DFormat_L6V5U5 ? D3DFormat_X8L8V8U8 : Format;
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

	if(FAILED(Result) && !IsBumpmapFormat(Format))
		appErrorf("CreateTexture failed (Format: %i)", Format); // Should never happen but this is a better error than the engine produces

	if(FAILED(Result)){ // If X8L8V8U8 is not supported V8U8 might still be so try that. Visually the same except for missing luminance
		FallbackFormat = D3DFormat_V8U8;
		Result = D3DDeviceCreateTexture(D3DDevice,
										Width,
										Height,
										Levels,
										Usage,
										FallbackFormat,
										Pool,
										ppTexture);
	}

	if(FAILED(Result)){ // If no bumpmap format is available we fall back to ARGB. Looks fine visually and should always be supported
		FallbackFormat = D3DFormat_A8R8G8B8;
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

	D3DTextureGetLevelDesc = static_cast<D3DTextureGetLevelDescFunc>((*reinterpret_cast<void***>(*ppTexture))[D3DVTableIndex_TextureGetLevelDesc]);

	MaybePatchVTable(&D3DTextureLockRect, *ppTexture, D3DVTableIndex_TextureLockRect, D3DTextureLockRectOverride);
	MaybePatchVTable(&D3DTextureUnlockRect, *ppTexture, D3DVTableIndex_TextureUnlockRect, D3DTextureUnlockRectOverride);

	// Updating the current texture with the newly created one
	CurrentTexture = *ppTexture;
	CurrentTextureSourceFormat = Format;
	CurrentTextureTargetFormat = FallbackFormat;
	CurrentTextureNumMipLevels = Levels;

	return Result;
}

/*
 * CreateDevice
 */

typedef HRESULT(__stdcall*D3D8CreateDeviceFunc)(FD3D8* D3D8,
												UINT Adapter,
												enum D3DDEVTYPE DeviceType,
												HWND hFocusWindow,
												DWORD BehaviorFlags,
												struct D3DPRESENT_PARAMETERS* pPresentationParameters,
												class IDirect3DDevice8** ppReturnedDeviceInterface);

D3D8CreateDeviceFunc D3D8CreateDevice = NULL;

HRESULT __stdcall D3D8CreateDeviceOverride(FD3D8* D3D8,
										   UINT Adapter,
										   enum D3DDEVTYPE DeviceType,
										   HWND hFocusWindow,
										   DWORD BehaviorFlags,
										   struct D3DPRESENT_PARAMETERS* pPresentationParameters,
										   class IDirect3DDevice8** ppReturnedDeviceInterface){
	HRESULT Result = D3D8CreateDevice(D3D8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if(SUCCEEDED(Result))
		MaybePatchVTable(&D3DDeviceCreateTexture, *ppReturnedDeviceInterface, D3DVTableIndex_DeviceCreateTexture, D3DDeviceCreateTextureOverride);

	return Result;
}

/*
 * FModRenderInterface
 */

FModRenderInterface::FModRenderInterface(UModRenderDevice* InRenDev){
	RenDev = InRenDev;

	HitProxyStack.SetNoShrink(true);
	AllHitData.SetNoShrink(true);
	HitDataIndices.SetNoShrink(true);
}

EHitProxy FModRenderInterface::CurrentHitProxyType() const{
	return HitProxyStack.Num() > 0 ? HitProxyStack.Last().Type : HP_Unknown;
}

bool FModRenderInterface::OverrideSelectionForCurrentHitProxy() const{
	EHitProxy Type = CurrentHitProxyType();

	// The following types need to use the original selection mechanism

	return Type != HP_Unknown &&
	       Type != HP_BrowserMaterial &&
	       Type != HP_MaterialTree &&
	       Type != HP_MatineeTimePath &&
	       Type != HP_MatineeScene &&
	       Type != HP_MatineeAction &&
	       Type != HP_MatineeSubAction;
}

bool FModRenderInterface::ProcessHitColor(FColor HitColor, INT* OutIndex){
	INT Index;

	if(RenDev->LockedViewport->ColorBytes == 2)
		Index = (HitColor.R & 0x40) | ((HitColor.G << 6) & 0x40) | HitColor.B << 11;
	else
		Index = HitColor.R | HitColor.G << 8;

	--Index;

	if(Index >= 0 && Index < HitDataIndices.Num()){
		if(*OutIndex < 0)
			*OutIndex = Index;

		FHitProxyInfo* Info = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndices[Index]]);
		AActor*        HitActor = reinterpret_cast<HHitProxy*>(reinterpret_cast<BYTE*>(Info) + sizeof(FHitProxyInfo))->GetActor();

		// Checking for preferred selection types
		if(Info->Type == HP_GizmoAxis ||
		   Info->Type == HP_BrushVertex ||
		   Info->Type == HP_ActorVertex ||
		   Info->Type == HP_GlobalPivot ||
		   (HitActor && (HitActor->IsABrush()))){
			*OutIndex = Index;

			return true;
		}
	}

	return false;
}

void FModRenderInterface::ProcessHit(INT HitProxyIndex){
	INT HitCount = *reinterpret_cast<INT*>(reinterpret_cast<BYTE*>(Impl) + 40944); // Only way to get the hit count of the FD3DRenderInterface

	if(HitProxyIndex > 0 && HitProxyIndex <= HitDataIndices.Num()){
		_WORD ParentIndices[32];
		INT   NumParents = 0;

		// Collecting all parents of the successful hit
		for(SWORD ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndices[HitProxyIndex]])->ParentIndex;
			ParentIndex != INDEX_NONE;
			ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndices[ParentIndex]])->ParentIndex){

			ParentIndices[NumParents] = ParentIndex;
			++NumParents;
			checkSlow(NumParents <= ARRAY_COUNT(ParentIndices));
		}

		// Copying hit hierarchy to HitData
		for(INT i = 0; i < NumParents; ++i){
			HHitProxy* Parent = reinterpret_cast<HHitProxy*>(&AllHitData[HitDataIndices[ParentIndices[i]] + sizeof(FHitProxyInfo)]);

			appMemcpy(HitData + HitCount, Parent, Parent->Size);

			HitCount += Parent->Size;
		}

		HHitProxy* Hit = reinterpret_cast<HHitProxy*>(&AllHitData[HitDataIndices[HitProxyIndex] + sizeof(FHitProxyInfo)]);

		appMemcpy(HitData + HitCount, Hit, Hit->Size);
		*HitSize = HitCount + Hit->Size;
	}else{
		Cast<UEditorEngine>(GEngine)->Exec_Select("NONE", *GLog);
	}

	HitProxyStack.Empty();
	AllHitData.Empty();
	HitDataIndices.Empty();
	HitCount = 0;
	HitData = NULL;
}

void FModRenderInterface::PushHit(const BYTE* Data, INT Count){
	checkSlow(GIsEditor);
	checkSlow(UModRenderDevice::SelectionShader);
	checkSlow(HitData);
	checkSlow(HitSize);

	const TCHAR* Name = reinterpret_cast<const HHitProxy*>(Data)->GetName();

	EHitProxy HitType;

	if(appStricmp(Name, "HBspSurf") == 0){
		HitType = HP_BspSurf;
	}else if(appStricmp(Name, "HActor") == 0){
		HitType = HP_Actor;
	}else if(appStricmp(Name, "HBrushVertex") == 0){
		HitType = HP_BrushVertex;
	}else if(appStricmp(Name, "HCoords") == 0){
		HitType = HP_Coords;
	}else if(appStricmp(Name, "HTerrain") == 0){
		HitType = HP_Terrain;
	}else if(appStricmp(Name, "HTerrainToolLayer") == 0){
		HitType = HP_TerrainToolLayer;
	}else if(appStricmp(Name, "HMatineeTimePath") == 0){
		HitType = HP_MatineeTimePath;
	}else if(appStricmp(Name, "HMatineeScene") == 0){
		HitType = HP_MatineeScene;
	}else if(appStricmp(Name, "HMatineeAction") == 0){
		HitType = HP_MatineeAction;
	}else if(appStricmp(Name, "HMatineeSubAction") == 0){
		HitType = HP_MatineeSubAction;
	}else if(appStricmp(Name, "HMaterialTree") == 0){
		HitType = HP_MaterialTree;
	}else if(appStricmp(Name, "HGizmoAxis") == 0){
		HitType = HP_GizmoAxis;
	}else if(appStricmp(Name, "HActorVertex") == 0){
		HitType = HP_ActorVertex;
	}else if(appStricmp(Name, "HBezierControlPoint") == 0){
		HitType = HP_BezierControlPoint;
	}else if(appStricmp(Name, "HTextureView") == 0){
		HitType = HP_TextureView;
	}else if(appStricmp(Name, "HGlobalPivot") == 0){
		HitType = HP_GlobalPivot;
	}else if(appStricmp(Name, "HBrowserMaterial") == 0){
		HitType = HP_BrowserMaterial;
	}else if(appStricmp(Name, "HBackdrop") == 0){
		HitType = HP_Backdrop;
	}else{
		HitType = HP_Unknown;
	}

	_WORD HitDataIndex = AllHitData.Add(sizeof(FHitProxyInfo) + Count, false);
	_WORD Index        = HitDataIndices.AddItem(HitDataIndex);
	FHitProxyInfo Info(HitProxyStack.Num() > 0 ? HitProxyStack.Last().Index : INDEX_NONE, HitType);

	appMemcpy(&AllHitData[HitDataIndex], &Info, sizeof(FHitProxyInfo));
	appMemcpy(&AllHitData[HitDataIndex + sizeof(FHitProxyInfo)], Data, Count);
	HitProxyStack.AddItem(FHitProxyStackEntry(Index, HitType));

	if(!OverrideSelectionForCurrentHitProxy())
		Impl->PushHit(Data, Count);
}

void FModRenderInterface::PopHit(INT Count, UBOOL Force){
	if(!OverrideSelectionForCurrentHitProxy())
		Impl->PopHit(Count, Force);

	HitProxyStack.Pop();
}

void FModRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){
	if(OverrideSelectionForCurrentHitProxy()){
		FPlane ShaderColor(0.0f, 0.0f, 0.0f, 0.0f);
		_WORD HitDataIndex = HitProxyStack.Last().Index + 1; // Adding 1 because 0 means no hit. This is subtracted again later

		if(RenDev->LockedViewport->ColorBytes == 2){
			checkSlow(HitDataIndex < 32768);

			ShaderColor.X = (HitDataIndex & 0x1F) / 31.0f;
			ShaderColor.Y = ((HitDataIndex >> 5) & 0x1F) / 31.0f;
			ShaderColor.Z = ((HitDataIndex >> 10) & 0x1F) / 31.0f;
		}else{
			checkSlow(RenDev->LockedViewport->ColorBytes == 3 || RenDev->LockedViewport->ColorBytes == 4);

			ShaderColor.X = LOBYTE(HitDataIndex) / 255.0f;
			ShaderColor.Y = HIBYTE(HitDataIndex) / 255.0f;
		}

		FHitProxyInfo* Info = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndices[HitDataIndex - 1]]);
		AActor*        HitActor = reinterpret_cast<HHitProxy*>(reinterpret_cast<BYTE*>(Info) + sizeof(FHitProxyInfo))->GetActor();

		UModRenderDevice::SelectionShader->ZTest = !(HitActor && HitActor->IsABrush()); // Disable ZTest for brushes since they are rendered on top of everything else
		UModRenderDevice::SelectionShader->PSConstants[0].Value = ShaderColor;

		SetHardwareShaderMaterial(UModRenderDevice::SelectionShader, NULL, NULL);
	}

	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
}

/*
 * ModRenderDevice
 */

FLOAT(__fastcall*UEngineGetMaxTickRate)(UEngine*, DWORD) = NULL;

static FLOAT __fastcall EngineGetMaxTickRateOverride(UEngine* Self, DWORD Edx){
	FLOAT MaxTickRate = UEngineGetMaxTickRate(Self, Edx);

	return MaxTickRate <= 0.0f ? UModRenderDevice::FpsLimit : MaxTickRate; // If the engine doesn't set it's own tick rate (i.e. GetMaxTickRate returns 0), we use FpsLimit instead
}

UBOOL UModRenderDevice::Init(){
	// Setting override function for maximum tick rate since the original always returns 0
	MaybePatchVTable(&UEngineGetMaxTickRate, GEngine, 49, EngineGetMaxTickRateOverride);
	GConfig->GetFloat("Engine.GameEngine", "FpsLimit", FpsLimit);

	UBOOL Result = Super::Init();

	if(Result)
		MaybePatchVTable(&D3D8CreateDevice, Direct3D8, D3DVTableIndex_D3D8CreateDevice, D3D8CreateDeviceOverride);

	if(Result && !GIsEditor){
		// Get a list of all supported resolutions and apply them to the config

		DEVMODE dm = {0};

		dm.dmSize = sizeof(dm);

		TArray<DWORD> AvailableResolutions;

		for(int i = 0; EnumDisplaySettings(NULL, i, &dm) != 0; ++i)
			AvailableResolutions.AddUniqueItem(MAKELONG(dm.dmPelsWidth, dm.dmPelsHeight));

		if(AvailableResolutions.Num() > 0){
			Sort(AvailableResolutions.GetData(), AvailableResolutions.Num());

			FString ResolutionList = "(";

			for(int i = 0; i < AvailableResolutions.Num() - 1; ++i)
				ResolutionList += FString::Printf("\"%ix%i\",", LOWORD(AvailableResolutions[i]), HIWORD(AvailableResolutions[i]));

			ResolutionList += FString::Printf("\"%ix%i\")", LOWORD(AvailableResolutions.Last()), HIWORD(AvailableResolutions.Last()));

			GConfig->SetString("CTGraphicsOptionsPCMenu",
							   "Options[2].Items",
							   *ResolutionList,
							   *(FString("XInterfaceCTMenus.") + UObject::GetLanguage()));
		}

		// Create FOVChanger object. This might not be the best place but idk where else to put it...
		if(!FOVChanger){
			FOVChanger = ConstructObject<UObject>(LoadClass<UObject>(NULL, "Mod.FOVChanger", NULL, LOAD_NoFail | LOAD_Throw, NULL),
												  reinterpret_cast<UObject*>(-1),
												  FName("MainFOVChanger"));
			FOVChanger->AddToRoot(); // This object should never be garbage collected
			FOVChanger->ProcessEvent(NAME_Init, NULL);
		}
	}else if(!SelectionShader){
		// Initialize shader used for selection in the editor
		SelectionShader = new UHardwareShader();

		SelectionShader->VertexShaderText = "vs.1.1\n"
											"m4x4 r0, v0, c0\n"
		                                    "mov oPos, r0\n";
		SelectionShader->PixelShaderText = "ps.1.1\n"
		                                   "mov r0,c0\n";
		SelectionShader->VSConstants[0].Type = EVC_ObjectToScreenMatrix;
		SelectionShader->PSConstants[0].Type = EVC_MaterialDefined;
		SelectionShader->ZTest = 1;
		SelectionShader->ZWrite = 1;
	}

	return Result;
}

UBOOL UModRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar){
	if(GIsEditor){
		if(ParseCommand(&Cmd, "DEBUGSELECT")){
			bDebugSelectionBuffer = !bDebugSelectionBuffer;

			return 1;
		}else if(ParseCommand(&Cmd, "FIXSELECT")){
			bEnableSelectionFix = !bEnableSelectionFix;

			debugf("Selection fix %s", bEnableSelectionFix ? "enabled" : "disabled");

			return 1;
		}
	}else{
		if(ParseCommand(&Cmd, "SETFOV")){
			struct{
				APlayerController* Player;
				FLOAT FOV;
			} Params;

			TObjectIterator<UViewport> It;

			checkSlow(It);

			Params.Player = It->Actor;
			Params.FOV = appAtof(Cmd);

			Ar.Logf("Setting field of view to %f", Params.FOV);
			FOVChanger->ProcessEvent(FName("SetFOV"), &Params);

			return 1;
		}else if(ParseCommand(&Cmd, "SETFPSLIMIT")){
			if(appStrlen(Cmd) > 0){
				FpsLimit = Max(0.0f, appAtof(Cmd));
				GConfig->SetFloat("Engine.GameEngine", "FpsLimit", FpsLimit);
			}

			return 1;
		}else if(ParseCommand(&Cmd, "GETFPSLIMIT")){
			Ar.Logf("%f", FpsLimit);

			return 1;
		}
	}

	return Super::Exec(Cmd, Ar);
}

FRenderInterface* UModRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	FRenderInterface* RI = Super::Lock(Viewport, HitData, HitSize);

	LockedViewport = Viewport;

	if(bEnableSelectionFix && GIsEditor && RI && HitData){
		RenderInterface.Impl = RI;
		RenderInterface.HitData = HitData;
		RenderInterface.HitSize = HitSize;
		RenderInterface.HitCount = 0;

		return &RenderInterface;
	}

	return RI;
}

void UModRenderDevice::Unlock(FRenderInterface* RI){
	if(RI == &RenderInterface){
		checkSlow(RenderInterface.HitData);
		checkSlow(RenderInterface.HitSize);

		void* RenderTarget = NULL; // IDirect3DSurface8

		 // Calling IDirect3DDevice8::GetRenderTarget
		static_cast<HRESULT(__stdcall*)(void*, void**)>((*reinterpret_cast<void***>(Direct3DDevice8))[32])(Direct3DDevice8, &RenderTarget);

		checkSlow(RenderTarget != NULL);

		FD3DSurfaceDesc Desc;

		// Calling IDirect3dSurface8::GetDesc
		static_cast<HRESULT(__stdcall*)(void*, FD3DSurfaceDesc*)>((*reinterpret_cast<void***>(RenderTarget))[8])(RenderTarget, &Desc);

		FD3DLockedRect LockedRect;

		// Calling IDirect3DSurface8::LockRect
		static_cast<HRESULT(__stdcall*)(void*, FD3DLockedRect*, void*, DWORD)>((*reinterpret_cast<void***>(RenderTarget))[9])(RenderTarget, &LockedRect, NULL, 0);

		INT  HitProxyIndex = INDEX_NONE;

		switch(LockedViewport->ColorBytes){
		case 2:
			{
				_WORD* src = (_WORD*)LockedRect.pBits;
				src = (_WORD*)((BYTE*)src + LockedViewport->HitX * 2 + LockedViewport->HitY * LockedRect.Pitch);

				for(INT Y = 0; Y < LockedViewport->HitYL; Y++, src = (_WORD*)((BYTE*)src + LockedRect.Pitch)){
					for(INT X = 0; X < LockedViewport->HitXL; X++){
						if(src[X] != 0x0){
							FColor HitColor = FColor((src[X] >> 11) << 3, ((src[X] >> 6) & 0x3f) << 2, (src[X] & 0x1f) << 3);

							if(RenderInterface.ProcessHitColor(HitColor, &HitProxyIndex))
								goto end_pixel_check;
						}
					}
				}

				break;
			}
		case 3:
			{
				BYTE* src = (BYTE*)LockedRect.pBits;
				src = src + LockedViewport->HitX * 3  + LockedViewport->HitY * LockedRect.Pitch;

				for(INT Y = 0; Y < LockedViewport->HitYL; Y++, src += LockedRect.Pitch){
					for(INT X = 0; X < LockedViewport->HitXL; X++){
						if(*((DWORD*)&src[X * 3]) != 0x0){
							FColor HitColor = FColor(src[X * 3] + 2, src[X * 3] + 1, src[X * 3]);

							if(RenderInterface.ProcessHitColor(HitColor, &HitProxyIndex))
								goto end_pixel_check;
						}
					}
				}

				break;
			}
		case 4:
			{
				DWORD* src = (DWORD*)LockedRect.pBits;
				src = (DWORD*)((BYTE*)src + LockedViewport->HitX * 4 + LockedViewport->HitY * LockedRect.Pitch);

				for(INT Y = -LockedViewport->HitYL; Y < LockedViewport->HitYL; Y++, src = (DWORD*)((BYTE*)src + LockedRect.Pitch)){
					for(INT X = -LockedViewport->HitXL; X < LockedViewport->HitXL; X++){
						if(src[X] != 0x0){
							FColor HitColor = FColor((src[X] >> 16) & 0xff, (src[X] >> 8) & 0xff, (src[X]) & 0xff);

							if(RenderInterface.ProcessHitColor(HitColor, &HitProxyIndex))
								goto end_pixel_check;
						}
					}
				}

				break;
			}
		}

end_pixel_check:

		// Calling IDirect3DSurface8::UnlockRect
		static_cast<HRESULT(__stdcall*)(void*)>((*reinterpret_cast<void***>(RenderTarget))[10])(RenderTarget);

		Super::Unlock(RenderInterface.Impl);
		RenderInterface.ProcessHit(HitProxyIndex);

		if(bDebugSelectionBuffer){
			LockedViewport->Present();
			appSleep(5.0f);
		}
	}else{
		Super::Unlock(RI);
	}

	LockedViewport = NULL;
}

UObject*         UModRenderDevice::FOVChanger = NULL;
FLOAT            UModRenderDevice::FpsLimit   = 0.0f;
UHardwareShader* UModRenderDevice::SelectionShader = NULL;

IMPLEMENT_CLASS(UModRenderDevice)
