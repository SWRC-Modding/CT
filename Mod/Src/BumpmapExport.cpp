#include "../../Engine/Inc/Engine.h"

/*
 * TGA
 */

#pragma pack(push, 1)
struct FTGAHeader{
	BYTE  IdLength;
	BYTE  ColorMapType;
	BYTE  DataTypeCode;
	_WORD Colormaporigin;
	_WORD Colormaplength;
	BYTE  Colormapdepth;
	_WORD XOrigin;
	_WORD YOrigin;
	_WORD Width;
	_WORD Height;
	BYTE  BitsPerPixel;
	BYTE  ImageDescriptor;
};
#pragma pack(pop)

/*
 * UExportBumpMapsCommandlet
 */

class UExportBumpMapsCommandlet : public UCommandlet{
	DECLARE_CLASS(UExportBumpMapsCommandlet,UCommandlet,0,Mod)
public:
	void StaticConstructor();
	virtual INT Main(const TCHAR* Parms);

private:
	void ExportTga(FTexture* Texture, const FString& Filename);
};

IMPLEMENT_CLASS(UExportBumpMapsCommandlet)

void UExportBumpMapsCommandlet::StaticConstructor(){
	LogToStdout     = 0;
	IsClient        = 1;
	IsEditor        = 1;
	IsServer        = 1;
	LazyLoad        = 1;
	ShowErrorCount  = 0;
}

INT UExportBumpMapsCommandlet::Main(const TCHAR* Parms){
	FString PackageName;

	if(!ParseToken(Parms, PackageName, 0)){
		GWarn->Log("Package file not specified");

		return 1;
	}

	FString OutPath;

	if(!ParseToken(Parms, OutPath, 0))
		OutPath = "..\\Out";

	UObject* Package = LoadPackage(NULL, *PackageName, LOAD_NoFail);
	INT Result = 0;

	if(Package){
		if(GFileManager->MakeDirectory(*OutPath, 1)){
			for(TObjectIterator<UTexture> It; It; ++It){
				FBaseTexture* BaseTexture = It->GetRenderInterface();
				FTexture* Texture = BaseTexture ? BaseTexture->GetTextureInterface() : NULL;

				if(Texture && It->IsIn(Package) && IsBumpmap(Texture->GetFormat())){
					GWarn->Logf("Exporting bump map: %s", It->GetName());
					ExportTga(Texture, OutPath * It->GetName() + ".TGA");
				}
			}
		}else{
			GWarn->Logf("Failed to make directory %s", *OutPath);
			Result = 1;
		}
	}else{
		GWarn->Logf("Failed to load package %s", *PackageName);
		Result = 1;
	}

	return Result;
}

void UExportBumpMapsCommandlet::ExportTga(FTexture* Texture, const FString& Filename){
	check(sizeof(FTGAHeader) == 18);

	ETextureFormat Format = Texture->GetFormat();
	INT Width = Texture->GetWidth();
	INT Height = Texture->GetHeight();

	FTGAHeader Header = {0};

	Header.DataTypeCode = 2;
	Header.Width = static_cast<_WORD>(Width);
	Header.Height = static_cast<_WORD>(Height);
	Header.BitsPerPixel = 32;
	Header.ImageDescriptor = 0x20;

	FBufferArchive Archive;

	Archive.Serialize(&Header, sizeof(Header));
	Archive.Add(Width * Height * 4, false);

	BYTE* Dest = Archive.GetData() + Archive.Tell();
	void* Src = Texture->GetRawTextureData(0);

	switch(Format){
	case TEXF_V8U8:
		ConvertV8U8ToBGRA8(Dest, Src, Width, Height);
		break;
	case TEXF_L6V5U5:
		ConvertL6V5U5ToBGRA8(Dest, Src, Width, Height);
		break;
	case TEXF_X8L8V8U8:
		ConvertX8L8V8U8ToBGRA8(Dest, Src, Width, Height);
	}

	// X and Y are flipped on import in UnrealEd, so it needs to be reverted to prevent incorrect results when reimporting the texture

	INT NumPixels = Width * Height;

	for(INT i = 0; i < NumPixels; ++i)
		Exchange(Dest[i * 4 + 1], Dest[i * 4 + 2]);

	appSaveArrayToFile(Archive, *Filename);
}

