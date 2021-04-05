/*=============================================================================
	UnMovie.h: In-Game Movie Utility
	Copyright 2002 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Chris Linder of Demiurge Studios.
=============================================================================*/

// Codec for movies.
enum ECodecType{
	CODEC_Bink, // Not supported by UD3DRenderDevice
	CODEC_RoQ,
	CODEC_Invalid,
	CODEC_MPEG
};

//
// A movie that is rendered to a texture or the background.
//
class ENGINE_API FMovie{
public:
	FMovie(FString Filename, UBOOL UseSound);

	// Virtual functions
	virtual ~FMovie(){}
	virtual UBOOL Play(UBOOL LoopMovie) = 0;
	virtual void Pause(UBOOL Pause) = 0;
	virtual UBOOL IsPaused() = 0;
	virtual void StopNow() = 0;
	virtual void StopAtEnd() = 0;
	virtual UBOOL IsPlaying(){ return bMoviePlaying; }
	virtual INT GetWidth() = 0;
	virtual INT GetHeight() = 0;
	virtual void PreRender(void* RenderTarget, INT PosX, INT PosY) = 0;
	virtual void RenderToRGBAArray(BYTE* Buffer) = 0;
	virtual void RenderToNative(void* RenderTarget, INT PosX, INT PosY){}
	virtual void RenderToTexture(UTexture* Texture); // Calls RenderToRGBAArray by default
	virtual void Serialize(FArchive& Ar){}

protected:
	UBOOL bMoviePlaying;
	const UBOOL bUseSound;
};

//
// A movie using the RoQ format from the Quake engine.
//
class ENGINE_API FRoQMovie : public FMovie{
public:
	FRoQMovie(FString, UBOOL UseSound, INT FrameRate = 30);

	// Overrides
	virtual ~FRoQMovie();
	virtual UBOOL Play(UBOOL LoopMovie);
	virtual void Pause(UBOOL Pause);
	virtual UBOOL IsPaused();
	virtual void StopNow();
	virtual void StopAtEnd();
	virtual INT GetWidth();
	virtual INT GetHeight();
	virtual void PreRender(void* RenderTarget, INT PosX, INT PosY);
	virtual void RenderToRGBAArray(BYTE*);

protected:
	struct roq_info* ri;
	INT              FrameRate;
	UBOOL            bStopAtEnd;
	UBOOL            bPauseMovie;
	DOUBLE           StartTime;
	DOUBLE           PauseStartTime;
};

class ENGINE_API UMovie : public UObject{
public:
	FMovie* fMovie;

	UMovie();
};
