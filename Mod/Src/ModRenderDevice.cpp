#include "../../D3DDrv/Inc/D3DDrv.h"
#include "../Inc/Mod.h"
#include <Windows.h>

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
	D3DFormat_L6V5U5   = 61,
	D3DFormat_X8L8U8V8 = 62
};

struct FD3DLockedRect{
	INT   Pitch;
	void* pBits;
};

/*
 * L6V5U5 to X8L8U8V8 texture format conversion.
 *
 * L6V5U5 is not supported on newer hardwareand either makes the game and editor crash or the driver
 * it directly to X8L8U8V8 which results in strange visual artifacts. That's why we detect if a texture is created with the L6V5U5 format and perform
 * a proper conversion to X8L8U8V8.
 */

struct FTextureMipLevel{
	UINT  Width;
	UINT  Height;
	void* Pixels;
};

// Realistically there will always be only one current texture, but just to stay flexible we allow for multiple.
static TArray<FD3DTexture*>              CurrentTextures;
static TArray<INT>                       AvailableTextureIndices;
static TArray<TArray<FTextureMipLevel> > MipLevelsByTexture;

struct FL6U5V5Pixel{
	short          V:5;
	short          U:5;
	unsigned short L:6;
};

struct FX8L8U8V8Pixel{
	INT8  V;
	INT8  U;
	UINT8 L;
	UINT8 X;
};

static UINT8 Map6BitUnsignedTo8BitUnsigned(UINT8 U6){
	return (UINT8)(U6 * 255 / 63);
}

static INT8 Map5BitSignedTo8BitSigned(INT8 S5){
	const int min5 = -16;
	const int max5 = 15;
	const int range5 = max5 - min5;

	const int min8 = -128;
	const int max8 = 127;
	const int range8 = max8 - min8;

	return (INT8)((S5 - min5) * range8 / range5 + min8);
}

static void ConvertL6U5V5ToX8L8U8V8(const void* In, void* Out, UINT Width, UINT Height){
	for(UINT Y = 0; Y < Height; ++Y){
		for(UINT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FL6U5V5Pixel* P1 = static_cast<const FL6U5V5Pixel*>(In) + Index;
			FX8L8U8V8Pixel* P2 = static_cast<FX8L8U8V8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			P2->L = Map6BitUnsignedTo8BitUnsigned(P1->L);
			P2->X = P2->L; // L6V5U5 only has one luminance value so we can just reuse it
		}
	}
}

/*
 * GetLevelDesc
 *
 * Not overridden but used to get information about the size of the different mip levels.
 */

struct FD3DSurfaceDesc {
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
	check(D3DTextureLockRect);
	check(D3DTextureGetLevelDesc);

	INT Index = CurrentTextures.FindItemIndex(D3DTexture);

	if(Index != INDEX_NONE){
		FD3DSurfaceDesc SurfaceDesc;

		if(SUCCEEDED(D3DTextureGetLevelDesc(D3DTexture, Level, &SurfaceDesc))){
			TArray<FTextureMipLevel>& MipLevels = MipLevelsByTexture[Index];
			FTextureMipLevel& MipLevel = MipLevels[Level];

			MipLevel.Width = SurfaceDesc.Width;
			MipLevel.Height = SurfaceDesc.Height;
			MipLevel.Pixels = appMalloc(SurfaceDesc.Size);

			pLockedRect->Pitch = MipLevel.Width * sizeof(FL6U5V5Pixel);
			pLockedRect->pBits = MipLevel.Pixels;

			return S_OK;
		}
	}

	return D3DTextureLockRect(D3DTexture, Level, pLockedRect, pRect, Flags);
}

/*
 * UnlockRect
 */

typedef HRESULT(__stdcall*D3DTextureUnlockRectFunc)(FD3DTexture*, UINT);

D3DTextureUnlockRectFunc D3DTextureUnlockRect = NULL;

