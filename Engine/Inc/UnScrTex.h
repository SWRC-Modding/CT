/*=============================================================================
	UnScrTex.h: Unreal scripted texture class
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Jack Porter
=============================================================================*/

/*class ENGINE_API  UScriptedTexture : public UTexture{
	DECLARE_CLASS(UScriptedTexture,UTexture,0,Engine)

	// Constructors. Executed on both instantation and reloading of an object.
	UScriptedTexture();

	// UObject interface.	
	void PostLoad();
	void PostEditChange() {PostLoad();}
	void Destroy();

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Tick(FLOAT DeltaSeconds);

	// Variables.
	AActor*			NotifyActor;
    UTexture*		SourceTexture;

	// UScriptedTexture Interface.
	void DrawTile( FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, UTexture *Tex, FTextureInfo *Info, UBOOL bMasked, UBOOL bUseColor=0, BYTE ColorIndex=0 );

	// Natives.
	DECLARE_FUNCTION(execDrawText)
	DECLARE_FUNCTION(execDrawTile)
	DECLARE_FUNCTION(execDrawColoredText)
	DECLARE_FUNCTION(execReplaceTexture)
	DECLARE_FUNCTION(execTextSize)

private:
	UTexture*		OldSourceTex;
	BYTE*			LocalSourceBitmap;
	TMap< UTexture*, TArray< BYTE > >*	PaletteMap;
	FLOAT			LocalTime;
};*/


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
