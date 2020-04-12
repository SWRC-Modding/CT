/*=============================================================================
	UnTex.h: Unreal texture related classes.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants.
-----------------------------------------------------------------------------*/

enum{ NUM_PAL_COLORS=256 };	// Number of colors in a standard palette.

// Constants.
enum{
	EHiColor565_R = 0xf800,
	EHiColor565_G = 0x07e0,
	EHiColor565_B = 0x001f,

	EHiColor555_R = 0x7c00,
	EHiColor555_G = 0x03e0,
	EHiColor555_B = 0x001f,

	ETrueColor_R  = 0x00ff0000,
	ETrueColor_G  = 0x0000ff00,
	ETrueColor_B  = 0x000000ff,
};

/*-----------------------------------------------------------------------------
	UPalette.
-----------------------------------------------------------------------------*/

extern ENGINE_API FPlane FGetHSV(BYTE H, BYTE S, BYTE V);

//
// A palette object.  Holds NUM_PAL_COLORS unique FColor values,
// forming a 256-color palette which can be referenced by textures.
//
class ENGINE_API UPalette : public UObject{
	DECLARE_CLASS(UPalette,UObject,CLASS_SafeReplace,Engine)

	// Variables.
	TArray<FColor> Colors;

	// Constructors.
	UPalette();

	// UObject interface.
	void Serialize(FArchive& Ar);

	// UPalette interface.
	BYTE BestMatch(FColor Color, INT First);
	UPalette* ReplaceWithExisting();
	void FixPalette();
};

/*-----------------------------------------------------------------------------
	UTexture and FTextureInfo.
-----------------------------------------------------------------------------*/

// Texture level-of-detail sets.
enum ELODSet{
	LODSET_None        = 0,
	LODSET_World       = 1,
	LODSET_PlayerSkin  = 2,
	LODSET_WeaponSkin  = 3,
	LODSET_Terrain     = 4,
	LODSET_Interface   = 5,
	LODSET_RenderMap   = 6,
	LODSET_Lightmap    = 7,
	LODSET_MAX         = 8,
};

//
// Base mipmap.
//
struct ENGINE_API FMipmapBase{
public:
	BYTE*			DataPtr;		// Pointer to data, valid only when locked.
	INT				USize,  VSize;	// Power of two tile dimensions.
	BYTE			UBits,  VBits;	// Power of two tile bits.
	FMipmapBase(BYTE InUBits, BYTE InVBits)
	:	DataPtr		(0)
	,	USize		(1<<InUBits)
	,	VSize		(1<<InVBits)
	,	UBits		(InUBits)
	,	VBits		(InVBits)
	{}
	FMipmapBase()
	{}
};

//
// Texture mipmap.
//
struct ENGINE_API FMipmap : public FMipmapBase{
public:
	TLazyArray<BYTE> DataArray; // Data.

	FMipmap(){}

	FMipmap(BYTE InUBits, BYTE InVBits) : FMipmapBase(InUBits, InVBits),
										  DataArray(USize * VSize){}

	FMipmap(BYTE InUBits, BYTE InVBits, INT InSize) : FMipmapBase(InUBits, InVBits),
													  DataArray(InSize){}

	void Clear(){
		guard(FMipmap::Clear);
		appMemzero(&DataArray[0], DataArray.Num());
		unguard;
	}

	friend FArchive& operator<<(FArchive& Ar, FMipmap& M){
		guard(FMipmap<<);

		Ar << M.DataArray;
		Ar << M.USize << M.VSize << M.UBits << M.VBits;

		return Ar;

		unguard;
	}
};

//
// Texture clearing flags.
//
enum ETextureClear{
	TCLEAR_Temporal	= 1,	// Clear temporal texture effects.
	TCLEAR_Bitmap   = 2,    // Clear the immediate bitmap.
};

enum EEnvMapTransformType{
	EMTT_ViewSpace		= 0x00,
	EMTT_WorldSpace		= 0x01,
	EMTT_LightSpace		= 0x02,
};

//
//	FStaticTexture
//
class ENGINE_API FStaticTexture : public FTexture{
private:
	QWORD           CacheId;
	class UTexture* Texture;
	INT             LastRevision;

public:
	FStaticTexture(UTexture* InTexture);

	virtual QWORD GetCacheId();
	virtual INT GetRevision();

	virtual INT GetWidth();
	virtual INT GetHeight();
	virtual INT GetFirstMip();
	virtual INT GetNumMips();
	virtual ETextureFormat GetFormat();

	virtual ETexClampMode GetUClamp();
	virtual ETexClampMode GetVClamp();

	virtual void* GetRawTextureData(INT MipIndex);
	virtual void UnloadRawTextureData( INT MipIndex );
	virtual void GetTextureData(INT MipIndex,void* Dest,INT DestStride,ETextureFormat DestFormat,UBOOL ColoredMips=0);
	virtual UTexture* GetUTexture();
};

