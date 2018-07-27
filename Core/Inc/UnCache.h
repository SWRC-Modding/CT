/*===============================================================================
	UnCache.h: Unreal fast memory cache support.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney.
===============================================================================*/

/*-----------------------------------------------------------------------------
	FMemCache.
-----------------------------------------------------------------------------*/

//
// Memory cache.
//
class CORE_API FMemCache
{
public:
	// Information about a cache item (32 bytes).
	enum {COST_INFINITE=0x1000000};
	class CORE_API FCacheItem
	{
	public:
		friend class FMemCache;
		void Unlock()
		{
#if DO_GUARD_SLOW
			if( this == NULL )
				appErrorf( TEXT("Unlock: Null cache item") );
			if( Cost < COST_INFINITE )
				appErrorf( TEXT("Unlock: Item %08X.%08X is not locked"), (DWORD)(Id>>32), (DWORD)Id );
#endif
			Cost -= COST_INFINITE;
		}
		QWORD GetId()
		{
			return Id;
		}
		BYTE* GetData()
		{
			return Data;
		}
		INT GetSize()
		{
			return LinearNext->Data - Data;
		}
		BYTE GetExtra()
		{
			return Extra;
		}
		void SetExtra( BYTE B )
		{
			Extra = B;
		}
		typedef _WORD TCacheTime;
		INT GetCost()
		{
			return Cost;
		}
		TCacheTime GetTime()
		{
			return Time;
		}

		// Implementation.
	private:
		QWORD		Id;				// This item's cache id, 0=unused.
		BYTE*		Data;			// Pointer to the item's data.
		TCacheTime	Time;			// Last Get() time.
		BYTE		Segment;		// Number of the segment this item resides in.
		BYTE		Extra;			// Extra space for use.
		INT			Cost;			// Cost to flush this item.
		FCacheItem*	LinearNext;		// Next cache item in linear list, or NULL if last.
		FCacheItem*	LinearPrev;		// Previous cache item in linear list, or NULL if first.
		FCacheItem*	HashNext;		// Next cache item in hash table, or NULL if last.
	};

	// FMemCache interface.
	FMemCache() {Initialized=0;}
    void Init( INT BytesToAllocate, INT MaxItems, void* Start=NULL, INT SegSize=0 );
	void Exit( INT FreeMemory );
	void Flush( QWORD Id=0, DWORD Mask=~0, UBOOL IgnoreLocked=0 );
	BYTE* Create( QWORD Id, FCacheItem *&Item, INT CreateSize, INT Alignment=DEFAULT_ALIGNMENT, INT SafetyPad=0 );
	void Tick();
	void CheckState();
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	void Status( TCHAR* Msg );
	INT GetTime() {return Time;}

	// Accessors.
	FCacheItem* First()
	{
		return CacheItems;
	}
	FCacheItem* Last()
	{
		return LastItem;
	}
	FCacheItem* Next( FCacheItem* Item )
	{
		return Item->LinearNext;
	}
	DWORD GHash( QWORD Val )
	{
		DWORD D=(DWORD)Val;
		return (D ^ (D>>12) ^ (D>>24)) & (HASH_COUNT-1);
	}
	BYTE* Get( QWORD Id, FCacheItem*& Item, INT Alignment=DEFAULT_ALIGNMENT )
	{	
		guardSlow(FMemCache::Get);
		clockSlow(GetCycles);
#if DO_SLOW_CLOCK
		NumGets++;
#endif
		if( Id==MruId )
		{
			Item = MruItem;
			MruItem->Cost += COST_INFINITE;
			return Align( MruItem->Data, Alignment );
		}
		for( FCacheItem* HashItem=HashItems[GHash(Id)]; HashItem; HashItem=HashItem->HashNext )
		{
			if( HashItem->Id == Id )
			{
				// Set the item, lock it, and return its data.
				MruId			= Id;
				MruItem			= HashItem;
				Item            = HashItem;
				HashItem->Time  = Time;
				HashItem->Cost += COST_INFINITE;
				unclockSlow(GetCycles);
				return Align( HashItem->Data, Alignment );
			}
		}
		unclockSlow(GetCycles);
		return NULL;
		unguardSlow;
	}

private:
	// Constants.
	enum {HASH_COUNT=16384};
	enum {IGNORE_SIZE=256};

	// Variables.
	UBOOL		Initialized;
	INT			Time;
	QWORD		MruId;
	FCacheItem* MruItem;
	FCacheItem* ItemMemory;
	FCacheItem* CacheItems;
	FCacheItem* LastItem;
	FCacheItem* UnusedItems;
	FCacheItem* HashItems[HASH_COUNT];
	BYTE*       CacheMemory;

	// Stats.
	INT			NumGets, NumCreates, CreateCycles, GetCycles, TickCycles;
	INT			ItemsFresh, ItemsStale, ItemsTotal, ItemGaps;
	INT			MemFresh, MemStale, MemTotal;

	// Internal functions.
	void CreateNewFreeSpace( BYTE* Start, BYTE* End, FCacheItem* Prev, FCacheItem* Next, INT Segment );
	void Unhash( QWORD Id )
	{
		for( FCacheItem** PrevLink=&HashItems[GHash(Id)]; *PrevLink; PrevLink=&(*PrevLink)->HashNext )
		{
			if( (*PrevLink)->Id == Id )
			{
				*PrevLink = (*PrevLink)->HashNext;
				return;
			}
		}
		appErrorf( TEXT("Unhashed item") );
	}
	FCacheItem* MergeWithNext( FCacheItem* First );
	FCacheItem* FlushItem( FCacheItem* Item, UBOOL IgnoreLocked=0 );
	void ConditionalCheckState()
	{
#if DO_GUARD_SLOW
		CheckState();
#endif
	}

	// Friends.
	friend class FCacheItem;
};

// Global scope.
typedef FMemCache::FCacheItem FCacheItem;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
