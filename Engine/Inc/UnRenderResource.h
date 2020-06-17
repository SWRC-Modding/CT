/*=============================================================================
	UnRenderResource.h: Rendering resources, vertex buffers, index buffers
	Copyright 1997-2000 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Jack Porter
=============================================================================*/

#ifndef _UNRENDERRESOURCE_H_
#define _UNRENDERRESOURCE_H_

//
// Forward declarations.
//
class UTexture;
class FCubemap;
class FTexture;
class FRenderTarget;
class FCompositeTexture;

/*-----------------------------------------------------------------------------
	FColor.
-----------------------------------------------------------------------------*/

class ENGINE_API FColor
{
public:
	// Variables.
#if __INTEL_BYTE_ORDER__
#ifdef __PSX2_EE__
	BYTE R GCC_ALIGN(4);
	BYTE G,B,A;
#else
#if _MSC_VER
	union { struct{ BYTE B,G,R,A; }; DWORD AlignmentDummy; };
#else
	BYTE B GCC_ALIGN(4);
	BYTE G,R,A;
#endif
#endif
#else
	//!!TODO: handle packing.
	BYTE A,R,G,B;
#endif

	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA = 255 )
	:	R(InR), G(InG), B(InB), A(InA) {}
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
#ifdef ASM
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
	DWORD TrueColor() const
	{
		return DWColor();
	}
	_WORD HiColor565() const
	{
		return ((R >> 3) << 11) + ((G >> 2) << 5) + (B >> 3);
	}
	_WORD HiColor555() const
	{
		return ((R >> 3) << 10) + ((G >> 3) << 5) + (B >> 3);
	}
	_WORD HiColor4444() const
	{
		return ((A >> 4) << 12) + ((R >> 4) << 8) + ((G >> 4) << 4) + (B >> 4);
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
		if( GIsOpenGL )
			return FColor( B, G, R, A);
		else
			return *this;
	}
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

//
// FRenderResource
// Abstract rendering resource interface.
//
class FRenderResource{
public:
	QWORD CacheId;
	INT Revision;

	FRenderResource() : CacheId(0), Revision(1){}

	virtual ~FRenderResource(){}
	virtual INT GetRevision(){ return Revision; }
	virtual bool IsUMA(){ return false; }
	virtual bool bPersistantCache(){ return false; }

	QWORD GetCacheId(){ return CacheId; }
	void Serialize(FArchive& Ar);
};

//
//	EComponentType
//
enum EComponentType
{
	CT_Float4,
	CT_Float3,
	CT_Float2,
	CT_Float1,
	CT_Color
};

//
//	EFixedVertexFunction
//
enum EFixedVertexFunction{
	FVF_Position,
	FVF_Normal,
	FVF_Diffuse,
	FVF_Specular,
	FVF_TexCoord0,
	FVF_TexCoord1,
	FVF_TexCoord2,
	FVF_TexCoord3,
	FVF_TexCoord4,
	FVF_TexCoord5,
	FVF_TexCoord6,
	FVF_TexCoord7,
	FVF_Tangent,
	FVF_Binormal,
	FVF_MAX
};

//
//	FVertexComponent
//
struct FVertexComponent
{
	EComponentType			Type;
	EFixedVertexFunction	Function;

	FVertexComponent()
	{
	}

	FVertexComponent(EComponentType InType,EFixedVertexFunction InFunction)
	{
		Type = InType;
		Function = InFunction;
	}
};

enum { MAX_VERTEX_COMPONENTS = 8 };

//
//	FVertexStream
//	Abstract vertex stream interface.
//
class FVertexStream : public FRenderResource
{
public:

	virtual INT GetStride() = 0;
	virtual INT GetSize() = 0;
	virtual UBOOL HintDynamic() { return 0; }
	virtual UBOOL UseNPatches() { return 0; }

	virtual INT GetComponents(FVertexComponent* Components) = 0;

	virtual void GetStreamData(void* Dest) = 0;
	virtual void GetRawStreamData(void ** Dest, INT FirstVertex ) = 0;
};

//
//	FIndexBuffer
//	Abstract index buffer interface.
//
class FIndexBuffer : public FRenderResource
{
public:

	virtual INT GetSize() = 0;

	virtual void GetContents(void* Data) = 0;

