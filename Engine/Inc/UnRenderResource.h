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

class FColor{
public:
	// Variables.
	union { struct{ BYTE B,G,R,A; }; DWORD AlignmentDummy; };

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
class ENGINE_API FRenderResource{
public:
	QWORD CacheId;
	INT Revision;

	FRenderResource() : CacheId(0), Revision(1){}

	virtual ~FRenderResource(){}
	virtual INT GetRevision(){ return Revision; }
	virtual bool IsUMA(){ return false; }
	virtual bool bPersistentCache(){ return false; }

	QWORD GetCacheId(){ return CacheId; }
	void Serialize(FArchive& Ar);
};

//
//	EComponentType
//
enum EComponentType{
	CT_Float4,
	CT_Float3,
	CT_Float2,
	CT_Float1,
	CT_Color,
	CT_MAX
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
struct FVertexComponent{
	BYTE Type;     // EComponentType
	BYTE Function; // EFixedVertexFunction

	FVertexComponent(){}

	FVertexComponent(EComponentType InType,EFixedVertexFunction InFunction){
		Type = InType;
		Function = InFunction;
	}
};

enum { MAX_VERTEX_COMPONENTS = 16 };

//
//	FVertexStream
//	Abstract vertex stream interface.
//
class FVertexStream : public FRenderResource{
public:
	virtual INT GetStride() = 0;
	virtual INT GetSize() = 0;
	virtual UBOOL HintDynamic(){ return 0; }
	virtual UBOOL UseNPatches(){ return 0; }
	virtual INT GetComponents(FVertexComponent* Components) = 0;
	virtual void GetStreamData(void* Dest) = 0;
	virtual void GetRawStreamData(void ** Dest, INT FirstVertex ) = 0;
};

//
//	FIndexBuffer
//	Abstract index buffer interface.
//
class FIndexBuffer : public FRenderResource{
public:
	virtual INT GetSize() = 0;
	virtual void GetContents(void* Data) = 0;
	virtual INT GetIndexSize() = 0;
};

//
// ETextureFormat
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
	TEXF_V8U8,
	TEXF_LIN_RGB8,
	TEXF_L6V5U5,
	TEXF_X8L8V8U8,
	TEXF_MAX
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
	case TEXF_X8L8V8U8:
		return Base + Stride * Y + X * 4;
	case TEXF_RGB8:
	case TEXF_RRRGGGBBB:
	case TEXF_LIN_RGB8:
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
	virtual FBaseTexture* GetBaseTextureInterface(){ return this; }
	virtual FCubemap* GetCubemapInterface(){ return NULL; }
	virtual FTexture* GetTextureInterface(){ return NULL; }
	virtual FCompositeTexture* GetCompositeTextureInterface(){ return NULL; }
	virtual FRenderTarget* GetRenderTargetInterface(){ return NULL; }

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
class FTexture : public FBaseTexture{
public:
	virtual FTexture* GetTextureInterface(){ return this; }
	virtual void* GetRawTextureData(INT MipIndex) = 0;
	virtual void GetTextureData(INT MipIndex,void* Dest,INT DestStride,ETextureFormat DestFormat,UBOOL ColoredMips=0) = 0;
	virtual void UnloadRawTextureData( INT MipIndex ) = 0;
	virtual UTexture* GetUTexture() = 0;
};

//
// FCompositeTexture
//
class FCompositeTexture : public FBaseTexture{
public:
	virtual FCompositeTexture* GetCompositeTextureInterface(){ return this; }
	virtual INT GetNumChildren() = 0;
	virtual FTexture* GetChild(INT ChildIndex,INT* OutChildX,INT* OutChildY) = 0;
};

//
// FCubemap
//
class FCubemap : public FBaseTexture{
public:
	virtual FCubemap* GetCubemapInterface(){ return this; }
	virtual FTexture* GetFace(INT Face) = 0;
};

//
// FRenderTarget
//
class FRenderTarget : public FBaseTexture{
public:
	virtual FRenderTarget* GetRenderTargetInterface(){ return this; }
};

//
// Shader resources
//

class FShader : public FRenderResource{
public:
	FShader(class UHardwareShader* InShader, INT InPass) : Shader(InShader),
	                                                       Pass(InPass){
		CacheId = MakeCacheID(CID_RenderShader);
	}

	void IncRevision(){ ++Revision; }

	class UHardwareShader* GetShader() const{ return Shader; }
	INT GetPass() const{ return Pass; }

private:
	class UHardwareShader* Shader;
	INT Pass;
};


class FPixelShader : public FShader{
public:
	FPixelShader(class UHardwareShader* InHardwareShader, INT InPass) : FShader(InHardwareShader, InPass){}
};


class FVertexShader : public FShader{
public:
	FVertexShader(class UHardwareShader* InHardwareShader, INT InPass) : FShader(InHardwareShader, InPass){}
};

#endif

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
