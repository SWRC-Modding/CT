/*=============================================================================
	FRiffChunk.h: Unreal RIFF standard file management.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Utilities.
-----------------------------------------------------------------------------*/

// Make a "four character code" DWORD from a four-character string.
inline DWORD appFourCC( const TCHAR* Ch )
{
	if( !Ch )
		return 0;
	INT Len=appStrlen(Ch);
	return
		((DWORD)(BYTE)(Len>0?Ch[0]:32) << 0 )
	|	((DWORD)(BYTE)(Len>1?Ch[1]:32) << 8 )
	|	((DWORD)(BYTE)(Len>2?Ch[2]:32) << 16)
	|	((DWORD)(BYTE)(Len>3?Ch[3]:32) << 24);
}

/*-----------------------------------------------------------------------------
	General purpose RIFF chunk types.
-----------------------------------------------------------------------------*/

// Base class of all RIFF chunks.
struct FRiffChunk
{
	FRiffChunk( DWORD InFourCC )
	: FourCC(InFourCC)
	{}
	virtual ~FRiffChunk()
	{
		guard(FRiffChunk::~FRiffChunk);
		for( INT i=0; i<SubChunks.Num(); i++ )
			delete SubChunks( i );
		unguard;
	}
	virtual void SerializeChunk( FArchive& Ar, INT Size )
	{
		guard(FRiffChunk::SerializeChunk);
		if( Ar.IsLoading() )
			Unknown = TArray<BYTE>( Size );
		Ar.Serialize( &Unknown(0), Size );
		unguard;
	}
	virtual FRiffChunk* CreateSubChunk( DWORD FourCC )
	{
		guard(FRiffChunk::CreateSubChunk);
		return new FRiffChunk( FourCC );
		unguard;
	}
	void Load( FArchive& Ar, INT Size )
	{
		guard(FRiffChunk::Load);
		DWORD Tmp[2]={FourCC,0}; debugf(TEXT("FRiffChunk::Read FourCC <%s> Size %08X"),appFromAnsi((ANSICHAR*)Tmp), Size );
		INT DataPos = Ar.Tell();
		SerializeChunk( Ar, Size );

		TArray<BYTE> SubChunkData;
		SubChunkData = TArray<BYTE>( Size + DataPos - Ar.Tell() );
		Ar.Serialize( &SubChunkData(0), SubChunkData.Num() );
		FBufferReader SubAr( SubChunkData );
		while( !SubAr.AtEnd() )
		{
			DWORD       SubFourCC = Arctor<DWORD> ( SubAr     );
			INT         SubSize   = Arctor<INT>   ( SubAr     );
			INT         SubStart  = SubAr.Tell    (           );
			FRiffChunk* SubCh     = CreateSubChunk( SubFourCC );
			if( SubCh )
			{
				SubChunks.AddItem( SubCh );
				SubCh->Load( SubAr, SubSize );
			}
			SubAr.Seek( SubStart + SubSize );
		}
		unguard;
	}
	void Save( FArchive& Ar )
	{
		guard(FRiffChunk::Save);
		Ar << FourCC;
		INT StartPos = Ar.Tell(), Size=0;
		Ar << Size;
		SerializeChunk( Ar, 0 );
		for( INT i=0; i<SubChunks.Num(); i++ )
			SubChunks(i)->Save( Ar );
		INT EndPos = Ar.Tell();
		Size = EndPos - StartPos - sizeof(INT);
		Ar.Seek( StartPos );
		Ar << Size;
		Ar.Seek( EndPos );
		unguard;
	}
	TArray<BYTE> Unknown;
	DWORD FourCC;
	TArray<FRiffChunk*> SubChunks;
};