	virtual INT GetIndexSize() = 0;
};

//
// ETextureFormat
//
enum ETextureFormat{
	TEXF_P8        = 0x00,
	TEXF_RGBA7     = 0x01,
	TEXF_RGB16     = 0x02,
	TEXF_DXT1      = 0x03,
	TEXF_RGB8      = 0x04,
	TEXF_RGBA8     = 0x05,
	TEXF_NODATA    = 0x06,
	TEXF_DXT3      = 0x07,
	TEXF_DXT5      = 0x08,
	TEXF_L8        = 0x09,
	TEXF_G16       = 0x0a,
	TEXF_RRRGGGBBB = 0x0b,
	TEXF_V8U8      = 0x0c,
	TEXF_LIN_RGB8  = 0x0d,
	TEXF_L6V5U5    = 0x0e,
	TEXF_X8L8V8U8  = 0x0f,
	TEXF_MAX       = 0xff
};

//
// ETexClampMode
//
enum ETexClampMode{
	TC_Wrap,
	TC_Clamp,
	TC_MAX
};

//
//  IsDXTC
//
inline UBOOL IsDXTC( ETextureFormat Format ){
	return ( (Format == TEXF_DXT1) || (Format == TEXF_DXT3) || (Format == TEXF_DXT5) );
}


//
// GetBytesPerPixel
//
inline INT GetBytesPerPixel(ETextureFormat Format, INT NumPixels){
	switch(Format){
	case TEXF_DXT1:
		return NumPixels / 2;
	case TEXF_RGBA8:
	case TEXF_RGBA7:
	case TEXF_LIN_RGB8:
	case TEXF_X8L8V8U8:
		return NumPixels * 4;
	case TEXF_P8:
	case TEXF_DXT3:
	case TEXF_DXT5:
	case TEXF_L8:
		return NumPixels;
	case TEXF_RGB16:
	case TEXF_G16:
	case TEXF_V8U8:
	case TEXF_L6V5U5:
		return NumPixels * 2;
	case TEXF_RGB8:
		return NumPixels * 3;
	};

	return 0;
}

//
// CalculateTexelPointer
//
inline BYTE* CalculateTexelPointer(BYTE* Base, ETextureFormat Format, INT Stride, INT X, INT Y){
	switch(Format){
	case TEXF_DXT1:
		return Base + Stride * (Y / 4) + (X * 2);
	case TEXF_DXT3:
	case TEXF_DXT5:
		return Base + Stride * (Y / 4) + (X * 4);
	case TEXF_RGBA8:
	case TEXF_RGBA7:
		return Base + Stride * Y + X * 4;
	case TEXF_RGB8:
	case TEXF_RRRGGGBBB:
	case TEXF_LIN_RGB8:
	case TEXF_X8L8V8U8:
		return Base + Stride * Y + X * 3;
	case TEXF_RGB16:
	case TEXF_G16:
	case TEXF_V8U8:
	case TEXF_L6V5U5:
		return Base + Stride * Y + X * 2;
	case TEXF_P8:
	case TEXF_L8:
		return Base + Stride * Y + X;
	};

	return Base;
}

//
// Base texture interface.
//
class FBaseTexture : public FRenderResource{
public:
	virtual FBaseTexture* GetBaseTextureInterface() { return this; }
	virtual FCubemap* GetCubemapInterface() { return NULL; }
	virtual FTexture* GetTextureInterface() { return NULL; }
	virtual FCompositeTexture* GetCompositeTextureInterface() { return NULL; }
	virtual FRenderTarget* GetRenderTargetInterface() { return NULL; }

	virtual INT GetWidth() = 0;
	virtual INT GetHeight() = 0;
	virtual INT GetFirstMip() = 0;
	virtual INT GetNumMips() = 0;
	virtual ETextureFormat GetFormat() = 0;

	// temporary
	virtual ETexClampMode GetUClamp() = 0;
	virtual ETexClampMode GetVClamp() = 0;
};

//
// FTexture
//
class FTexture : public FBaseTexture
{
public:

	virtual FTexture* GetTextureInterface() { return this; }

