#include "../Inc/ModRenderDevice.h"
#include "../../Editor/Inc/Editor.h"
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

// Virtual table indices of different d3d functions that we are trying to hook
enum{
	// D3D8
	D3DVTableIndex_D3D8CreateDevice    = 15,
	// D3DDevice
	D3DVTableIndex_DeviceCreateTexture = 20,
	// D3DTexture
	D3DVTableIndex_TextureLockRect     = 16,
	D3DVTableIndex_TextureUnlockRect   = 17
};

/*
 * L6V5U5 to X8L8V8U8 texture format conversion.
 *
 * L6V5U5 is not supported on newer hardware and either makes the game and editor crash or the driver converts it directly to X8L8V8U8 which results in visual artifacts.
 * That's why we detect if a texture is created with the L6V5U5 format and perform a proper conversion to X8L8V8U8 or if that is not available V8U8 and if not even that one works ARGB.
 */

// Information about the current texture returned by CreateTexture and used by Lock/UnlockRect
static IDirect3DTexture8* CurrentTexture;
static D3DFORMAT          CurrentTextureSourceFormat;
static D3DFORMAT          CurrentTextureTargetFormat;
static UINT               CurrentTextureNumMipLevels;
static UINT               CurrentMipLevelWidth;
static UINT               CurrentMipLevelHeight;
static void*              CurrentMipLevelPixels;

// Conversion from one bumpmap format to another

/*
 * L6V5U5 format conversion
 */