// Root RIFF chunk class; file types (i.e. WAVE) are subclasses of this.
struct FRiffChunk_RIFF : public FRiffChunk
{
	FRiffChunk_RIFF( DWORD InFileType)
	: FRiffChunk( appFourCC(TEXT("RIFF")) )
	, FileType( InFileType )
	{}
	void SerializeChunk( FArchive& Ar, INT Size )
	{
		DWORD ExpectedRiffFileType = FileType;
		Ar << FileType;
		check(FileType==ExpectedRiffFileType);
	}
	DWORD FileType;
};

/*-----------------------------------------------------------------------------
	WAVE file RIFF chunk types.
-----------------------------------------------------------------------------*/

// WAVE.fmt chunk type.
struct FRiffChunk_fmt : public FRiffChunk
{
	FRiffChunk_fmt()
	: FRiffChunk( appFourCC(TEXT("fmt")) )
	{}
	void SerializeChunk( FArchive& Ar, INT Size )
	{
		Ar << wFormatTag << nChannels << nSamplesPerSec << nAvgBytesPerSec << nBlockAlign << nBitsPerSample;
		debugf(TEXT("wFormatTag: %i"),wFormatTag);
		debugf(TEXT("nChannels: %i"),nChannels);
		debugf(TEXT("nSamplesPerSec: %i"),nSamplesPerSec);
		debugf(TEXT("nAvgBytesPerSec: %i"),nAvgBytesPerSec);
		debugf(TEXT("nBlockAlign: %i"),nBlockAlign);
		debugf(TEXT("nBitsPerSample: %i"),nBitsPerSample);
	}
	_WORD	wFormatTag;
	_WORD	nChannels;
	INT		nSamplesPerSec;
	INT		nAvgBytesPerSec;
	_WORD	nBlockAlign;
	_WORD	nBitsPerSample;
};

// WAVE.data chunk type.
struct FRiffChunk_data : public FRiffChunk
{
	FRiffChunk_data()
	: FRiffChunk( appFourCC(TEXT("data")) )
	{}
	void SerializeChunk( FArchive& Ar, INT Size )
	{
		if( Ar.IsLoading() )
			Bits = TArray<BYTE>( Size );
		Ar.Serialize( &Bits(0), Bits.Num() );
	}
	TArray<BYTE> Bits;
};

// WAVE chunk type.
struct FRiffChunk_WAVE : public FRiffChunk_RIFF
{
	FRiffChunk_WAVE()
	: FRiffChunk_RIFF( appFourCC(TEXT("WAVE")) )
	{}
	void SerializeChunk( FArchive& Ar, INT Size )
	{
		FRiffChunk_RIFF::SerializeChunk( Ar, Size );
	}
	FRiffChunk* CreateSubChunk( DWORD FourCC )
	{
		if( FourCC==appFourCC(TEXT("fmt")) )
			return new FRiffChunk_fmt;
		else if( FourCC==appFourCC(TEXT("data")) )
			return new FRiffChunk_data;
		else
			return new FRiffChunk( FourCC );
	}
};

// Load a RIFF file with a certain root type.
template <class T> T* LoadRiffFile( const TCHAR* Filename, FFileManager* FileManager=GFileManager )
{
	guard(LoadRiffFile);

	// Load file.
	TArray<BYTE> Data;
	verify(appLoadFileToArray( Data, Filename, FileManager ));
	FBufferReader Ar( Data );

	// Import RIFF.
	T* RootChunk = new T;
	DWORD RiffFileFourCC = Arctor<DWORD>( Ar );
	check(RiffFileFourCC==RootChunk->FourCC);
	INT Size = Arctor<INT>( Ar );
	RootChunk->Load( Ar, Size );
	return RootChunk;

	unguard;
}

// Save a RIFF file.
UBOOL SaveRiffFile( FRiffChunk* RootChunk, const TCHAR* Filename, FFileManager* FileManager=GFileManager )
{
	guard(SaveRiffFile);

	// Export RIFF.
	FArchive* Ar = FileManager->CreateFileWriter(Filename);
	if( !Ar )
		return 0;
	RootChunk->Save( *Ar );
	UBOOL Result = Ar->Close();
	delete Ar;
	return Result;

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
