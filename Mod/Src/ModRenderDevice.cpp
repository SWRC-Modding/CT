#include "../../D3DDrv/Inc/D3DDrv.h"

#include <Windows.h>

/*
 * Patches the given vtable with a custom function and returns the old function
 */
static void* PatchVTable(void** vtable, INT Index, void* Func){
	DWORD OldProtect;

	if(!VirtualProtect(vtable, Index * sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtect)){
		GLog->Logf(NAME_Error, "Unable to patch vtable: VirtualProtect failed with error code %i", GetLastError());

		return NULL;
	}

	void* OldFunc = vtable[Index];

	vtable[Index] = Func;

	return OldFunc;
}

/*
 * Texture that is currently locked. In theory there could be multiple but the way unreal handles this there's always only one
 * so no need for anything fancy.
 */
static struct{
	void* Pixels;
	INT Width;
	INT Height;
} LockedTexture;

/*
 * Functions which are used to patch the d3d object vtables.
 * We could simply inherit from the d3d interfaces but this way it is simpler and doesn't add any external dependencies.
 */

/*
 * LockRect
 */

struct D3DLockedRect{
	INT Pitch;
	void* pBits;
};

typedef HRESULT(__stdcall*D3DLockRectFunc)(void*, UINT, D3DLockedRect*, const RECT*, DWORD);

D3DLockRectFunc D3DLockRect = NULL;

static HRESULT __stdcall LockRectOverride(void* D3DTexture, UINT Level, D3DLockedRect* pLockedRect, const RECT* pRect, DWORD Flags){
	check(D3DLockRect);

	HRESULT Result = D3DLockRect(D3DTexture, Level, pLockedRect, pRect, Flags);

	if(SUCCEEDED(Result) && (LockedTexture.Width + LockedTexture.Height) > 0){
		LockedTexture.Pixels = pLockedRect->pBits;
	}else{
		LockedTexture.Pixels = NULL;
		LockedTexture.Width = 0;
		LockedTexture.Height = 0;
	}

	return Result;
}

/*
 * LockRect
 */

typedef HRESULT(__stdcall*D3DUnlockRectFunc)(void*, UINT);

D3DUnlockRectFunc D3DUnlockRect = NULL;

struct L6U5V5Pixel{
	short V:5;
	short U:5;
	unsigned short L:6;
	//short pad;
};

struct X8L8U8V8Pixel{
	INT8 V;
	INT8 U;
	UINT8 L;
	UINT8 X;
};

UINT8 Map6BitUnsignedTo8BitUnsigned(UINT8 b6){
	return (UINT8)(b6 * 255 / 63);
}

INT8 Map5BitSignedTo8BitSigned(INT8 b5){
	const int min5 = -16;
	const int max5 = 15;
	const int range5 = max5 - min5;

	const int min8 = -128;
	const int max8 = 127;
	const int range8 = max8 - min8;

	return (INT8)((b5 - min5) * range8 / range5 + min8);
}

static HRESULT __stdcall UnlockRectOverride(void* D3DTexture, UINT Level){
	check(D3DUnlockRect);

	// Converting locked texture pixels from L6V5U5 to X8L8U8V8
	if(LockedTexture.Pixels){
		INT BufferSize = LockedTexture.Width * LockedTexture.Height * sizeof(X8L8U8V8Pixel);
		void* TempBuffer = appMalloc(BufferSize);

		appMemcpy(TempBuffer, LockedTexture.Pixels, BufferSize);

		for(INT y = 0; y < LockedTexture.Height; ++y){
			for(INT x = 0; x < LockedTexture.Width; ++x){
				// I have absolutely no idea what the multiplication with 2 does but that's what made it work somehow...
				L6U5V5Pixel* P1 = static_cast<L6U5V5Pixel*>(TempBuffer) + (y * LockedTexture.Width * 2 + x);
				X8L8U8V8Pixel* P2 = static_cast<X8L8U8V8Pixel*>(LockedTexture.Pixels) + y * LockedTexture.Width + x;

				P2->V = Map5BitSignedTo8BitSigned(P1->V);
				P2->U = Map5BitSignedTo8BitSigned(P1->U);
				P2->L = Map6BitUnsignedTo8BitUnsigned(P1->L);
				P2->X = Map6BitUnsignedTo8BitUnsigned(P1->L);
			}
		}

		appFree(TempBuffer);

		LockedTexture.Pixels = NULL;
		LockedTexture.Width = 0;
		LockedTexture.Height = 0;
	}

	return D3DUnlockRect(D3DTexture, Level);
}

/*
 * CreateTexture
 */

typedef HRESULT(__stdcall*D3DCreateTextureFunc)(void*, UINT, UINT, UINT, DWORD, enum D3DFormat, enum D3DPool, void**);

D3DCreateTextureFunc D3DCreateTexture = NULL;

static HRESULT __stdcall CreateTextureOverride(void* D3DDevice,
											   UINT Width,
											   UINT Height,
											   UINT Levels,
											   DWORD Usage,
											   enum D3DFormat Format,
											   enum D3DPool Pool,
											   void** ppTexture){
	check(D3DCreateTexture);

	if(Format == 61) { // D3DFMT_L6V5U5 = 61
		Format = static_cast<enum D3DFormat>(62); // D3DFMT_X8L8V8U8 = 62

		// It is assumed that the texture is immediately locked after creation (which it is) so these dimensions will always refer to the correct texture
		LockedTexture.Width = Width;
		LockedTexture.Height = Height;
	}

	HRESULT Result = D3DCreateTexture(D3DDevice, Width, Height, Levels, Usage, Format, Pool, ppTexture);

	if(SUCCEEDED(Result)){
		if(!D3DLockRect)
			D3DLockRect = static_cast<D3DLockRectFunc>(PatchVTable(*reinterpret_cast<void***>(*ppTexture), 16, LockRectOverride));

		if(!D3DUnlockRect)
			D3DUnlockRect = static_cast<D3DUnlockRectFunc>(PatchVTable(*reinterpret_cast<void***>(*ppTexture), 17, UnlockRectOverride));
	}


	return Result;
}

/*
 * ModRenderDevice
 */

class MOD_API UModRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(UModRenderDevice, UD3DRenderDevice, 0, Mod)
public:
	// Overriding SetRes because that's where the direct3d device is created
	// NOTE: We could also just create a dummy device and patch the vtable there but this would require linking against d3d8.dll
	virtual UBOOL SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes = 0, UBOOL bSaveSize = true){
		UBOOL result = UD3DRenderDevice::SetRes(Viewport, NewX, NewY, Fullscreen, ColorBytes, bSaveSize);

		if(result && !D3DCreateTexture)
			D3DCreateTexture = static_cast<D3DCreateTextureFunc>(PatchVTable(*reinterpret_cast<void***>(Direct3DDevice8), 20, CreateTextureOverride));

		return result;
	}
};

IMPLEMENT_CLASS(UModRenderDevice)
