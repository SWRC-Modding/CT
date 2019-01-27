/*=============================================================================
	UnName.h: Unreal global name types.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Definitions.
----------------------------------------------------------------------------*/

//! Maximum size of name
enum{ NAME_SIZE = 64 };

//! Name index
typedef INT NAME_INDEX;

/**
 * @brief Enumeration for finding name
 */
enum EFindName{
	FNAME_Find,			//!< Find a name; return 0 if it doesn't exist.
	FNAME_Add,			//!< Find a name or add it if it doesn't exist.
	FNAME_Intrinsic,	//!< Find a name or add it intrinsically if it doesn't exist.
};

/*----------------------------------------------------------------------------
	FNameEntry
----------------------------------------------------------------------------*/

/**
 * @brief A global name, as stored in the global name table.
 */
struct FNameEntry{
	// Variables.
	NAME_INDEX	Index;				//!< Index of name in hash.
	DWORD		Flags;				//!< RF_TagImp, RF_TagExp, RF_Native.
	FNameEntry*	HashNext;			//!< Pointer to the next entry in this hash bin's linked list.

	// The name string.
	TCHAR		Name[NAME_SIZE];	//!< Name, variable-sized.

	// Functions.
	CORE_API friend FArchive& operator<<(FArchive& Ar, FNameEntry& E);
	CORE_API friend FNameEntry* AllocateNameEntry(const TCHAR* Name, DWORD Index, DWORD Flags, FNameEntry* HashNext);
};

template<>
struct TTypeInfo<FNameEntry*> : public TTypeInfoBase<FNameEntry*>{
	static UBOOL NeedsDestructor(){ return 0; }
};

/*----------------------------------------------------------------------------
	FName.
----------------------------------------------------------------------------*/

#define checkName checkSlow

/**
 * @brief Public name, available to the world.
 *
 * Names are stored as WORD indices into the name table and
 * every name in Unreal is stored once and only once in that table.
 * Names are case-insensitive.
 */
class FName{
public:
	// Accessors
	const TCHAR* operator*() const{ return Entry->Name; }
	NAME_INDEX GetIndex() const{ return Entry->Index; }
	DWORD GetFlags() const{ return Entry->Flags; }
	void SetFlags(DWORD Set) const{ Entry->Flags |= Set; }
	void ClearFlags(DWORD Clear) const{ Entry->Flags &= ~Clear; }

	bool operator==(const FName& Other) const{ return Entry == Other.Entry; }
	bool operator!=(const FName& Other) const{ return Entry != Other.Entry; }

	// Constructors
	FName() : Entry(Names[NAME_None]){}
	FName(EName N) : Entry(Names[N]){}
	CORE_API FName(const TCHAR* Name, EFindName FindType);
	CORE_API FName(const FString& Name, EFindName FindType);

	FName(const TCHAR* Name){
		*this = FName(Name, FNAME_Add);

		if(!Entry)
			Entry = Names[NAME_None];
	}

	FName(const FString& Name){ *this = FName(*Name); }

	// Name subsystem.
	CORE_API static void StaticInit();
	CORE_API static void StaticExit();
	CORE_API static void DeleteEntry(INT i);
	CORE_API static void DisplayHash(class FOutputDevice& Ar);
	CORE_API static void Hardcode(FNameEntry* AutoName);

	// Name subsystem accessors
	static const TCHAR* SafeString(EName Index){ return Initialized ? Names[Index]->Name : "Uninitialized"; }
	static bool SafeSuppressed(EName Index){ return Initialized && ((Names[Index]->Flags & 0x00001000) != 0); }
	static INT GetMaxNames(){ return Names.Num(); }
	static FNameEntry* GetEntry(INT i){ return Names[i]; }
	static bool GetInitialized(){ return Initialized; }

private:
	FNameEntry* Entry; // Names in RC store a pointer to the name entry instead of an index into the Names array

	FNameEntry* GetEntry() const{ return Entry; }

	// Static subsystem variables
	CORE_API static TArray<FNameEntry*>	Names;			 // Table of all names.
	CORE_API static TArray<INT>         Available;       // Indices of available names.
	CORE_API static FNameEntry*			NameHash[4096];  // Hashed names.
	CORE_API static bool				Initialized;	 // Subsystem initialized.
};

inline DWORD GetTypeHash(const FName N){
	return N.GetIndex();
}

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