	virtual void* GetRawTextureData(INT MipIndex) = 0;
	virtual void GetTextureData(INT MipIndex,void* Dest,INT DestStride,ETextureFormat DestFormat,UBOOL ColoredMips=0) = 0;
	virtual void UnloadRawTextureData( INT MipIndex ) = 0;
	virtual UTexture* GetUTexture() = 0;
};

//
// FCompositeTexture
//
class FCompositeTexture : public FBaseTexture
{
public:

	virtual FCompositeTexture* GetCompositeTextureInterface() { return this; }

	virtual INT GetNumChildren() = 0;
	virtual FTexture* GetChild(INT ChildIndex,INT* OutChildX,INT* OutChildY) = 0;
};

//
// FCubemap
//
class FCubemap : public FBaseTexture
{
public:

	virtual FCubemap* GetCubemapInterface() { return this; }

	virtual FTexture* GetFace( INT Face ) = 0;
};

//
// FRenderTarget
//
class FRenderTarget : public FBaseTexture
{
public:

	virtual FRenderTarget* GetRenderTargetInterface() { return this; }
};


/*************************************************************************************
	Backward compatibility code.
**************************************************************************************/

/*------------------------------------------------------------------------------------
	Forward declarations.
------------------------------------------------------------------------------------*/

class		URenderResource;
class   	UVertexBuffer;
class		USkinVertexBuffer;
class   	UIndexBuffer;

/*------------------------------------------------------------------------------------
	URenderResource.
------------------------------------------------------------------------------------*/
class ENGINE_API URenderResource : public UObject
{
	DECLARE_ABSTRACT_CLASS(URenderResource,UObject,0,Engine)

	INT Revision;

	// UObject interface.
	void Serialize(FArchive& Ar);
};

/*------------------------------------------------------------------------------------
	UVertexStreamBase.
------------------------------------------------------------------------------------*/

enum EStreamType
{
	ST_MISC				= 0x0,
	ST_VECTOR3			= 0x1,
	ST_COLOR			= 0x2,
	ST_UV2				= 0x3,
	ST_FVF				= 0x4,
	ST_POSNORMTEX		= 0x5,
};

class ENGINE_API UVertexStreamBase: public URenderResource
{
	DECLARE_ABSTRACT_CLASS(UVertexStreamBase,URenderResource,0,Engine)

	INT		ItemSize;
	DWORD	PolyFlags;	// How we intend to render this VB.
	DWORD	StreamType;

	// UVertexStreamBase interface
	UVertexStreamBase( INT InItemSize, DWORD InPolyFlags, DWORD InStreamType )
		: ItemSize(InItemSize), PolyFlags(InPolyFlags), StreamType(InStreamType) {};
	virtual void* GetData()=0;
	virtual INT GetDataSize()=0;
	void SetPolyFlags( DWORD InPolyFlags ){
		if( PolyFlags != InPolyFlags )
			Revision++;
		PolyFlags = InPolyFlags;
	}

	// UObject interface
	void Serialize(FArchive& Ar){
		Super::Serialize(Ar);
		if( Ar.Ver() >= 75 )
			Ar << ItemSize << PolyFlags << StreamType;
	}
};

/*------------------------------------------------------------------------------------
	UVertexBuffer.
------------------------------------------------------------------------------------*/
struct FUntransformedVertex{
	FVector	Position,
			Normal;
	DWORD	Color;
	FLOAT	U,
			V;
	FLOAT	U2,
			V2;
	FUntransformedVertex(){}
	FUntransformedVertex(FVector InPosition){
		Position = InPosition;
	}

	// Serialization.
	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FUntransformedVertex& V)
	{
		return Ar
			<< V.Position
			<< V.Normal
			<< V.Color
			<< V.U
			<< V.V
			<< V.U2
			<< V.V2;
	}
};

class ENGINE_API UVertexBuffer : public UVertexStreamBase{
	DECLARE_CLASS(UVertexBuffer,UVertexStreamBase,0,Engine)

	TArray<FUntransformedVertex>	Vertices;

	static URenderDevice* RenderDevice;

	UVertexBuffer() : UVertexStreamBase( sizeof(FUntransformedVertex), 0, ST_FVF )  {}
	UVertexBuffer( DWORD InPolyFlags ) : UVertexStreamBase( sizeof(FUntransformedVertex), InPolyFlags, ST_MISC) {}

