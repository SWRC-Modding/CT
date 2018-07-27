/*=============================================================================
	FFileManagerGeneric.h: Unreal generic file manager support code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	This base class simplifies FFileManager implementations by providing
	simple, unoptimized implementations of functions whose implementations
	can be derived from other functions.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	File Manager.
-----------------------------------------------------------------------------*/

class FFileManagerGeneric : public FFileManager
{
public:
	INT FileSize( const TCHAR* Filename )
	{
		// Create a generic file reader, get its size, and return it.
		guard(FFileManagerGeneric::FileSize);
		FArchive* Ar = CreateFileReader( Filename );
		if( !Ar )
			return -1;
		INT Result = Ar->TotalSize();
		delete Ar;
		return Result;
		unguard;
	}
	UBOOL Copy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, void (*Progress)(FLOAT Fraction) )
	{
		// Direct file copier.
		guard(FFileManagerGeneric::Copy);
		UBOOL Success = 0;
		if( Progress )
			Progress( 0.0f );
		FArchive* Src = CreateFileReader( SrcFile );
		if( Src )
		{
			INT Size = Src->TotalSize();
			FArchive* Dest = CreateFileWriter( DestFile, (ReplaceExisting?0:FILEWRITE_NoReplaceExisting) | (EvenIfReadOnly?FILEWRITE_EvenIfReadOnly:0) );
			if( Dest )
			{
				BYTE Buffer[4096];
				for( INT Total=0; Total<Size; Total+=sizeof(Buffer) )
				{
					INT Count = Min( Size-Total, (INT)sizeof(Buffer) );
					Src->Serialize( Buffer, Count );
					if( Src->IsError() )
						break;
					Dest->Serialize( Buffer, Count );
					if( Dest->IsError() )
						break;
					if( Progress )
						Progress( (FLOAT)Total / Size );
				}
				Success = Dest->Close();
				delete Dest;
				if( !Success )
					Delete( DestFile );
			}
			Success = Success && Src->Close();
			delete Src;
		}
		if( Progress )
			Progress( 1.0 );
		return Success;
		unguard;
	}
	UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 )
	{
		// Support code for making a directory tree.
		guard(FFileManagerGeneric::MakeDirectory);
		check(Tree);
		INT SlashCount=0, CreateCount=0;
		for( TCHAR Full[256]=TEXT(""), *Ptr=Full; ; *Ptr++=*Path++ )
		{
			if( *Path==PATH_SEPARATOR[0] || *Path==0 )
			{
				if( SlashCount++>0 && !IsDrive(Full) )
				{
					*Ptr = 0;
					if( !MakeDirectory( Full, 0 ) )
						return 0;
					CreateCount++;
				}
			}
			if( *Path==0 )
				break;
		}
		return CreateCount!=0;
		unguard;
	}
	UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 )
	{
		// Support code for removing a directory tree.
		guard(FFileManagerGeneric::DeleteDirectory);
		check(Tree);
		if( !appStrlen(Path) )
			return 0;
		FString Spec = FString(Path) * TEXT("*");
		TArray<FString> List = FindFiles( *Spec, 1, 0 );
		for( INT i=0; i<List.Num(); i++ )
			if( !Delete(*(FString(Path) * List(i)),1,1) )
				return 0;
		List = FindFiles( *Spec, 0, 1 );
		for( i=0; i<List.Num(); i++ )
			if( !DeleteDirectory(*(FString(Path) * List(i)),1,1) )
				return 0;
		return DeleteDirectory( Path, RequireExists, 0 );
		unguard;

	}
	UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL ReplaceExisting=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 )
	{
		// Move file manually.
		guard(FFileManagerGeneric::Move);
		if( !Copy(Dest,Src,ReplaceExisting,EvenIfReadOnly,Attributes,NULL) )
			return 0;
		Delete( Src, 1, 1 );
		return 1;
		unguard;
	}
private:
	UBOOL IsDrive( const TCHAR* Path )
	{
		// Does Path refer to a drive letter or BNC path?
		guard(FFileManagerGeneric::IsDrive);
		if( appStricmp(Path,TEXT(""))==0 )
			return 1;
		else if( appToUpper(Path[0])!=appToLower(Path[0]) && Path[1]==':' && Path[2]==0 )
			return 1;
		else if( appStricmp(Path,TEXT("\\"))==0 )
			return 1;
		else if( appStricmp(Path,TEXT("\\\\"))==0 )
			return 1;
		else if( Path[0]=='\\' && Path[1]=='\\' && !appStrchr(Path+2,'\\') )
			return 1;
		else if( Path[0]=='\\' && Path[1]=='\\' && appStrchr(Path+2,'\\') && !appStrchr(appStrchr(Path+2,'\\')+1,'\\') )
			return 1;
		else
			return 0;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
