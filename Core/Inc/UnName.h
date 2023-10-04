/*=============================================================================
	UnName.h: Unreal global name types.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Definitions.
----------------------------------------------------------------------------*/

// Maximum size of name
enum{ NAME_SIZE = 64 };

/*
 * Enumeration for finding name
 */
enum EFindName{
	FNAME_Find,      // Find a name; return 0 if it doesn't exist.
	FNAME_Add,       // Find a name or add it if it doesn't exist.
	FNAME_Intrinsic, // Find a name or add it intrinsically if it doesn't exist.
};

/*----------------------------------------------------------------------------
	FNameEntry
----------------------------------------------------------------------------*/

/*
 * A global name, as stored in the global name table.
 */
struct FNameEntry{
	// Variables.
	INT         Index;    // Index of name in hash.
	DWORD       Flags;    // RF_TagImp, RF_TagExp, RF_Native.
	FNameEntry* HashNext; // Pointer to the next entry in this hash bin's linked list.

	// The name string.
	TCHAR       Name[NAME_SIZE]; // Name, variable-sized.

	// Functions.
	CORE_API friend FArchive& operator<<(FArchive& Ar, FNameEntry& E);
	CORE_API friend FNameEntry* AllocateNameEntry(const TCHAR* Name, DWORD Index, DWORD Flags, FNameEntry* HashNext);
};

/*----------------------------------------------------------------------------
	FName.
----------------------------------------------------------------------------*/

/*
 * Public name, available to the world.
 *
 * Names are stored as WORD indices into the name table and
 * every name in Unreal is stored once and only once in that table.
 * Names are case-insensitive.
 */
class FName{
public:
	// Constructors
	FName(){}
	FName(EName N) : Entry(N != NAME_None ? Names[N] : NULL){}
	FName(const TCHAR* Name, EFindName FindType = FNAME_Add);
	FName(const FString& Name, EFindName FindType = FNAME_Add);

	// Accessors
	const TCHAR* operator*() const{ return Entry ? Entry->Name : Names[NAME_None]->Name; }
	INT GetIndex() const{ return Entry ? Entry->Index : 0; }
	DWORD GetFlags() const{ return Entry ? Entry->Flags : 0; }
	void SetFlags(DWORD Set) const{ if(Entry)Entry->Flags |= Set; }
	void ClearFlags(DWORD Clear) const{ if(Entry)Entry->Flags &= ~Clear; }
	FNameEntry* GetEntry() const{ return Entry; }

	bool operator!() const{ return Entry == NULL; }
	bool operator+() const{ return Entry != NULL; }
	bool operator==(FName Other) const{ return Entry == Other.Entry; }
	bool operator!=(FName Other) const{ return Entry != Other.Entry; }
	bool operator<(FName Other) const{ return Entry < Other.Entry; }
	operator FString(){ return FString(**this); }

	// Name subsystem.
	static void StaticInit();
	static void StaticExit();
	static void DeleteEntry(INT i);
	static void DisplayHash(class FOutputDevice& Ar);
	static void Hardcode(FNameEntry* AutoName);

	// Name subsystem accessors
	static const TCHAR* SafeString(EName Index){ return Initialized ? Names[Index]->Name : "Uninitialized"; }
	static bool SafeSuppressed(EName Index){ return Initialized && ((Names[Index]->Flags & 0x00001000) != 0); }
	static INT GetMaxNames(){ return Names.Num(); }
	static FNameEntry* GetEntry(INT i){ return Names[i]; }
	static bool GetInitialized(){ return Initialized; }

private:
	FNameEntry* Entry; // Names in RC store a pointer to the name entry instead of an index into the Names array

	// Static subsystem variables
	CORE_API static TArray<FNameEntry*> Names;          // Table of all names.
	CORE_API static TArray<INT>         Available;      // Indices of available names.
	CORE_API static FNameEntry*         NameHash[4096]; // Hashed names.
	CORE_API static bool                Initialized;    // Subsystem initialized.
};

inline DWORD GetTypeHash(FName N){
	return N.GetIndex();
}

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