	// UVertexStream Interface
	void* GetData() { return &Vertices[0]; }
	INT GetDataSize() { return Vertices.Num() * sizeof(FUntransformedVertex); }

	// UObject Interface
	virtual void Serialize(FArchive& Ar){
		guard(UVertexBuffer::Serialize);

		Super::Serialize(Ar);
		Ar << Vertices;

		if(Ar.Ver() >= 73 && Ar.Ver() <= 74)
			Ar << PolyFlags;

		unguard;
	}
};

/*------------------------------------------------------------------------------------
	UVertexStreamVECTOR.
------------------------------------------------------------------------------------*/

class ENGINE_API UVertexStreamVECTOR : public UVertexStreamBase
{
	DECLARE_CLASS(UVertexStreamVECTOR,UVertexStreamBase,0,Engine)

	TArray<FVector>	VectorData;

	UVertexStreamVECTOR() : UVertexStreamBase( sizeof(FVector), 0, ST_VECTOR3 )  {}
	UVertexStreamVECTOR( DWORD InPolyFlags ) : UVertexStreamBase( sizeof(FVector), InPolyFlags, ST_VECTOR3) {}

	// UVertexStream Interface
	void* GetData() { return &VectorData[0]; }
	INT GetDataSize() { return VectorData.Num() * sizeof(FVector); }

	// UObject Interface
	virtual void Serialize(FArchive& Ar)
	{
		guard(UVertexStreamVECTOR::Serialize);
		Super::Serialize(Ar);
		Ar << VectorData;
		unguard;
	}
};

/*------------------------------------------------------------------------------------
	UVertexStreamCOLOR.
------------------------------------------------------------------------------------*/

class FColorHack
{
public:

#if __INTEL_BYTE_ORDER__
#ifdef __PSX2_EE__
	BYTE R,G,B,A;
#else
	BYTE B,G,R,A;
#endif
#else
	BYTE A,R,G,B;
#endif

	DWORD& DWColor(void) {return *((DWORD*)this);}
	const DWORD& DWColor(void) const {return *((DWORD*)this);}

	FColorHack()
	{
	}

	FColorHack(BYTE InR,BYTE InG,BYTE InB,BYTE InA)
	{
		R = InR;
		G = InG;
		B = InB;
		A = InA;
	}

	FColorHack(BYTE InR,BYTE InG,BYTE InB)
	{
		R = InR;
		G = InG;
		B = InB;
		A = 255;
	}

	friend FArchive& operator<<(FArchive& Ar,FColorHack& Color)
	{
		return Ar << Color.R << Color.G << Color.B << Color.A;
	}
};

class ENGINE_API UVertexStreamCOLOR : public UVertexStreamBase
{
	DECLARE_CLASS(UVertexStreamCOLOR,UVertexStreamBase,0,Engine)

	TArray<FColorHack>	ColorData;

	UVertexStreamCOLOR() : UVertexStreamBase( sizeof(FColorHack), 0, ST_COLOR )  {}
	UVertexStreamCOLOR( DWORD InPolyFlags ) : UVertexStreamBase( sizeof(FColorHack), InPolyFlags, ST_COLOR) {}

	// UVertexStream Interface
	void* GetData() { return &ColorData[0]; }
	INT GetDataSize() { return ColorData.Num() * sizeof(FColorHack); }

	// UObject Interface
	virtual void Serialize(FArchive& Ar)
	{
		guard(UVertexStreamCOLOR::Serialize);
		Super::Serialize(Ar);
		Ar << ColorData;
		unguard;
	}
};

/*------------------------------------------------------------------------------------
	UVertexStreamUV.
------------------------------------------------------------------------------------*/

struct FUV2Data
{
	FLOAT U;
	FLOAT V;
	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FUV2Data& UVData)
	{
		return Ar << UVData.U << UVData.V;
	}
};

class ENGINE_API UVertexStreamUV : public UVertexStreamBase
{
	DECLARE_CLASS(UVertexStreamUV,UVertexStreamBase,0,Engine)

	TArray<FUV2Data>	UVData;

	UVertexStreamUV() : UVertexStreamBase( sizeof(FUV2Data), 0, ST_UV2 )  {}
	UVertexStreamUV( DWORD InPolyFlags ) : UVertexStreamBase( sizeof(FUV2Data), InPolyFlags, ST_UV2) {}

