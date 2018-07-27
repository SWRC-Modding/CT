/*=============================================================================
	FMallocAnsi.h: ANSI memory allocator.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
// ANSI C memory allocator.
//
class FMallocAnsi : public FMalloc
{
public:
	// FMalloc interface.
	void* Malloc( DWORD Size, const TCHAR* Tag )
	{
		guard(FMallocAnsi::Malloc);
		check(Size>0);
		void* Ptr = malloc( Size );
		check(Ptr);
		return Ptr;
		unguard;
	}
	void* Realloc( void* Ptr, DWORD NewSize, const TCHAR* Tag )
	{
		guard(FMallocAnsi::Realloc);
		check(NewSize>=0);
		void* Result;
		if( Ptr && NewSize )
		{
			Result = realloc( Ptr, NewSize );
		}
		else if( NewSize )
		{
			Result = malloc( NewSize );
		}
		else
		{
			if( Ptr )
				free( Ptr );
			Result = NULL;
		}
		return Result;
		unguardf(( TEXT("%08X %i %s"), (INT)Ptr, NewSize, Tag ));
	}
	void Free( void* Ptr )
	{
		guard(FMallocAnsi::Free);
		free( Ptr );
		unguard;
	}
	void DumpAllocs()
	{
		guard(FMallocAnsi::DumpAllocs);
		debugf( NAME_Exit, TEXT("Allocation checking disabled") );
		unguard;
	}
	void HeapCheck()
	{
		guard(FMallocAnsi::HeapCheck);
#if _MSC_VER
		INT Result = _heapchk();
		check(Result!=_HEAPBADBEGIN);
		check(Result!=_HEAPBADNODE);
		check(Result!=_HEAPBADPTR);
		check(Result!=_HEAPEMPTY);
		check(Result==_HEAPOK);
#endif
		unguard;
	}
	void Init()
	{
		guard(FMallocAnsi::Init);
		unguard;
	}
	void Exit()
	{
		guard(FMallocAnsi::Exit);
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
