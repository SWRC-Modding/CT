/*=============================================================================
	UnTex.h: Unreal texture related classes.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants.
-----------------------------------------------------------------------------*/

enum {NUM_PAL_COLORS=256};	// Number of colors in a standard palette.

// Constants.
enum
{
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

//
// A truecolor value.
//
#define GET_COLOR_DWORD(color) (*(DWORD*)&(color))
class ENGINE_API FColor{
public:
	// Variables.
	union { struct{ BYTE B,G,R,A; }; DWORD AlignmentDummy; };

	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA = 255 )
		:	R(InR), G(InG), B(InB), A(InA) {}
	//FColor(const FLinearColor& C);
	FColor( const FPlane& P )
			:	R(Clamp(appFloor(P.X*255),0,255))
			,	G(Clamp(appFloor(P.Y*255),0,255))
			,	B(Clamp(appFloor(P.Z*255),0,255))
			,	A(Clamp(appFloor(P.W*255),0,255))
		{}
	FColor( DWORD InColor )
	{ DWColor() = InColor; }

	// Serializer.
	friend FArchive& operator<< (FArchive &Ar, FColor &Color )
	{
		return Ar << Color.R << Color.G << Color.B << Color.A;
	}

	// Operators.
	UBOOL operator==( const FColor &C ) const
	{
		return DWColor() == C.DWColor();
	}
	UBOOL operator!=( const FColor& C ) const
	{
		return DWColor() != C.DWColor();
	}
	void operator+=(FColor C)
	{
#if ASM
		_asm
		{
			mov edi, DWORD PTR [this]
			mov ebx, DWORD PTR [C]
			mov eax, DWORD PTR [edi]

			xor ecx, ecx
			add al, bl
			adc ecx, 0xffffffff
			not ecx
			or al, cl
			mov BYTE PTR [edi], al
			inc edi
			shr eax, 8
			shr ebx, 8

			xor ecx, ecx
			add al, bl
			adc ecx, 0xffffffff
			not ecx
			or al, cl
			mov BYTE PTR [edi], al
			inc edi
			shr eax, 8
			shr ebx, 8

			xor ecx, ecx
			add al, bl
			adc ecx, 0xffffffff
			not ecx
			or al, cl
			mov BYTE PTR [edi], al
			inc edi
			shr eax, 8
			shr ebx, 8

			xor ecx, ecx
			add al, bl
			adc ecx, 0xffffffff
			not ecx
			or al, cl
			mov BYTE PTR [edi], al
		}
#else
		R = (BYTE) Min((INT) R + (INT) C.R,255);
		G = (BYTE) Min((INT) G + (INT) C.G,255);
		B = (BYTE) Min((INT) B + (INT) C.B,255);
		A = (BYTE) Min((INT) A + (INT) C.A,255);
#endif
	}
	INT Brightness() const
	{
		return (2*(INT)R + 3*(INT)G + 1*(INT)B)>>3;
	}
	FLOAT FBrightness() const
	{
		return (2.f*R + 3.f*G + 1.f*B)/(6.f*256.f);
	}
	FPlane Plane() const
	{
		return FPlane(R/255.f,G/255.f,B/255.f,A/255.f);
	}
	FColor Brighten( INT Amount )
	{
		return FColor( Plane() * (1.f - Amount/24.f) );
	}
	FColor RenderColor()
	{
		return *this;
	}
	//FLinearColor FromRGBE() const;
	operator FPlane() const
	{
		return FPlane(R/255.f,G/255.f,B/255.f,A/255.f);
	}
	operator FVector() const
	{
		return FVector(R/255.f, G/255.f, B/255.f);
	}
	operator DWORD() const 
	{ 
		return DWColor(); 
	}
};

extern ENGINE_API FPlane FGetHSV( BYTE H, BYTE S, BYTE V );

//
// A palette object.  Holds NUM_PAL_COLORS unique FColor values, 
// forming a 256-color palette which can be referenced by textures.
//
class ENGINE_API UPalette : public UObject
{
	DECLARE_CLASS(UPalette,UObject,CLASS_SafeReplace,Engine)

	// Variables.
	TArray<FColor> Colors;

	// Constructors.
	UPalette();

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UPalette interface.
	BYTE BestMatch( FColor Color, INT First );
	UPalette* ReplaceWithExisting();
	void FixPalette();
};

/*-----------------------------------------------------------------------------
	UTexture and FTextureInfo.
-----------------------------------------------------------------------------*/