	// UVertexStream Interface
	void* GetData() { return &UVData[0]; }
	INT GetDataSize() { return UVData.Num() * sizeof(FUV2Data); }

	// UObject Interface
	virtual void Serialize(FArchive& Ar)
	{
		guard(UVertexStreamUV::Serialize);
		Super::Serialize(Ar);
		Ar << UVData;
		unguard;
	}
};

/*------------------------------------------------------------------------------------
	UVertexStreamPosNormTex
------------------------------------------------------------------------------------*/

struct FPosNormTexData
{
	FVector	Position,
			Normal;
	FLOAT	U,
			V;
	FLOAT	U2,
			V2;

	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FPosNormTexData& V)
	{
		return Ar
			<< V.Position
			<< V.Normal
			<< V.U
			<< V.V
			<< V.U2
			<< V.V2;
	}
};

class ENGINE_API UVertexStreamPosNormTex : public UVertexStreamBase
{
	DECLARE_CLASS(UVertexStreamPosNormTex,UVertexStreamBase,0,Engine)

	TArray<FPosNormTexData>	Data;

	UVertexStreamPosNormTex() : UVertexStreamBase( sizeof(FPosNormTexData), 0, ST_POSNORMTEX )  {}
	UVertexStreamPosNormTex( DWORD InPolyFlags ) : UVertexStreamBase( sizeof(FPosNormTexData), InPolyFlags, ST_POSNORMTEX) {}

	// UVertexStream Interface
	void* GetData() { return &Data[0]; }
	INT GetDataSize() { return Data.Num() * sizeof(FPosNormTexData); }

	// UObject Interface
	virtual void Serialize(FArchive& Ar)
	{
		guard(UVertexStreamPosNormTex::Serialize);
		Super::Serialize(Ar);
		Ar << Data;
		unguard;
	}
};




/*------------------------------------------------------------------------------------
	USkinVertexBuffer.
------------------------------------------------------------------------------------*/
/*
			D3DVSD_REG(   , D3DVSDT_FLOAT3 ),  // Position
			D3DVSD_REG(   , D3DVSDT_FLOAT3 ),  // Normal
			D3DVSD_REG(   , D3DVSDT_FLOAT2 ),  // Tex coords
			D3DVSD_REG(   , D3DVSDT_RGBA4 ),   // Weight 0, 1, 2, 3
			D3DVSD_REG(   , D3DVSDT_FLOAT4 ),  // Index  0, 1, 2, 3
*/
#if SUPPORTS_PRAGMA_PACK
#pragma pack(push,1)
#endif
//!!vogel: TODO: GCC_PACK
struct FSkinVertex
{
	FVector	Position;  // 3d  12
	FVector Normal;    // 3d  12
 	FLOAT	U,V;	   // 2d   8
	FLOAT   Index[4];  // DWORD   BoneIndices; // total
	FLOAT	Weight[4]; // 4d  16 // 4 weights...

	FSkinVertex()
	{}
	FSkinVertex( FVector InPosition )
	{
		Position = InPosition;
	}
	// Serialization.
	ENGINE_API friend FArchive& operator<<(FArchive& Ar,FSkinVertex& V)
	{
		return Ar
			<< V.Position
			<< V.Normal
			<< V.U
			<< V.V
			<< V.Index[0]
			<< V.Index[1]
			<< V.Index[2]
			<< V.Index[3]
			<< V.Weight[0]
			<< V.Weight[1]
			<< V.Weight[2]
			<< V.Weight[3];
	}
};
#if SUPPORTS_PRAGMA_PACK
#pragma pack(pop)
#endif

class ENGINE_API USkinVertexBuffer : public URenderResource
{
	DECLARE_CLASS(USkinVertexBuffer,URenderResource,0,Engine)

	TArray<FSkinVertex>	Vertices;

	// UObject interface.
	void Serialize(FArchive& Ar);
};



/*------------------------------------------------------------------------------------
	UIndexBuffer.
------------------------------------------------------------------------------------*/
class ENGINE_API UIndexBuffer : public URenderResource
{
	DECLARE_CLASS(UIndexBuffer,URenderResource,0,Engine)

	TArray<_WORD>	Indices;

	static URenderDevice* RenderDevice;

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
};

#endif

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