static HRESULT __stdcall D3DTextureUnlockRectOverride(FD3DTexture* D3DTexture, UINT Level){
	check(D3DTextureUnlockRect);

	INT TextureIndex = CurrentTextures.FindItemIndex(D3DTexture);

	if(TextureIndex != INDEX_NONE){
		check(MipLevelsByTexture.IsValidIndex(TextureIndex));
		check(AvailableTextureIndices.FindItemIndex(TextureIndex) == INDEX_NONE);
		check(MipLevelsByTexture[TextureIndex].IsValidIndex(Level));

		FTextureMipLevel& MipLevel = MipLevelsByTexture[TextureIndex][Level];

		FD3DLockedRect LockedRect;
		HRESULT Result = D3DTextureLockRect(D3DTexture, Level, &LockedRect, NULL, 0);

		if(SUCCEEDED(Result)){
			ConvertL6U5V5ToX8L8U8V8(MipLevel.Pixels, LockedRect.pBits, MipLevel.Width, MipLevel.Height);

			Result = D3DTextureUnlockRect(D3DTexture, Level);
		}

		appFree(MipLevel.Pixels);
		MipLevel.Pixels = NULL;

		if(Level == MipLevelsByTexture[TextureIndex].Num() - 1){
			MipLevelsByTexture[TextureIndex].Empty();

			if(TextureIndex == CurrentTextures.Num() - 1){
				CurrentTextures.Pop();
				MipLevelsByTexture.Pop();
			}else{
				CurrentTextures[TextureIndex] = NULL;
				AvailableTextureIndices.AddItem(TextureIndex);
			}
		}

		return Result;
	}

	return D3DTextureUnlockRect(D3DTexture, Level);
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
	check(D3DDeviceCreateTexture);

	HRESULT Result = D3DDeviceCreateTexture(D3DDevice,
											Width,
											Height,
											Levels,
											Usage,
											Format == D3DFormat_L6V5U5 ? D3DFormat_X8L8U8V8 : Format,
											Pool,
											ppTexture);

	if(SUCCEEDED(Result) && Format == D3DFormat_L6V5U5){
		// Patching vtables if it wasn't done already

		if(!D3DTextureGetLevelDesc)
			D3DTextureGetLevelDesc = static_cast<D3DTextureGetLevelDescFunc>((*reinterpret_cast<void***>(*ppTexture))[D3DVTableIndex_TextureGetLevelDesc]);

		if(!D3DTextureLockRect){
			D3DTextureLockRect = static_cast<D3DTextureLockRectFunc>(PatchVTable(*reinterpret_cast<void***>(*ppTexture),
																				 D3DVTableIndex_TextureLockRect,
																				 D3DTextureLockRectOverride));
		}

		if(!D3DTextureUnlockRect){
			D3DTextureUnlockRect = static_cast<D3DTextureUnlockRectFunc>(PatchVTable(*reinterpret_cast<void***>(*ppTexture),
																					 D3DVTableIndex_TextureUnlockRect,
																					 D3DTextureUnlockRectOverride));
		}

		INT TextureIndex;

		if(AvailableTextureIndices.Num() > 0) {
			TextureIndex = AvailableTextureIndices.Pop();
			CurrentTextures[TextureIndex] = *ppTexture;
		}else{
			TextureIndex = CurrentTextures.AddItem(*ppTexture);
		}

		if(MipLevelsByTexture.Num() <= TextureIndex)
			MipLevelsByTexture.Set(TextureIndex + 1);

		MipLevelsByTexture[CurrentTextures.FindItemIndex(*ppTexture)].Set(Levels);
	}

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
	check(D3D8CreateDevice);

	HRESULT Result = D3D8CreateDevice(D3D8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if(SUCCEEDED(Result)){
		D3DDeviceCreateTextureFunc CreateTextureFunc =
			static_cast<D3DDeviceCreateTextureFunc>(PatchVTable(*reinterpret_cast<void***>(*ppReturnedDeviceInterface),
																						   D3DVTableIndex_DeviceCreateTexture,
																						   D3DDeviceCreateTextureOverride));

		if(CreateTextureFunc != D3DDeviceCreateTextureOverride)
			D3DDeviceCreateTexture = CreateTextureFunc;
	}

	return Result;
}

/*
 * ModRenderDevice
 */

class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	virtual UBOOL Init(){
		UBOOL Result = Super::Init();

		if(Result){
			D3D8CreateDevice = static_cast<D3D8CreateDeviceFunc>(PatchVTable(*reinterpret_cast<void***>(Direct3D8),
																			 D3DVTableIndex_D3D8CreateDevice,
																			 D3D8CreateDeviceOverride));
		}

		return Result;
	}
};

IMPLEMENT_CLASS(UModRenderDevice)