enum ETextureArithOp{
	TAO_Assign                = 0,
	TAO_Add                   = 1,
	TAO_Subtract              = 2,
	TAO_Multiply              = 3,
	TAO_AssignAlpha           = 4,
	TAO_MultiplyOneMinusAlpha = 5,
	TAO_AssignLtoR            = 6,
	TAO_AssignLtoG            = 7,
	TAO_AssignLtoB            = 8,
	TAO_AssignLtoA            = 9,
};

//
// A complex material texture.
//
class ENGINE_API UTexture : public UBitmapMaterial{
	DECLARE_CLASS(UTexture,UBitmapMaterial,CLASS_SafeReplace,Engine)

	UPalette*  Palette;        // Palette if 8-bit palettized.
	UMaterial* Detail;         // Detail texture to apply.
	FLOAT      DetailScale;    // Detail texture scale.
	FColor     MipZero;        // Overall average color of texture.
	FColor     MaxColor;       // Maximum color for normalization.
	DOUBLE     LastUpdateTime; // Last time texture was locked for rendering.

	// Deprecated stuff.
	UTexture* OLDDetailTexture;       // Detail texture to apply.
	UTexture* OLDEnvironmentMap;      // Environment map.
	BYTE      OLDEnvMapTransformType; // Transform type for environment map.
	FLOAT     OLDSpecular;            // Diffuse lighting coefficient (0.f-1.f).

	// Flags.
	BITFIELD    bMasked:1;             // Texture is masked.
	BITFIELD    bAlphaTexture:1;       // Texture is an alphatexture.
	BITFIELD    bTwoSided:1;           // Texture should be rendered two sided when placed directly on a surface.
	BITFIELD    bHighColorQuality:1;   // High color quality hint.
	BITFIELD    bHighTextureQuality:1; // High color quality hint.
	BITFIELD    bRealtime:1;           // Texture changes in realtime.
	BITFIELD    bParametric:1;         // Texture data need not be stored.
	BITFIELD    bRealtimeChanged:1;    // Changed since last render.
	BITFIELD    OLDbHasComp:1;         // !!OLDVER Compressed version included?
	BITFIELD    bNoRawData:1;          // Texture has no raw data (i.e; updated directly...such as MovieTexture)

	BYTE        LODSet; // Level of detail type.
    INT         NormalLOD, MinLOD, MaxLOD;

	// Animation related.
	UTexture* AnimNext;     // Next texture in looped animation sequence.
	UTexture* AnimCur;      // Current animation frame.
	BYTE      PrimeCount;   // Priming total for algorithmic textures.
	BYTE      PrimeCurrent; // Priming current for algorithmic textures.
	FLOAT     MinFrameRate; // Minimum animation rate in fps.
	FLOAT     MaxFrameRate; // Maximum animation rate in fps.
	FLOAT     Accumulator;  // Frame accumulator.

	// Table of mipmaps.
	TArray<FMipmap> Mips;          // Mipmaps in native format.
	BYTE            OLDCompFormat; // !!OLDVER Compressed texture format.

	BYTE		PS2FirstMip;       // using part of the pad space
	BYTE		PS2NumMips;        // using part of the pad space

	FStaticTexture* RenderInterface; // The interface used to render this texture.

	DOUBLE __LastUpdateTime; // Last time texture was locked for rendering.

	// Static.
	static class UClient* __Client;

	// Constructor.
	UTexture();

	// UObject interface.
	void Serialize( FArchive& Ar );
	const TCHAR* Import( const TCHAR* Buffer, const TCHAR* BufferEnd, const TCHAR* FileType );
	void Export( FArchive& Ar, const TCHAR* FileType, INT Indent );
	void PostLoad();
	void Destroy();
	void PostEditChange(); // gam

	// UBitmap interface.
	DWORD GetColorsIndex()
	{
		return Palette->GetIndex();
	}
	FColor* GetColors()
	{
		return Palette ? &Palette->Colors[0] : NULL;
	}
	INT GetNumMips()
	{
		return Mips.Num();
	}
	FMipmapBase* GetMip( INT i )
	{
		return &Mips[i];
	}
	FString GetFormatDesc();

	// UTexture interface.
	virtual void Clear(FColor TexelColor);
	virtual void Clear(DWORD ClearFlags);
	virtual void Init(INT InUSize, INT InVSize);
	virtual void Tick(FLOAT DeltaSeconds);
	virtual void ConstantTimeTick();
	virtual void MousePosition(DWORD Buttons, FLOAT X, FLOAT Y) {}
	virtual void Click(DWORD Buttons, FLOAT X, FLOAT Y){}
	virtual void Update(FTime Time);
	virtual void Prime();
	virtual void Load();
	virtual void ArithOp(UTexture* Operand, ETextureArithOp Operation);

	inline FColor GetTexel(FLOAT u1, FLOAT v1, FLOAT u2, FLOAT v2);

