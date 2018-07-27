/*=============================================================================
	FFileManagerArc.cpp: Unreal archive-based file manager.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Archives.
-----------------------------------------------------------------------------*/

// Archives.
enum {ARCHIVE_MAGIC=0x9fe3c5a3};
enum {ARCHIVE_HEADER_SIZE=5*4};
enum {ARCHIVE_VERSION=1};
enum EArchiveFlags
{
	ARCHIVEF_Bootstrap      = 0x00000001,
	ARCHIVEF_Compressed     = 0x00000004,
};
struct FArchiveItem
{
	FString _Filename_;
	DWORD   Offset;
	DWORD   Size;
	DWORD	Flags;
	FArchiveItem()
	{}
	FArchiveItem( const TCHAR* InFilename, DWORD InOffset, DWORD InSize, DWORD InFlags )
	: _Filename_(InFilename), Offset(InOffset), Size(InSize), Flags(InFlags)
	{}
	friend FArchive& operator<<( FArchive& Ar, FArchiveItem& Item )
	{
		guard(FArchiveItem<<);
		return Ar << Item._Filename_ << Item.Offset << Item.Size << Item.Flags;
		unguard;
	}
};
struct FArchiveHeader
{
	INT Magic, TableOffset, FileSize, Ver, CRC;
	TArray<FArchiveItem> _Items_;
	FArchiveHeader()
	: Magic(ARCHIVE_MAGIC), TableOffset(-1), FileSize(0), Ver(ARCHIVE_VERSION), CRC(0)
	{}
	friend FArchive& operator<<( FArchive& Ar, FArchiveHeader& Head )
	{
		guard(FArchiveHeader<<);
		//warning: Must match ARCHIVE_HEADER_SIZE.
		return Ar << Head.Magic << Head.TableOffset << Head.FileSize << Head.Ver << Head.CRC;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	File manager interceptor.
-----------------------------------------------------------------------------*/

// Cannonicize file name character.
inline TCHAR appToNormal(TCHAR c)
{
	return c=='\\' ? '/' : appToUpper(c);
}

// Wildcard matching test.
inline UBOOL WildcardMatch( const TCHAR* Pattern, const TCHAR* String )
{
	guard(WildcardMatch);
	for( ; *Pattern!='*'; String++,Pattern++ )
		if( appToNormal(*String)!=appToNormal(*Pattern) )
			return 0;
		else if( *Pattern==0 )
			return 1;
	do
		if( WildcardMatch(Pattern+1,String) )
			return 1;
	while( *String++ );
	return 0;
	unguard;
}

// Hack because there isn't a well defined "current working directory" idea with archives.
inline FString ToArcFilename( const TCHAR* Filename )
{
	guard(ToArcFilename);
	return 
		(Filename[0]=='.' && Filename[1]=='.' && (Filename[2]=='\\' || Filename[2]=='/'))
	?	(Filename+3)
	:	(FString(TEXT("System")) * Filename);
	unguard;
}
inline FString FromArcFilename( const TCHAR* Filename )
{
	guard(FromArcFilename);
	FString Found = Filename;
	return
		(Found.Left(7)==TEXT("System") PATH_SEPARATOR)
	?	Found.Mid(7)
	:	Found = FString(TEXT("..")) * Found;
	unguard;
}

// File manager.
class FFileManagerArc : public FFileManager
{
	// File reader.
	class FFileReaderArc : public FArchive
	{
	public:
		FFileReaderArc(FFileManagerArc* InMgr,INT InBase,INT InSize)
		: Mgr(InMgr), Base(InBase), Size(InSize), Pos(0)
		{
			ArIsLoading = ArIsPersistent = 1;
		}
		~FFileReaderArc()
		{
			if(Mgr && Mgr->Cur==this)
				Mgr->Cur = NULL;
		}
		INT Tell()
		{
			return Pos;
		}
		INT TotalSize()
		{
			return Size;
		}
		//!!void Precache(INT HintCount)
		void Seek(INT InPos)
		{
			check(InPos>=0 && InPos<=Size);
			Pos = InPos;
			if(Mgr->Cur==this)
				Mgr->Ar->Seek(Base+InPos);
		}
		void Serialize(void* V,INT Length)
		{
			check(Pos+Length<=Size);
			if(Mgr->Cur!=this)
			{
				Mgr->Cur=this;
				Mgr->Ar->Seek(Base+Pos);
			}
			Mgr->Ar->Serialize(V,Length);
			Pos += Length;
		}
		FFileManagerArc* Mgr;
		INT Base, Size, Pos;
	};
public:
	FFileManagerArc(FFileManager* InFM,const TCHAR* InWad,UBOOL InVerify)
	: FM(InFM), Wad(InWad), Header(), Ar(NULL), Cur(NULL), Verify(InVerify)
	{}
	FFileManager*    FM;
	const TCHAR*     Wad;
	FArchiveHeader   Header;
	FArchive*        Ar;
	FFileReaderArc*  Cur;
	UBOOL            Verify;
	FArchiveItem* Lookup( const TCHAR* Filename )
	{
		FString Find = ToArcFilename( Filename );
		for( INT i=0; i<Header._Items_.Num(); i++ )
			if( WildcardMatch(*Find,*Header._Items_(i)._Filename_) )
				return &Header._Items_(i);
		return NULL;
	}

	// FFileManager interface.
	void Init(UBOOL Startup)
	{
		FM->Init(Startup);

		// Check size.
		INT RealSize = FM->FileSize(Wad);
		if( RealSize<=0 )
			appErrorf( TEXT("Can't find module %s"), Wad );

		// Read the module's header.
		Ar = FM->CreateFileReader(Wad);
		check(Ar);
		INT HeaderPos = Ar->TotalSize()-ARCHIVE_HEADER_SIZE;
		Ar->Seek( HeaderPos );
		*Ar << Header;

		// Verify the module's correctness.
		if( Ar->IsError() || Header.Magic!=ARCHIVE_MAGIC || Header.Ver!=ARCHIVE_VERSION || Header.FileSize!=RealSize )
			appErrorf( TEXT("The module %s is incomplete -- probably due to an incomplete or failed download"), Wad );

		// Verify the module's CRC, unless we're an executable (in which case the sfx already did that).
		if( Verify && FString(Wad).Right(4)!=TEXT(".exe") )
		{
			Ar->Seek( 0 );
			INT CRC=0;
			BYTE Buffer[16384];
			for( INT i=0; i<HeaderPos; i+=sizeof(Buffer) )
			{
				INT Count = Min<INT>( HeaderPos-i, sizeof(Buffer) );
				Ar->Serialize( Buffer, Count );
				if( Ar->IsError() )
					appErrorf( TEXT("The module %s can't be read"), Wad );
				CRC = appMemCrc( Buffer, Count, CRC );
			}
			if( CRC!=Header.CRC )
				appErrorf( TEXT("The module %s is corrupt -- probably due to an incomplete or corrupt download"), Wad );
		}

		// Read the module's directory.
		Ar->Seek( Header.TableOffset );
		*Ar << Header._Items_;
		check(!Ar->IsError());
	}
	FArchive* CreateFileReader( const TCHAR* Filename, DWORD Flags=0, FOutputDevice* Error=GNull )
	{
		guard(FFileManagerArc::CreateFileReader);
		FArchiveItem* Item=Lookup(Filename);
		return
			Item
		?	new FFileReaderArc(this,Item->Offset,Item->Size)
		:	FM->CreateFileReader(Filename,Flags,Error);
		unguard;
	}
	FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error=GNull )
	{
		return FM->CreateFileWriter( Filename, Flags, Error );
	}
	INT FileSize( const TCHAR* Filename )
	{
		guard(FFileManagerArc::FileSize);
		FArchiveItem* Item=Lookup(Filename);
		return Item ? Item->Size : FM->FileSize(Filename);
		unguard;
	}
	UBOOL Delete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 )
	{
		return FM->Delete( Filename, RequireExists, EvenReadOnly );
	}
	UBOOL Copy( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0, void (*Progress)(FLOAT Fraction)=NULL )
	{
		return FM->Copy( Dest, Src, Replace, EvenIfReadOnly, Attributes, Progress );
	}
	UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 )
	{
		return FM->Move( Dest, Src, Replace, EvenIfReadOnly, Attributes );
	}
	SQWORD GetGlobalTime( const TCHAR* Filename )
	{
		return FM->GetGlobalTime( Filename );
	}
	UBOOL SetGlobalTime( const TCHAR* Filename )
	{
		return FM->SetGlobalTime( Filename );
	}
	UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 )
	{
		return FM->MakeDirectory( Path, Tree );
	}
	UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 )
	{
		return FM->DeleteDirectory( Path, RequireExists, Tree );
	}
	TArray<FString> FindFiles( const TCHAR* Filename, UBOOL Files, UBOOL Directories )
	{
		TArray<FString> Result = FM->FindFiles( Filename, Files, Directories );
		FString Find = ToArcFilename( Filename );
		for( INT i=0; i<Header._Items_.Num(); i++ )
		{
			FArchiveItem& Item = Header._Items_(i);
			if( WildcardMatch( *Find, *Item._Filename_ ) )
			{
				FString Found = FromArcFilename(*Item._Filename_);
				if( Result.FindItemIndex(*Found)==INDEX_NONE )
					new(Result)FString(*Found);
			}
		}
		return Result;
	}
	UBOOL SetDefaultDirectory( const TCHAR* Filename )
	{
		return FM->SetDefaultDirectory( Filename );
	}
	FString GetDefaultDirectory()
	{
		return FM->GetDefaultDirectory();
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
