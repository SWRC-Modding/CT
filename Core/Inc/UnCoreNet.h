/*=============================================================================
	UnCoreNet.h: Core networking support.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
// Information about a field.
//
class CORE_API FFieldNetCache{
public:
	UField* Field;
	INT FieldNetIndex;
	INT ConditionIndex;

	FFieldNetCache(){}
	FFieldNetCache(UField* InField, INT InFieldNetIndex, INT InConditionIndex) : Field(InField),
																				 FieldNetIndex(InFieldNetIndex),
																				 ConditionIndex(InConditionIndex){}
	friend CORE_API FArchive& operator<<(FArchive& Ar, FFieldNetCache& F);
};

//
// Information about a class, cached for network coordination.
//
class CORE_API FClassNetCache{
	friend class UPackageMap;
public:
	TArray<FFieldNetCache*> RepProperties;

	FClassNetCache();
	FClassNetCache(UClass* Class);

	INT GetMaxIndex(){ return FieldsBase+Fields.Num(); }
	INT FClassNetCache::GetRepConditionCount(){ return RepConditionCount; }
	FFieldNetCache* GetFromField(UObject* Field){
		guardSlow(FClassNetCache::GetFromField);

		FFieldNetCache* Result = NULL;

		for(FClassNetCache* C = this; C; C = C->Super){
			Result = C->FieldMap.FindRef(Field);

			if(Result)
				break;
		}

		return Result;

		unguardSlow;
	}
	FFieldNetCache* GetFromIndex(INT Index){
		guardSlow(FFieldNetCache::GetFromIndex);

		for(FClassNetCache* C = this; C; C = C->Super){
			if(Index >= C->FieldsBase && Index < C->FieldsBase + C->Fields.Num())
				return &C->Fields[Index-C->FieldsBase];
		}

		return NULL;

		unguardSlow;
	}

	CORE_API friend FArchive& operator<<(FArchive& Ar, FClassNetCache& Cache);

private:
	INT FieldsBase;
	FClassNetCache* Super;
	INT RepConditionCount;
	UClass* Class;
	TArray<FFieldNetCache> Fields;
	TMap<UObject*, FFieldNetCache*> FieldMap;
};

//
// Ordered information of linker file requirements.
//
class CORE_API FPackageInfo{
public:
	// Variables.
	FString			URL;				// URL of the package file we need to request.
	ULinkerLoad*	Linker;				// Pointer to the linker, if loaded.
	UObject*		Parent;				// The parent package.
	FGuid			Guid;				// Package identifier.
	INT				FileSize;			// File size.
	INT				ObjectBase;			// Net index of first object.
	INT				ObjectCount;		// Number of objects, defined by server.
	INT				NameBase;			// Net index of first name.
	INT				NameCount;			// Number of names, defined by server.
	INT				LocalGeneration;	// This machine's generation of the package.
	INT				RemoteGeneration;	// Remote machine's generation of the package.
	DWORD			PackageFlags;		// Package flags.

	// Functions.
	FPackageInfo(ULinkerLoad* InLinker = NULL);

	CORE_API friend FArchive& operator<<(FArchive& Ar, FPackageInfo& I);
};

//
// Maps objects and names to and from indices for network communication.
//
class CORE_API UPackageMap : public UObject{
	DECLARE_CLASS(UPackageMap,UObject,CLASS_Transient,Core);

	// UObject interface.
	void Serialize(FArchive& Ar);
	void Destroy();

	// UPackageMap interface.
	virtual UBOOL CanSerializeObject(UObject* Obj);
	virtual UBOOL SerializeObject(FArchive& Ar, UClass* Class, UObject*& Obj);
	virtual UBOOL SerializeName(FArchive& Ar, FName& Name);
	virtual INT ObjectToIndex(UObject* Object);
	virtual UObject* IndexToObject(INT Index, UBOOL Load);
	virtual INT AddLinker(ULinkerLoad* Linker);
	virtual void Compute();
	virtual INT GetMaxObjectIndex(){ return MaxObjectIndex; }
	virtual FClassNetCache* GetClassNetCache(UClass* Class);
	virtual UBOOL SupportsPackage(UObject* InOuter);

	void Copy(UPackageMap* Other);
	void CopyLinkers( UPackageMap* Other );

	// Variables.
	TArray<FPackageInfo> List;

protected:
	TMap<UObject*,INT> LinkerMap;
	TMap<UObject*,FClassNetCache*> ClassFieldIndices;
	TArray<INT> NameIndices;
	DWORD MaxObjectIndex;
	DWORD MaxNameIndex;
	char Padding[12]; // Padding
};

inline FArchive& operator<<(FArchive& Ar, FClassNetCache*){ return Ar; }

//
// Information for tracking retirement and retransmission of a property.
//
struct FPropertyRetirement
{
	INT			InPacketId;		// Packet received on, INDEX_NONE=none.
	INT			OutPacketId;	// Packet sent on, INDEX_NONE=none.
	BYTE		Reliable;		// Whether it was sent reliably.

	FPropertyRetirement() : OutPacketId(INDEX_NONE),
							InPacketId(INDEX_NONE){}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