static void ConvertL6V5U5ToX8L8V8U8(const void* In, void* Out, INT Width, INT Height){
	for(INT Y = 0; Y < Height; ++Y){
		for(INT X = 0; X < Width; ++X){
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

static void ConvertL6V5U5ToV8U8(const void* In, void* Out, INT Width, INT Height){
	for(INT Y = 0; Y < Height; ++Y){
		for(INT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FL6V5U5Pixel* P1 = static_cast<const FL6V5U5Pixel*>(In) + Index;
			FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			// No luminance
		}
	}
}

/*
 * X8L8V8U8 format conversion
 */

static void ConvertX8L8V8U8ToV8U8(const void* In, void* Out, INT Width, INT Height){
	for(INT Y = 0; Y < Height; ++Y){
		for(INT X = 0; X < Width; ++X){
			INT Index = Y * Width + X;
			const FX8L8V8U8Pixel* P1 = static_cast<const FX8L8V8U8Pixel*>(In) + Index;
			FV8U8Pixel* P2 = static_cast<FV8U8Pixel*>(Out) + Index;

			P2->V = Map5BitSignedTo8BitSigned(P1->V);
			P2->U = Map5BitSignedTo8BitSigned(P1->U);
			// No luminance
		}
	}
}

/*
 * LockRect
 */

typedef HRESULT(__stdcall*D3DTextureLockRectFunc)(IDirect3DTexture8*, UINT, D3DLOCKED_RECT*, const RECT*, DWORD);

D3DTextureLockRectFunc D3DTextureLockRect = NULL;

static HRESULT __stdcall D3DTextureLockRectOverride(IDirect3DTexture8* D3DTexture, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags){
	if(D3DTexture != CurrentTexture)
		return D3DTextureLockRect(D3DTexture, Level, pLockedRect, pRect, Flags);

	D3DSURFACE_DESC SurfaceDesc;
	HRESULT GetLevelDescResult = D3DTexture->GetLevelDesc(Level, &SurfaceDesc);

	if(FAILED(GetLevelDescResult))
		return GetLevelDescResult;

	CurrentMipLevelWidth = SurfaceDesc.Width;
	CurrentMipLevelHeight = SurfaceDesc.Height;
	CurrentMipLevelPixels = appMalloc(SurfaceDesc.Size);

	INT BytesPerPixel = 0;

	switch(CurrentTextureSourceFormat){
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
		appErrorf("Unexpected texture format (%i)", CurrentTextureSourceFormat);
	}

	pLockedRect->Pitch = CurrentMipLevelWidth * BytesPerPixel;
	pLockedRect->pBits = CurrentMipLevelPixels;

	return S_OK;
}

/*
 * UnlockRect
 */

typedef HRESULT(__stdcall*D3DTextureUnlockRectFunc)(IDirect3DTexture8*, UINT);

D3DTextureUnlockRectFunc D3DTextureUnlockRect = NULL;

static HRESULT __stdcall D3DTextureUnlockRectOverride(IDirect3DTexture8* D3DTexture, UINT Level){
	if(D3DTexture != CurrentTexture)
		return D3DTextureUnlockRect(D3DTexture, Level);

	D3DLOCKED_RECT LockedRect;
	HRESULT Result = D3DTextureLockRect(D3DTexture, Level, &LockedRect, NULL, 0);

	if(SUCCEEDED(Result)){
		if(CurrentTextureSourceFormat == D3DFMT_V8U8){
			ConvertV8U8ToRGBA8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
		}else if(CurrentTextureSourceFormat == D3DFMT_L6V5U5){
			if(CurrentTextureTargetFormat == D3DFMT_V8U8)
				ConvertL6V5U5ToV8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFMT_X8L8V8U8)
				ConvertL6V5U5ToX8L8V8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFMT_A8R8G8B8)
				ConvertL6V5U5ToRGBA8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
		}else if(CurrentTextureSourceFormat == D3DFMT_X8L8V8U8){
			if(CurrentTextureTargetFormat == D3DFMT_V8U8)
				ConvertX8L8V8U8ToV8U8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
			else if(CurrentTextureTargetFormat == D3DFMT_A8R8G8B8)
				ConvertX8L8V8U8ToRGB8(CurrentMipLevelPixels, LockedRect.pBits, CurrentMipLevelWidth, CurrentMipLevelHeight);
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

typedef HRESULT(__stdcall*D3DDeviceCreateTextureFunc)(IDirect3DDevice8*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture8**);

D3DDeviceCreateTextureFunc D3DDeviceCreateTexture = NULL;

static HRESULT __stdcall D3DDeviceCreateTextureOverride(IDirect3DDevice8* D3DDevice,
														UINT Width,
														UINT Height,
														UINT Levels,
														DWORD Usage,
														D3DFORMAT Format,
														D3DPOOL Pool,
														IDirect3DTexture8** ppTexture){
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

	if(FAILED(Result)){ // If X8L8V8U8 is not supported V8U8 might still be so try that. Visually the same except for missing luminance
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

	if(FAILED(Result)){ // If no bumpmap format is available we fall back to ARGB. Looks fine visually and should always be supported
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

typedef HRESULT(__stdcall*D3D8CreateDeviceFunc)(IDirect3D8* D3D8,
                                                UINT Adapter,
                                                D3DDEVTYPE DeviceType,
                                                HWND hFocusWindow,
                                                DWORD BehaviorFlags,
                                                D3DPRESENT_PARAMETERS* pPresentationParameters,
                                                IDirect3DDevice8** ppReturnedDeviceInterface);

D3D8CreateDeviceFunc D3D8CreateDevice = NULL;

HRESULT __stdcall D3D8CreateDeviceOverride(IDirect3D8* D3D8,
                                           UINT Adapter,
                                           D3DDEVTYPE DeviceType,
                                           HWND hFocusWindow,
                                           DWORD BehaviorFlags,
                                           D3DPRESENT_PARAMETERS* pPresentationParameters,
                                           IDirect3DDevice8** ppReturnedDeviceInterface){
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

	HitStack.SetNoShrink(true);
	AllHitData.SetNoShrink(true);
}

/*
 * Converts a color value read from the frame buffer to an index that is stored in OutIndex.
 * If this function returns true, it means that a 'preferred' selection type was found which should be used if there are more than one possible selections.
 */
bool FModRenderInterface::ProcessHitColor(FColor HitColor, INT* OutIndex){
	INT Index = HitColor.R | HitColor.G << 8 | HitColor.B << 16;

	--Index; // Index was incremented before drawing so that a value of 0 means 'no selection'. Here it is decremented again to get the actual array index.

	if(Index >= 0 && Index < AllHitData.Num() - (INT)sizeof(HHitProxy)){
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

void FModRenderInterface::ProcessHit(INT HitProxyIndex){
	if(HitProxyIndex >= 0 && HitProxyIndex < AllHitData.Num() - (INT)sizeof(HHitProxy)){
		INT ParentIndices[32];
		INT NumParents = 0;

		// Collecting all parents of the successful hit
		for(INT ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitProxyIndex])->ParentIndex;
			ParentIndex != INDEX_NONE;
			ParentIndex = reinterpret_cast<FHitProxyInfo*>(&AllHitData[ParentIndex])->ParentIndex){

			ParentIndices[NumParents] = ParentIndex;
			++NumParents;
			checkSlow(NumParents <= ARRAY_COUNT(ParentIndices));
		}

		INT HitCount = 0;

		// Copying hit hierarchy to HitData
		for(INT i = 0; i < NumParents; ++i){
			HHitProxy* Parent = reinterpret_cast<HHitProxy*>(&AllHitData[ParentIndices[i] + sizeof(FHitProxyInfo)]);

			appMemcpy(HitData + HitCount, Parent, Parent->Size);

			HitCount += Parent->Size;
		}

		HHitProxy* Hit = reinterpret_cast<HHitProxy*>(&AllHitData[HitProxyIndex + sizeof(FHitProxyInfo)]);

		appMemcpy(HitData + HitCount, Hit, Hit->Size);
		*HitSize = HitCount + Hit->Size;
	}else{
		// Select nothing
		UEditorEngine* Editor = CastChecked<UEditorEngine>(GEngine);

		Editor->Trans->Begin("Select None");
		Editor->SelectNone(Editor->Level, 1, 1);
		Editor->Trans->End();
	}

	HitStack.Empty();
	AllHitData.Empty();
	HitData = NULL;
}

void FModRenderInterface::PushHit(const BYTE* Data, INT Count){
	checkSlow(GIsEditor);
	checkSlow(UModRenderDevice::SolidSelectionShader);
	checkSlow(UModRenderDevice::AlphaSelectionShader);
	checkSlow(HitData);
	checkSlow(HitSize);

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

void FModRenderInterface::PopHit(INT Count, UBOOL Force){
	HitStack.Pop();
}

void FModRenderInterface::SetMaterial(UMaterial* Material, FString* ErrorString, UMaterial** ErrorMaterial, INT* NumPasses){
	Impl->SetMaterial(Material, ErrorString, ErrorMaterial, NumPasses);

	// Checking whether the current material has a texture with an alpha channel somewhere down the hierarchy.
	// If it has, it is used to draw the selection buffer for more accuracy. If not, the polygons are completly filled.
	while(Material){
		if(Material->IsA<UBitmapMaterial>()){
			UTexture* Tmp = Cast<UTexture>(Material);

			if(Tmp && Tmp->bAlphaTexture)
				CurrentTexture = static_cast<UBitmapMaterial*>(Material);
			else
				CurrentTexture = NULL;

			return;
		}else if(Material->IsA<UShader>()){
			Material = static_cast<UShader*>(Material)->Diffuse;
		}else{
			Material = NULL;
		}
	}

	CurrentTexture = NULL;
}

UBOOL FModRenderInterface::SetHardwareShaderMaterial(UHardwareShader* Material, FString* ErrorString, UMaterial** ErrorMaterial){
	CurrentTexture = NULL;

	return Impl->SetHardwareShaderMaterial(Material, ErrorString, ErrorMaterial);
}

void FModRenderInterface::DrawPrimitive(EPrimitiveType PrimitiveType, INT FirstIndex, INT NumPrimitives, INT MinIndex, INT MaxIndex){
	UHardwareShader* Shader;

	if(HitStack.Num() > 0){
		INT            HitDataIndex = HitStack.Last();
		FHitProxyInfo* Info = reinterpret_cast<FHitProxyInfo*>(&AllHitData[HitDataIndex]);
		AActor*        HitActor = reinterpret_cast<HHitProxy*>(reinterpret_cast<BYTE*>(Info) + sizeof(FHitProxyInfo))->GetActor();

		// Sprites are drawn with alpha regardless of whether UTexture::bAlphaTexture is set or not so we need to check for it
		if(HitActor && (HitActor->DrawType == DT_Sprite || HitActor->DrawType == DT_Particle)){
			Shader = UModRenderDevice::AlphaSelectionShader;
			Shader->Textures[0] = Cast<UBitmapMaterial>(HitActor->Texture);
		}else{
			// Alpha is ignored in the texture browser since there it should be possible to click anywhere on a texture to select it
			if(CurrentTexture &&
			   appStricmp(reinterpret_cast<HHitProxy*>(&AllHitData[HitStack.Last() + sizeof(FHitProxyInfo)])->GetName(), "HBrowserMaterial") != 0){
				Shader = UModRenderDevice::AlphaSelectionShader;
				Shader->Textures[0] = CurrentTexture;
			}else{
				Shader = UModRenderDevice::SolidSelectionShader;
				Shader->ZTest = !(HitActor && (HitActor->DrawType == DT_Brush || HitActor->DrawType == DT_AntiPortal)); // Disable ZTest for brushes since they are rendered on top of everything else
			}
		}

		// Convert index to shader color

		++HitDataIndex; // Adding 1 because 0 means no hit. This is subtracted again later.

		checkSlow(RenDev->LockedViewport->ColorBytes == 3 || RenDev->LockedViewport->ColorBytes == 4);

		Shader->PSConstants[0].Value.X = static_cast<BYTE>(HitDataIndex) / 255.0f;
		Shader->PSConstants[0].Value.Y = static_cast<BYTE>(HitDataIndex >> 8) / 255.0f;
		Shader->PSConstants[0].Value.Z = static_cast<BYTE>(HitDataIndex >> 16) / 255.0f;
	}else{
		Shader = UModRenderDevice::SolidSelectionShader;
		Shader->PSConstants[0].Value = FPlane(0.0f, 0.0f, 0.0f, 0.0f);
	}

	SetHardwareShaderMaterial(Shader, NULL, NULL);
	Impl->DrawPrimitive(PrimitiveType, FirstIndex, NumPrimitives, MinIndex, MaxIndex);
}

/*
 * ModRenderDevice
 */

UBOOL UModRenderDevice::Init(){
	// Initialize fixes. This might not be the best place but idk where else to put it...
	InitSWRCFix();
	bEnableSelectionFix = USWRCFix::Instance->EnableEditorSelectionFix;

	UBOOL Result = Super::Init();

	if(Result){
		MaybePatchVTable(&D3D8CreateDevice, Direct3D8, D3DVTableIndex_D3D8CreateDevice, D3D8CreateDeviceOverride);

		if(!SolidSelectionShader){
			// Initialize shader used for selection in the editor
			SolidSelectionShader = new UHardwareShader();

			SolidSelectionShader->VertexShaderText = "vs.1.1\n"
			                                         "m4x4 r0, v0, c0\n"
			                                         "mov oPos, r0\n";
			SolidSelectionShader->PixelShaderText = "ps.1.1\n"
			                                        "mov r0, c0\n";
			SolidSelectionShader->VSConstants[0].Type = EVC_ObjectToScreenMatrix;
			SolidSelectionShader->PSConstants[0].Type = EVC_MaterialDefined;
			SolidSelectionShader->ZTest = 1;
			SolidSelectionShader->ZWrite = 1;
		}

		if(!AlphaSelectionShader){
			// Initialize shader used for selection of objects with alpha channel in the editor
			AlphaSelectionShader = new UHardwareShader();

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
		}
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
	}else if(USWRCFix::Instance){
		if(ParseCommand(&Cmd, "SETFOV")){
			TObjectIterator<UViewport> It;

			checkSlow(It);

			FLOAT FOV = appAtof(Cmd);

			Ar.Logf("Setting field of view to %f", FOV);
			USWRCFix::Instance->SetFOV(It->Actor, FOV);

			return 1;
		}else if(ParseCommand(&Cmd, "SETFPSLIMIT")){
			if(appStrlen(Cmd) > 0){
				USWRCFix::Instance->FpsLimit = Max(0.0f, appAtof(Cmd));
				USWRCFix::Instance->SaveConfig();
			}

			return 1;
		}else if(ParseCommand(&Cmd, "GETFPSLIMIT")){
			Ar.Logf("%f", USWRCFix::Instance->FpsLimit);

			return 1;
		}
	}

	return Super::Exec(Cmd, Ar);
}

FRenderInterface* UModRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	FRenderInterface* RI = Super::Lock(Viewport, HitData, HitSize);

	if(bEnableSelectionFix && GIsEditor && RI && HitData && CastChecked<UEditorEngine>(GEngine)->Mode != EM_EyeDropper){
		LockedViewport = Viewport;
		RI->EnableFog(0); // No fog in the selection buffer or else there will be wrong color values.
		RenderInterface.Impl = RI;
		RenderInterface.HitData = HitData;
		RenderInterface.HitSize = HitSize;
		// Storing color values
		C_ActorArrow = GEngine->C_ActorArrow;
		GEngine->C_ActorArrow = FColor(0x00000000);

		return &RenderInterface;
	}

	return RI;
}

void UModRenderDevice::Unlock(FRenderInterface* RI){
	if(RI == &RenderInterface){
		checkSlow(LockedViewport);
		checkSlow(RenderInterface.HitData);
		checkSlow(RenderInterface.HitSize);

		IDirect3DSurface8* RenderTarget = NULL;

		Direct3DDevice8->GetRenderTarget(&RenderTarget);

		checkSlow(RenderTarget != NULL);

		D3DSURFACE_DESC Desc;

		RenderTarget->GetDesc(&Desc);

		D3DLOCKED_RECT LockedRect;

		RenderTarget->LockRect(&LockedRect, NULL, 0);

		INT HitProxyIndex = INDEX_NONE;

		if(LockedViewport->ColorBytes != 4){
			appMsgf(3, "16-bit color is not supported when using the selection fix. Switch to 32 or use the 'FIXSELECT' command to toggle the fix off");
		}else{
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
			for(INT Y = -LockedViewport->HitYL; Y < LockedViewport->HitYL + 1; Y++, src = (DWORD*)((BYTE*)src + LockedRect.Pitch)){
				for(INT X = -LockedViewport->HitXL; X < LockedViewport->HitXL + 1; X++){
					if(src + X >= LockedRect.pBits && src[X] != 0x0){
						FColor HitColor = FColor((src[X] >> 16) & 0xff, (src[X] >> 8) & 0xff, (src[X]) & 0xff);
						INT Index = INDEX_NONE;
						FLOAT Dist = FVector(X, Y, 0.0f).Size2D(); // Distance of the hit from the center of the hit area. The closer it is, the higher the priority over other hits.

						if(RenderInterface.ProcessHitColor(HitColor, &Index)){
							if(Dist < PreferredHitDist){
								PreferredHitDist = Dist;
								PreferredHitProxyIndex = Index;
							}
						}else if(Index >= 0){
							if(Dist < HitDist){
								HitDist = Dist;
								HitProxyIndex = Index;
							}
						}
					}
				}
			}

			if(PreferredHitProxyIndex >= 0)
				HitProxyIndex = PreferredHitProxyIndex;
		}

		RenderTarget->UnlockRect();
		Super::Unlock(RenderInterface.Impl);
		RenderInterface.ProcessHit(HitProxyIndex);

		// Restoring color values
		GEngine->C_ActorArrow = C_ActorArrow;

		if(bDebugSelectionBuffer){
			LockedViewport->Present();
			appSleep(3.0f);
		}
	}else{
		Super::Unlock(RI);
	}

	LockedViewport = NULL;
}

UHardwareShader* UModRenderDevice::SolidSelectionShader = NULL;
UHardwareShader* UModRenderDevice::AlphaSelectionShader = NULL;

IMPLEMENT_CLASS(UModRenderDevice)
