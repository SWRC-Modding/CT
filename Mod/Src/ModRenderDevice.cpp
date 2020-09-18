#include "../../D3DDrv/Inc/D3DDrv.h"
#include "../Inc/Mod.h"
#include <Windows.h>

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
 * ModRenderDevice
 */

class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	static UObject* FOVChanger;

	virtual UBOOL Init(){
		UBOOL Result = Super::Init();

		if(Result)
			MaybePatchVTable(&D3D8CreateDevice, Direct3D8, D3DVTableIndex_D3D8CreateDevice, D3D8CreateDeviceOverride);

		if(!GIsEditor){
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
				checkSlow(FOVChanger);
				FOVChanger->AddToRoot(); // This object should never be garbage collected
				FOVChanger->ProcessEvent(NAME_Init, NULL);
			}
		}

		return Result;
	}

	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		if(!GIsEditor && ParseCommand(&Cmd, "SETFOV")){
			float FOV = appAtof(Cmd);

			Ar.Logf("Setting field of view to %f", FOV);
			FOVChanger->ProcessEvent(FName("SetFOV"), &FOV);

			return 1;
		}

		return Super::Exec(Cmd, Ar);
	}
};

UObject* UModRenderDevice::FOVChanger = NULL;

IMPLEMENT_CLASS(UModRenderDevice)