	// UTexture functions.
	void BuildRemapIndex( UBOOL Masked );
	void CreateMips( UBOOL FullMips, UBOOL Downsample );
	void CreateColorRange();
	UBOOL Compress( ETextureFormat Format, UBOOL Mipmaps = 1, struct FDXTCompressionOptions* Options = NULL );
	UBOOL Decompress( ETextureFormat Format );
	ETextureFormat ConvertDXT();
	ETextureFormat ConvertDXT( INT Miplevel, UBOOL ForceRGBA, UBOOL Use4444, BYTE** Data );
	INT DefaultLOD();
	DOUBLE GetLastUpdateTime() { return __LastUpdateTime; }
	void SetLastUpdateTime(DOUBLE T) { __LastUpdateTime = T; }

	// UBitmapMaterial interface.
	virtual FBaseTexture* GetRenderInterface() { return RenderInterface; }
	virtual UBitmapMaterial* Get( FTime Time, UViewport* Viewport )
	{
		Update( Time );
		return AnimCur ? AnimCur : this;
	}

	// UMaterial interface
	virtual UBOOL RequiresSorting()
	{
		return bAlphaTexture;
	}
	virtual UBOOL IsTransparent()
	{
		return bAlphaTexture || bMasked;
	}
};


//
// Information about a locked texture. Used for ease of rendering.
//
enum {MAX_MIPS=12};
struct ENGINE_API FTextureInfo{
	friend class UBitmap;
	friend class UTexture;

	// Variables.
	UTexture*		Texture;				// Optional texture.
	QWORD			CacheID;				// Unique cache ID.
	QWORD			PaletteCacheID;			// Unique cache ID of palette.
	FVector			Pan;					// Panning value relative to texture planes.
	FColor*			MaxColor;				// Maximum color in texture and all its mipmaps.
	ETextureFormat	Format;					// Texture format.
	FLOAT			UScale;					// U Scaling.
	FLOAT			VScale;					// V Scaling.
	INT				USize;					// Base U size.
	INT				VSize;					// Base V size.
	INT				UClamp;					// U clamping value, or 0 if none.
	INT				VClamp;					// V clamping value, or 0 if none.
	INT				NumMips;				// Number of mipmaps.
	INT				LOD;					// Level of detail, 0=highest.
	FColor*			Palette;				// Palette colors.
	BITFIELD		bHighColorQuality:1;	// High color quality hint.
	BITFIELD		bHighTextureQuality:1;	// High color quality hint.
	BITFIELD		bRealtime:1;			// Texture changes in realtime.
	BITFIELD		bParametric:1;			// Texture data need not be stored.
	BITFIELD		bRealtimeChanged:1;		// Changed since last render.
	FMipmapBase*	Mips[MAX_MIPS];			// Array of NumMips of mipmaps.

	// Functions.
	void Load();
	void Unload();
	void CacheMaxColor();
};

/*-----------------------------------------------------------------------------
	UFont.
-----------------------------------------------------------------------------*/

// Font constants.
enum {NUM_FONT_PAGES=256};
enum {NUM_FONT_CHARS=256};

//
// Information about one font glyph which resides in a texture.
//
struct ENGINE_API FFontCharacter{
	// Variables.
	INT StartU, StartV;
	INT USize, VSize;
	//BYTE TextureIndex;

	// Serializer.

	friend FArchive& operator<<(FArchive& Ar, FFontCharacter& Ch)
	{
		guard(FFontCharacter<<);
		return Ar << Ch.StartU << Ch.StartV << Ch.USize << Ch.VSize;
		unguard;
	}

};

//
// A font page.
//
/*
struct ENGINE_API FFontPage{
	// Variables.
	UTexture* Texture;
	TArray<FFontCharacter> Characters;

	// Serializer.

	friend FArchive& operator<<(FArchive& Ar, FFontPage& Ch)
	{
		guard(FFontCharacter<<);
		return Ar << Ch.Texture << Ch.Characters;
		unguard;
	}


	struct FFontPage& operator=(struct FFontCharacter const &);
};
*/

//
// A font object, containing information about a set of glyphs.
// The glyph bitmaps are stored in the contained textures, while
// the font database only contains the coordinates of the individual
// glyph.
//
class ENGINE_API UFont : public UObject{
	DECLARE_CLASS(UFont,UObject,0,Engine)

	// Variables.
	TArray<FFontCharacter> Characters;
	TArray<class UTexture*> Textures;
	TMap<TCHAR, TCHAR> CharRemap;
	UBOOL IsRemapped;

	// Found in UT2003, IDK what gam means, probably the signature
	INT Kerning; // gam

	// Constructors.
	UFont();

	// Found in IDA
	UFont(UFont const &);


	// UObject interface.
	void Serialize(FArchive& Ar);

	// UFont interface
	TCHAR RemapChar(TCHAR ch)
	{
		TCHAR *p;
		if(!IsRemapped)
			return ch;
		p = CharRemap.Find(ch);
		return p ? *p : 32; // return space if not found.
	}

	// Found in IDA
	void GetCharSize(UFont* Font, TCHAR InCh, INT& Width, INT& Height);

	// Also found in IDA
	UFont& operator=(class UFont const &);


};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
