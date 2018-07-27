/*=============================================================================
	FFileManagerWindows.h: Unreal Windows OS based file manager.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "FFileManagerGeneric.h"

/*-----------------------------------------------------------------------------
	File Manager.
-----------------------------------------------------------------------------*/

// File manager.
class FArchiveFileReader : public FArchive
{
public:
	FArchiveFileReader( HANDLE InHandle, FOutputDevice* InError, INT InSize )
	:	Handle			( InHandle )
	,	Error			( InError )
	,	Size			( InSize )
	,	Pos				( 0 )
	,	BufferBase		( 0 )
	,	BufferCount		( 0 )
	{
		guard(FArchiveFileReader::FArchiveFileReader);
		ArIsLoading = ArIsPersistent = 1;
		unguard;
	}
	~FArchiveFileReader()
	{
		guard(FArchiveFileReader::~FArchiveFileReader);
		if( Handle )
			Close();
		unguard;
	}
	void Precache( INT HintCount )
	{
		guardSlow(FArchiveFileReader::Precache);
		checkSlow(Pos==BufferBase+BufferCount);
		BufferBase = Pos;
		BufferCount = Min( Min( HintCount, (INT)(ARRAY_COUNT(Buffer) - (Pos&(ARRAY_COUNT(Buffer)-1))) ), Size-Pos );
		INT Count=0;
		ReadFile( Handle, Buffer, BufferCount, (DWORD*)&Count, NULL );
		if( Count!=BufferCount )
		{
			ArIsError = 1;
			Error->Logf( TEXT("ReadFile failed: Count=%i BufferCount=%i Error=%s"), Count, BufferCount, appGetSystemErrorMessage() );
		}
		unguardSlow;
	}
	void Seek( INT InPos )
	{
		guard(FArchiveFileReader::Seek);
		check(InPos>=0);
		check(InPos<=Size);
		if( SetFilePointer( Handle, InPos, 0, FILE_BEGIN )==0xFFFFFFFF )
		{
			ArIsError = 1;
			Error->Logf( TEXT("SetFilePointer Failed %i/%i: %i %s"), InPos, Size, Pos, appGetSystemErrorMessage() );
		}
		Pos         = InPos;
		BufferBase  = Pos;
		BufferCount = 0;
		unguard;
	}
	INT Tell()
	{
		return Pos;
	}
	INT TotalSize()
	{
		return Size;
	}
	UBOOL Close()
	{
		guardSlow(FArchiveFileReader::Close);
		if( Handle )
			CloseHandle( Handle );
		Handle = NULL;
		return !ArIsError;
		unguardSlow;
	}
	void Serialize( void* V, INT Length )
	{
		guardSlow(FArchiveFileReader::Serialize);
		while( Length>0 )
		{
			INT Copy = Min( Length, BufferBase+BufferCount-Pos );
			if( Copy==0 )
			{
				if( Length >= ARRAY_COUNT(Buffer) )
				{
					INT Count=0;
					ReadFile( Handle, V, Length, (DWORD*)&Count, NULL );
					if( Count!=Length )
					{
						ArIsError = 1;
						Error->Logf( TEXT("ReadFile failed: Count=%i Length=%i Error=%s"), Count, Length, appGetSystemErrorMessage() );
					}
					Pos += Length;
					BufferBase += Length;
					return;
				}
				Precache( MAXINT );
				Copy = Min( Length, BufferBase+BufferCount-Pos );
				if( Copy<=0 )
				{
					ArIsError = 1;
					Error->Logf( TEXT("ReadFile beyond EOF %i+%i/%i"), Pos, Length, Size );
				}
				if( ArIsError )
					return;
			}
			appMemcpy( V, Buffer+Pos-BufferBase, Copy );
			Pos       += Copy;
			Length    -= Copy;
			V          = (BYTE*)V + Copy;
		}
		unguardSlow;
	}
protected:
	HANDLE			Handle;
	FOutputDevice*	Error;
	INT				Size;
	INT				Pos;
	INT				BufferBase;
	INT				BufferCount;
	BYTE			Buffer[1024];
};
class FArchiveFileWriter : public FArchive
{
public:
	FArchiveFileWriter( HANDLE InHandle, FOutputDevice* InError, INT InPos )
	:	Handle		( InHandle )
	,	Error		( InError )
	,	Pos			( InPos )
	,	BufferCount	( 0 )
	{
		ArIsSaving = ArIsPersistent = 1;
	}
	~FArchiveFileWriter()
	{
		guard(FArchiveFileWriter::~FArchiveFileWriter);
		if( Handle )
			Close();
		Handle = NULL;
		unguard;
	}
	void Seek( INT InPos )
	{
		Flush();
		if( SetFilePointer( Handle, InPos, 0, FILE_BEGIN )==0xFFFFFFFF )
		{
			ArIsError = 1;
			Error->Logf( LocalizeError("SeekFailed",TEXT("Core")) );
		}
		Pos = InPos;
	}
	INT Tell()
	{
		return Pos;
	}
	UBOOL Close()
	{
		guardSlow(FArchiveFileWriter::Close);
		Flush();
		if( Handle && !CloseHandle(Handle) )
		{
			ArIsError = 1;
			Error->Logf( LocalizeError("WriteFailed",TEXT("Core")) );
		}
		return !ArIsError;
		unguardSlow;
	}
	void Serialize( void* V, INT Length )
	{
		Pos += Length;
		INT Copy;
		while( Length > (Copy=ARRAY_COUNT(Buffer)-BufferCount) )
		{
			appMemcpy( Buffer+BufferCount, V, Copy );
			BufferCount += Copy;
			Length      -= Copy;
			V            = (BYTE*)V + Copy;
			Flush();
		}
		if( Length )
		{
			appMemcpy( Buffer+BufferCount, V, Length );
			BufferCount += Length;
		}
	}
	void Flush()
	{
		if( BufferCount )
		{
			INT Result=0;
			if( !WriteFile( Handle, Buffer, BufferCount, (DWORD*)&Result, NULL ) )
			{
				ArIsError = 1;
				Error->Logf( LocalizeError("WriteFailed",TEXT("Core")) );
			}
		}
		BufferCount = 0;
	}
protected:
	HANDLE			Handle;
	FOutputDevice*	Error;
	INT				Pos;
	INT				BufferCount;
	BYTE			Buffer[4096];
};
class FFileManagerWindows : public FFileManagerGeneric
{
public:
	FArchive* CreateFileReader( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error )
	{
		guard(FFileManagerWindows::CreateFileReader);
		DWORD  Access    = GENERIC_READ;
		DWORD  WinFlags  = FILE_SHARE_READ;
		DWORD  Create    = OPEN_EXISTING;
		HANDLE Handle    = TCHAR_CALL_OS( CreateFileW( Filename, Access, WinFlags, NULL, Create, FILE_ATTRIBUTE_NORMAL, NULL ), CreateFileA( TCHAR_TO_ANSI(Filename), Access, WinFlags, NULL, Create, FILE_ATTRIBUTE_NORMAL, NULL ) );
		if( Handle==INVALID_HANDLE_VALUE )
		{
			if( Flags & FILEREAD_NoFail )
				appErrorf( TEXT("Failed to read file: %s"), Filename );
			return NULL;
		}
		return new(TEXT("WindowsFileReader"))FArchiveFileReader(Handle,Error,GetFileSize(Handle,NULL));
		unguard;
	}
	FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error )
	{
		guard(FFileManagerWindows::CreateFileWriter);
		if( Flags & FILEWRITE_EvenIfReadOnly )
			TCHAR_CALL_OS(SetFileAttributesW(Filename, 0),SetFileAttributesA(TCHAR_TO_ANSI(Filename), 0));
		DWORD  Access    = GENERIC_WRITE;
		DWORD  WinFlags  = (Flags & FILEWRITE_AllowRead) ? FILE_SHARE_READ : 0;
		DWORD  Create    = (Flags & FILEWRITE_Append) ? OPEN_ALWAYS : (Flags & FILEWRITE_NoReplaceExisting) ? CREATE_NEW : CREATE_ALWAYS;
		HANDLE Handle    = TCHAR_CALL_OS( CreateFileW( Filename, Access, WinFlags, NULL, Create, FILE_ATTRIBUTE_NORMAL, NULL ), CreateFileA( TCHAR_TO_ANSI(Filename), Access, WinFlags, NULL, Create, FILE_ATTRIBUTE_NORMAL, NULL ) );
		INT    Pos       = 0;
		if( Handle==INVALID_HANDLE_VALUE )
		{
			if( Flags & FILEWRITE_NoFail )
				appErrorf( TEXT("Failed to create file: %s"), Filename );
			return NULL;
		}
		if( Flags & FILEWRITE_Append )
			Pos = SetFilePointer( Handle, 0, 0, FILE_END );
		return new(TEXT("WindowsFileWriter"))FArchiveFileWriter(Handle,Error,Pos);
		unguard;
	}
	INT FileSize( const TCHAR* Filename )
	{
		guard(FFileManagerWindows::FileSize);
		HANDLE Handle = TCHAR_CALL_OS( CreateFileW( Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ), CreateFileA( TCHAR_TO_ANSI(Filename), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) );
		if( Handle==INVALID_HANDLE_VALUE )
			return -1;
		DWORD Result = GetFileSize( Handle, NULL );
		CloseHandle( Handle );
		return Result;
		unguard;
	}
	UBOOL Copy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, void (*Progress)(FLOAT Fraction) )
	{
		if( EvenIfReadOnly )
			TCHAR_CALL_OS(SetFileAttributesW(DestFile, 0),SetFileAttributesA(TCHAR_TO_ANSI(DestFile), 0));
		UBOOL Result
		=	Progress
		?	FFileManagerGeneric::Copy( DestFile, SrcFile, ReplaceExisting, EvenIfReadOnly, Attributes, Progress )
		:	TCHAR_CALL_OS(CopyFileW(SrcFile, DestFile, !ReplaceExisting),CopyFileA(TCHAR_TO_ANSI(SrcFile), TCHAR_TO_ANSI(DestFile), !ReplaceExisting))!=0;
		if( Result && !Attributes )
			TCHAR_CALL_OS(SetFileAttributesW(DestFile, 0),SetFileAttributesA(TCHAR_TO_ANSI(DestFile), 0));
		return Result;
	}
	UBOOL Delete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 )
	{
		guard(FFileManagerWindows::Delete);
		if( EvenReadOnly )
			TCHAR_CALL_OS(SetFileAttributesW(Filename,FILE_ATTRIBUTE_NORMAL),SetFileAttributesA(TCHAR_TO_ANSI(Filename),FILE_ATTRIBUTE_NORMAL));
		return TCHAR_CALL_OS(DeleteFile(Filename),DeleteFileA(TCHAR_TO_ANSI(Filename)))!=0 || (!RequireExists && GetLastError()==ERROR_FILE_NOT_FOUND);
		unguard;
	}
	UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 )
	{
		guard(FFileManagerWindows::Move);
		//warning: MoveFileEx is broken on Windows 95 (Microsoft bug).
		Delete( Dest, 0, 1 );
		INT Result = TCHAR_CALL_OS( MoveFileW(Src,Dest), MoveFileA(TCHAR_TO_ANSI(Src),TCHAR_TO_ANSI(Dest)) );
		if( !Result )
			debugf( NAME_Warning, TEXT("Error moving file '%s' to '%s'"), Src, Dest );
		return Result!=0;
		unguard;
	}
	SQWORD GetGlobalTime( const TCHAR* Filename )
	{
		//return grenwich mean time as expressed in nanoseconds since the creation of the universe.
		//time is expressed in meters, so divide by the speed of light to obtain seconds.
		//assumes the speed of light in a vacuum is constant.
		//the file specified by Filename is assumed to be in your reference frame, otherwise you
		//must transform the result by the path integral of the minkowski metric tensor in order to
		//obtain the correct result.
		return 0;
	}
	UBOOL SetGlobalTime( const TCHAR* Filename )
	{
		return 0;//!!
	}
	UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 )
	{
		guard(FFileManagerWindows::MakeDirectory);
		if( Tree )
			return FFileManagerGeneric::MakeDirectory( Path, Tree );
		return TCHAR_CALL_OS( CreateDirectoryW(Path,NULL), CreateDirectoryA(TCHAR_TO_ANSI(Path),NULL) )!=0 || GetLastError()==ERROR_ALREADY_EXISTS;
		unguard;
	}
	UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 )
	{
		guard(FFileManagerWindows::DeleteDirectory);
		if( Tree )
			return FFileManagerGeneric::DeleteDirectory( Path, RequireExists, Tree );
		return TCHAR_CALL_OS( RemoveDirectoryW(Path), RemoveDirectoryA(TCHAR_TO_ANSI(Path)) )!=0 || (!RequireExists && GetLastError()==ERROR_FILE_NOT_FOUND);
		unguard;
	}
	TArray<FString> FindFiles( const TCHAR* Filename, UBOOL Files, UBOOL Directories )
	{
		guard(FFileManagerWindows::FindFiles);
		TArray<FString> Result;
		HANDLE Handle=NULL;
#if UNICODE
		if( GUnicodeOS )
		{
			WIN32_FIND_DATAW Data;
			Handle=FindFirstFileW(Filename,&Data);
			if( Handle!=INVALID_HANDLE_VALUE )
				do
					if
					(	appStricmp(Data.cFileName,TEXT("."))
					&&	appStricmp(Data.cFileName,TEXT(".."))
					&&	((Data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?Directories:Files) )
						new(Result)FString(Data.cFileName);
				while( FindNextFileW(Handle,&Data) );
		}
		else
#endif
		{
			WIN32_FIND_DATAA Data;
			Handle=FindFirstFileA(TCHAR_TO_ANSI(Filename),&Data);
			if( Handle!=INVALID_HANDLE_VALUE )
				do
					if
					(	appStricmp(ANSI_TO_TCHAR(Data.cFileName),TEXT("."))
					&&	appStricmp(ANSI_TO_TCHAR(Data.cFileName),TEXT(".."))
					&&	((Data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?Directories:Files) )
						new(Result)FString(ANSI_TO_TCHAR(Data.cFileName));
				while( FindNextFileA(Handle,&Data) );
		}
		if( Handle )
			FindClose( Handle );
		return Result;
		unguard;
	}
	UBOOL SetDefaultDirectory( const TCHAR* Filename )
	{
		guard(FFileManagerWindows::SetDefaultDirectory);
		return TCHAR_CALL_OS(SetCurrentDirectoryW(Filename),SetCurrentDirectoryA(TCHAR_TO_ANSI(Filename)))!=0;
		unguard;
	}
	FString GetDefaultDirectory()
	{
		guard(FFileManagerWindows::GetDefaultDirectory);
#if UNICODE
		if( GUnicodeOS )
		{
			TCHAR Buffer[1024]=TEXT("");
			GetCurrentDirectoryW(ARRAY_COUNT(Buffer),Buffer);
			return FString(Buffer);
		}
		else
#endif
		{
			ANSICHAR Buffer[1024]="";
			GetCurrentDirectoryA(ARRAY_COUNT(Buffer),Buffer);
			return appFromAnsi( Buffer );
		}
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