// Texture level-of-detail sets.
enum ELODSet
{
	LODSET_None  = 0,  // No level of detail mipmap tossing.
	LODSET_World = 1,  // World level-of-detail set.
	LODSET_Skin  = 2,  // Skin level-of-detail set.
	LODSET_MAX   = 8,  // Maximum.
};

enum {MAX_TEXTURE_LOD=4};

//
// Base mipmap.
//
struct ENGINE_API FMipmapBase
{
public:
	BYTE*			DataPtr;		// Pointer to data, valid only when locked.
	INT				USize,  VSize;	// Power of two tile dimensions.
	BYTE			UBits,  VBits;	// Power of two tile bits.
	FMipmapBase( BYTE InUBits, BYTE InVBits )
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
struct ENGINE_API FMipmap : public FMipmapBase
{
public:
	TLazyArray<BYTE> DataArray; // Data.
	FMipmap()
	{}
	FMipmap( BYTE InUBits, BYTE InVBits )
	:	FMipmapBase( InUBits, InVBits )
	,	DataArray( USize * VSize )
	{}
	FMipmap( BYTE InUBits, BYTE InVBits, INT InSize )
	:	FMipmapBase( InUBits, InVBits )
	,	DataArray( InSize )
	{}
	void Clear()
	{
		guard(FMipmap::Clear);
		appMemzero( &DataArray[0], DataArray.Num() );
		unguard;
	}
	friend FArchive& operator<<( FArchive& Ar, FMipmap& M )
	{
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
enum ETextureClear
{
	TCLEAR_Temporal	= 1,	// Clear temporal texture effects.
	TCLEAR_Bitmap   = 2,    // Clear the immediate bitmap.
};

//
// Texture formats.
//
enum ETextureFormat{
	TEXF_P8,
	TEXF_RGBA7,
	TEXF_RGB16,
	TEXF_DXT1,
	TEXF_RGB8,
	TEXF_RGBA8,
	TEXF_NODATA,
	TEXF_DXT3,
	TEXF_DXT5,
	TEXF_L8,
	TEXF_G16,
	TEXF_RRRGGGBBB,
	TEXF_MAX
};

//
// A low-level bitmap.
//
class ENGINE_API UBitmap : public UObject
{
	DECLARE_ABSTRACT_CLASS(UBitmap,UObject,0,Engine)
	
	// General bitmap information.
	BYTE		Format;					// ETextureFormat.
	UPalette*	Palette;			// Palette if 8-bit palettized.
	INT			Padding[13];		// Pad for 52 bytes
	BYTE		Pad1;				// 1 More bytes
	// I have a hunch that this goes here...
	BYTE		UClampMode, VClampMode; // ETexClampMode
	BYTE		UBits, VBits;		// # of bits in USize, i.e. 8 for 256.
	BYTE		Pad;				// Padding byte
	INT			USize, VSize;		// Size, must be power of 2.
	INT			UClamp, VClamp;		// Clamped width, must be <= size.
	// Somewhere in here goes DontCache
	// BITFIELD DontCache:1 GCC_PACK(4)
	FColor		MipZero;			// Overall average color of texture.
	FColor		MaxColor;			// Maximum color for normalization.
	INT			__LastUpdateTime[2];// Last time texture was locked for rendering.

	// Static.
	static class UClient* __Client;

	// Constructor.
	UBitmap();

	// UBitmap interface.
	virtual void Lock( FTextureInfo& TextureInfo, FTime Time, INT LOD, URenderDevice* RenDev )=0;
	virtual void Unlock( FTextureInfo& TextureInfo )=0;
	virtual FMipmapBase* GetMip( INT i )=0;
	FTime GetLastUpdateTime() {FTime T; appMemcpy(&T,&__LastUpdateTime,sizeof(FTime)); return T;}
	void SetLastUpdateTime(FTime T) {appMemcpy(&__LastUpdateTime,&T,sizeof(FTime));}
};

//	
// A complex material texture.	
//
class ENGINE_API UTexture : public UBitmap
{
	DECLARE_CLASS(UTexture,UBitmap,CLASS_SafeReplace,Engine)

	// Subtextures.
	UTexture*	BumpMap;			// Bump map to illuminate this texture with.
	UTexture*	DetailTexture;		// Detail texture to apply.
	UTexture*	MacroTexture;		// Macrotexture to apply, not currently used.

	// Surface properties.
	FLOAT		Diffuse;			// Diffuse lighting coefficient (0.f-1.f).
	FLOAT		Specular;			// Specular lighting coefficient (0.f-1.f).
	FLOAT		Alpha;				// Reflectivity (0.f-0.1f).
	FLOAT       Scale;              // Scaling relative to parent, 1.f=normal.
	FLOAT		Friction;			// Surface friction coefficient, 1.f=none, 0.95f=some.
	FLOAT		MipMult;			// Mipmap multiplier.

	// Sounds.
	USound*		FootstepSound;		// Footstep sound.
	USound*		HitSound;			// Sound when the texture is hit with a projectile.

	// Flags.
	DWORD		PolyFlags;			// Polygon flags to be applied to Bsp polys with texture (See PF_*).
	BITFIELD	bHighColorQuality:1; // High color quality hint.
	BITFIELD	bHighTextureQuality:1; // High color quality hint.
	BITFIELD	bRealtime:1;        // Texture changes in realtime.
	BITFIELD	bParametric:1;      // Texture data need not be stored.
	BITFIELD	bRealtimeChanged:1; // Changed since last render.
	BITFIELD    bHasComp:1;         // Compressed version included?
	BYTE        LODSet; // Level of detail type.

	// Animation related.
	UTexture*	AnimNext;			// Next texture in looped animation sequence.
	UTexture*	AnimCur;			// Current animation frame.
	BYTE		PrimeCount;			// Priming total for algorithmic textures.
	BYTE		PrimeCurrent;		// Priming current for algorithmic textures.
	FLOAT		MinFrameRate;		// Minimum animation rate in fps.
	FLOAT		MaxFrameRate;		// Maximum animation rate in fps.
	FLOAT		Accumulator;		// Frame accumulator.

	// Table of mipmaps.
	TArray<FMipmap> Mips;			// Mipmaps in native format.
	TArray<FMipmap> CompMips;		// Mipmaps in requested format.
	BYTE            CompFormat;     // Decompressed texture format.

	// Constructor.
	UTexture();

	// UObject interface.
	void Serialize( FArchive& Ar );
	const TCHAR* Import( const TCHAR* Buffer, const TCHAR* BufferEnd, const TCHAR* FileType );
	void Export( FArchive& Ar, const TCHAR* FileType, INT Indent );
	void PostLoad();
	void Destroy();

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
	void Lock( FTextureInfo& TextureInfo, FTime Time, INT LOD, URenderDevice* RenDev );
	void Unlock( FTextureInfo& TextureInfo );

	// UTexture interface.
	virtual void Clear( DWORD ClearFlags );
	virtual void Init( INT InUSize, INT InVSize );
	virtual void Tick( FLOAT DeltaSeconds );
	virtual void ConstantTimeTick();
	virtual void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y ) {}
	virtual void Click( DWORD Buttons, FLOAT X, FLOAT Y ) {}
	virtual void Update( FTime Time );
	virtual void Prime();

	// UTexture functions.
	void BuildRemapIndex( UBOOL Masked );
	void CreateMips( UBOOL FullMips, UBOOL Downsample );
	void CreateColorRange();
	UBOOL Compress( ETextureFormat Format, UBOOL Mipmap );
	UBOOL Decompress( ETextureFormat Format );
	INT DefaultLOD();

	// UTexture accessors.
	UTexture* Get( FTime Time )
	{
		Update( Time );
		return AnimCur ? AnimCur : this;
	}
};


//
// Information about a locked texture. Used for ease of rendering.
//
enum {MAX_MIPS=12};
struct ENGINE_API FTextureInfo
{
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
struct ENGINE_API FFontCharacter
{
	// Variables.
	INT StartU, StartV;
	INT USize, VSize;
	//BYTE TextureIndex;

	// Serializer.
	
	friend FArchive& operator<<( FArchive& Ar, FFontCharacter& Ch )
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
struct ENGINE_API FFontPage
{
	// Variables.
	UTexture* Texture;
	TArray<FFontCharacter> Characters;

	// Serializer.
	
	friend FArchive& operator<<( FArchive& Ar, FFontPage& Ch )
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
class ENGINE_API UFont : public UObject
{
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
	void Serialize( FArchive& Ar );

	// UFont interface
	TCHAR RemapChar(TCHAR ch)
	{
		TCHAR *p;
		if( !IsRemapped )
			return ch;
		p = CharRemap.Find(ch);
		return p ? *p : 32; // return space if not found.
	}
	
	// Found in IDA
	void GetCharSize(UFont* Font, TCHAR InCh, INT& Width, INT& Height );
	
	// Also found in IDA
	UFont& operator=(class UFont const &);

	
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
