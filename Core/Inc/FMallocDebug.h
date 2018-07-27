/*=============================================================================
	FMallocDebug.h: Debug memory allocator.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

// Tags.
enum {MEM_PreTag =0xf0ed1cee};
enum {MEM_PostTag=0xdeadf00f};
enum {MEM_Tag    =0xfe      };
enum {MEM_WipeTag=0xcd      };

// Debug memory allocator.
class FMallocDebug : public FMalloc
{
private:
	// Structure for memory debugging.
	struct FMemDebugBase
	{
		TCHAR*		Tag;
		SIZE_T		Size;
		INT			RefCount;
		INT			PreTag;
	};
	typedef TDoubleLinkedList<FMemDebugBase> FMemDebug;

	// Variables.
	FMemDebug* GFirstDebug;
	UBOOL MemInited;

public:
	// FMalloc interface.
	FMallocDebug()
	:	GFirstDebug	( NULL )
	,	MemInited	( 0 )
	{}
	void* Malloc( DWORD Size, const TCHAR* Tag )
	{
		guard(FMallocDebug::Malloc);
		checkSlow(MemInited);
		check((INT)Size>0);
		FMemDebug* Ptr = NULL;
		guard(CallMalloc);
			Ptr = (FMemDebug*)malloc( sizeof(FMemDebug) + Size + sizeof(INT) );
			check(Ptr);
		unguard;
		TCHAR* DupTag = NULL;
		guard(DupTag);
			DupTag = (TCHAR*)malloc((appStrlen(Tag)+1)*sizeof(TCHAR));
			appStrcpy( DupTag, Tag );
		unguard;
		guard(SetPtr);
			Ptr->Tag      = DupTag;
			Ptr->RefCount = 1;
			Ptr->Size     = Size;
			Ptr->Next     = GFirstDebug;
			Ptr->PrevLink = &GFirstDebug;
			Ptr->PreTag   = MEM_PreTag;
		unguard;
		guard(SetPost);
			*(INT*)((BYTE*)Ptr+sizeof(FMemDebug)+Size) = MEM_PostTag;
		unguard;
		guard(FillMem);
			appMemset( Ptr+1, MEM_Tag, Size );
		unguard;
		guard(DoPrevLink);
			if( GFirstDebug )
			{
				check(GIsCriticalError||GFirstDebug->PrevLink==&GFirstDebug);
				GFirstDebug->PrevLink = &Ptr->Next;
			}
			GFirstDebug = Ptr;
		unguard;
		return Ptr+1;
		unguardf(( TEXT("%i %s"), Size, Tag ));
	}
	void* Realloc( void* InPtr, DWORD NewSize, const TCHAR* Tag )
	{
		guard(FMallocDebug::Realloc);
		checkSlow(MemInited);
		if( InPtr && NewSize )
		{
			check(GIsCriticalError||((FMemDebug*)InPtr-1)->RefCount==1);
			check(GIsCriticalError||((FMemDebug*)InPtr-1)->Size>0);
			void* Result = appMalloc( NewSize, Tag );
			appMemcpy( Result, InPtr, Min(((FMemDebug*)InPtr-1)->Size,NewSize) );
			appFree( InPtr );
			return Result;
		}
		else if( NewSize )
		{
			return appMalloc( NewSize, Tag );
		}
		else
		{
			if( InPtr )
				appFree( InPtr );
			return NULL;
		}
		unguardf(( TEXT("%08X %i %s"), (INT)InPtr, NewSize, Tag ));
	}
	void Free( void* InPtr )
	{
		guard(FMallocDebug::Free);
		checkSlow(MemInited);
		if( !InPtr )
			return;

		FMemDebug* Ptr = (FMemDebug*)InPtr - 1;
		check(GIsCriticalError||Ptr->Size>0);
		check(GIsCriticalError||Ptr->RefCount==1);
		check(GIsCriticalError||Ptr->PreTag==MEM_PreTag);
		check(GIsCriticalError||*(INT*)((BYTE*)InPtr+Ptr->Size)==MEM_PostTag);
		appMemset( InPtr, MEM_WipeTag, Ptr->Size );
		Ptr->Size = 0;
		Ptr->RefCount = 0;

		check(GIsCriticalError||Ptr->PrevLink);
		check(GIsCriticalError||*Ptr->PrevLink==Ptr);
		*Ptr->PrevLink = Ptr->Next;
		if( Ptr->Next )
			Ptr->Next->PrevLink = Ptr->PrevLink;

		free( Ptr->Tag );
		free( Ptr );

		unguard;
	}
	void DumpAllocs()
	{
		guard(FMallocDebug::DumpAllocs);
		INT Count=0;
		INT Chunks=0;
		debugf( TEXT("Unfreed memory:") );
		for( FMemDebug* Ptr=GFirstDebug; Ptr; Ptr=Ptr->Next )
		{
			TCHAR Temp[256];
			appStrncpy( Temp, (TCHAR*)(Ptr+1), Min((SIZE_T)255,Ptr->Size) );
			//debugf( TEXT("   % 10i %s <%s>"), Ptr->Size, Ptr->Tag, Temp );
			Count += Ptr->Size;
			Chunks++;
		}
		debugf( TEXT("End of list: %i Bytes still allocated"), Count );
		debugf( TEXT("             %i Chunks allocated"), Chunks );
		unguard;
	}
	void HeapCheck()
	{
		guard(FMallocDebug::HeapCheck);
		for( FMemDebug** Link = &GFirstDebug; *Link; Link=&(*Link)->Next )
			check(GIsCriticalError||*(*Link)->PrevLink==*Link);
		unguard;
	}
	void Init()
	{
		guard(FMallocDebug::Init);
		check(!MemInited);
		MemInited=1;
		unguard;
	}
	void Exit()
	{
		guard(FMallocDebug::Exit);
		check(MemInited);
		MemInited=0;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
