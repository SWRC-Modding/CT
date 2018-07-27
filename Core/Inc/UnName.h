/*=============================================================================
	UnName.h: Unreal global name types.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Definitions.
----------------------------------------------------------------------------*/

//Maximum size of name
enum {NAME_SIZE	= 64};

//Name index
typedef INT NAME_INDEX;

//Enumeration for finding name
enum EFindName{
	FNAME_Find,			//Find a name; return 0 if it doesn't exist.
	FNAME_Add,			//Find a name or add it if it doesn't exist.
	FNAME_Intrinsic,	//Find a name or add it intrinsically if it doesn't exist.
};

/*----------------------------------------------------------------------------
	FNameEntry
----------------------------------------------------------------------------*/

//
//	A global name, as stored in the global name table.
//
struct FNameEntry{
	//Variables.
	NAME_INDEX	Index;				//Index of name in hash.
	DWORD		Flags;				//RF_TagImp, RF_TagExp, RF_Native.
	FNameEntry*	HashNext;			//Pointer to the next entry in this hash bin's linked list.

	//The name string.
	TCHAR		Name[NAME_SIZE];	//Name, variable-sized.

	//Functions.
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

//
// Public name, available to the world.  Names are stored as WORD indices
// into the name table and every name in Unreal is stored once
// and only once in that table.  Names are case-insensitive.
//
#define checkName checkSlow
class FName{
public:
	//Accessors
	const TCHAR* operator*() const{ return Entry->Name; }

	NAME_INDEX GetIndex() const{
		checkName(Index < Names.Num());
		checkName(Names[Index]);
		return Entry->Index;
	}

	DWORD GetFlags() const{
		checkName(Index < Names.Num());
		checkName(Names[Index]);
		return Entry->Flags;
	}

	void SetFlags(DWORD Set) const{
		checkName(Index < Names.Num());
		checkName(Names[Index]);
		Entry->Flags |= Set;
	}

	void ClearFlags(DWORD Clear) const{
		checkName(Index < Names.Num());
		checkName(Names[Index]);
		Entry->Flags &= ~Clear;
	}

	bool operator==(const FName& Other) const{ return Entry == Other.Entry; }
	bool operator!=(const FName& Other) const{ return Entry != Other.Entry; }

	//Constructors
	FName(){}
	FName(EName N) : Entry(Names[N]){}
	CORE_API FName(const TCHAR* Name, EFindName FindType);
	CORE_API FName(const FString& Name, EFindName FindType);

	/*
	*	Creating an FString with "None" produces an invalid name when
	*	using the constructor from Core.dll which in turn leads to crashes
	*	when it it used so this constructor exists to circumvent the issue
	*	by checking the input string first.
	*	This is only necessary because there's an ini entry called "None"
	*	that gets converted to an FName when reading the ini file
	*/
	FName(const TCHAR* Name){
		if(appStricmp(Name, "None") != 0)
			*this = FName(Name, FNAME_Add);
		else
			Entry = Names[NAME_None];
	}

	//Name subsystem.
	CORE_API static void StaticInit();
	CORE_API static void StaticExit();
	CORE_API static void DeleteEntry(int i);
	CORE_API static void DisplayHash(class FOutputDevice& Ar);
	CORE_API static void Hardcode(FNameEntry* AutoName);

	//Name subsystem accessors
	static const TCHAR* SafeString(EName Index){ return Initialized ? Names[Index]->Name : "Uninitialized"; }
	static bool SafeSuppressed(EName Index){ return Initialized && ((Names[Index]->Flags & 0x00001000) != 0); }
	static int GetMaxNames(){ return Names.Num(); }
	static FNameEntry* GetEntry(int i){ return Names[i]; }
	static UBOOL GetInitialized(){ return Initialized; }

private:
	//RC stores a pointer to the name entry instead of an index into the Names array
	FNameEntry* Entry;

	FNameEntry* GetEntry() const{ return Entry; }

	//Static subsystem variables
	CORE_API static TArray<FNameEntry*>	Names;			 //Table of all names.
	CORE_API static TArray<INT>         Available;       //Indices of available names.
	CORE_API static FNameEntry*			NameHash[4096];  //Hashed names.
	CORE_API static bool				Initialized;	 //Subsystem initialized.
};

inline DWORD GetTypeHash(const FName N){
	return N.GetIndex();
}

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/