/*=============================================================================
	UnLinker.h: Unreal object linker.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FObjectExport.
-----------------------------------------------------------------------------*/

/**
 * @brief Information about an exported object.
 */
struct CORE_API FObjectExport{
	// Variables.
	INT         ClassIndex;		//!< Persistent.
	INT         SuperIndex;		//!< Persistent (for UStruct-derived objects only).
	INT			PackageIndex;	//!< Persistent.
	FName		ObjectName;		//!< Persistent.
	DWORD		ObjectFlags;	//!< Persistent.
	INT         SerialSize;		//!< Persistent.
	INT         SerialOffset;	//!< Persistent (for checking only).
	UObject*	_Object;		//!< Internal.
	INT			_iHashNext;		//!< Internal.

	// Functions.
	FObjectExport();
	FObjectExport(UObject* InObject);

	friend FArchive& operator<<(FArchive& Ar, FObjectExport& E){
		guard(FObjectExport<<);

		Ar << AR_INDEX(E.ClassIndex);
		Ar << AR_INDEX(E.SuperIndex);
		Ar << E.PackageIndex;
		Ar << E.ObjectName;
		Ar << E.ObjectFlags;
		Ar << AR_INDEX(E.SerialSize);

		if(E.SerialSize)
			Ar << AR_INDEX(E.SerialOffset);

		return Ar;

		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FObjectImport.
-----------------------------------------------------------------------------*/

/**
 * @brief Information about an imported object.
 */
struct CORE_API FObjectImport{
	// Variables.
	FName			ClassPackage;	//!< Persistent.
	FName			ClassName;		//!< Persistent.
	INT				PackageIndex;	//!< Persistent.
	FName			ObjectName;		//!< Persistent.
	UObject*		XObject;		//!< Internal (only really needed for saving, can easily be gotten rid of for loading).
	ULinkerLoad*	SourceLinker;	//!< Internal.
	INT             SourceIndex;	//!< Internal.

	// Functions.
	FObjectImport();
	FObjectImport(UObject* InObject);

	friend FArchive& operator<<(FArchive& Ar, FObjectImport& I){
		guard(FObjectImport<<);

		Ar << I.ClassPackage << I.ClassName;
		Ar << I.PackageIndex;
		Ar << I.ObjectName;

		if(Ar.IsLoading()){
			I.SourceIndex = INDEX_NONE;
			I.XObject     = NULL;
		}

		return Ar;

		unguard;
	}
};

/*----------------------------------------------------------------------------
	Items stored in Unrealfiles.
----------------------------------------------------------------------------*/

/**
 * @brief Unrealfile summary, stored at top of file.
 */
struct FGenerationInfo{
	INT ExportCount, NameCount;

	FGenerationInfo(INT InExportCount, INT InNameCount);

	friend FArchive& operator<<(FArchive& Ar, FGenerationInfo& Info){
		guard(FGenerationInfo<<);

		return Ar << Info.ExportCount << Info.NameCount;

		unguard;
	}
};

struct FPackageFileSummary{
	// Variables.
	INT		Tag;

protected:
	INT		FileVersion;

public:
	DWORD	PackageFlags;
	INT		NameCount,		NameOffset;
	INT		ExportCount,	ExportOffset;
	INT     ImportCount,	ImportOffset;
	FGuid	Guid;
	TArray<FGenerationInfo> Generations;

	// Constructor.
	FPackageFileSummary();
	INT GetFileVersion() const { return (FileVersion & 0xffff); }
	INT GetFileVersionLicensee() const { return ((FileVersion >> 16) & 0xffff); }
	void SetFileVersions(INT Epic, INT Licensee) { if(GSys->LicenseeMode == 0) FileVersion = Epic; else FileVersion = ((Licensee << 16) | Epic); }

	friend FArchive& operator<<(FArchive& Ar, FPackageFileSummary& Sum){
		guard(FUnrealfileSummary<<);

		Ar << Sum.Tag;

		if(!Ar.IsLoading() || Sum.Tag == PACKAGE_FILE_TAG){
			Ar << Sum.FileVersion;
			Ar << Sum.PackageFlags;
			Ar << Sum.NameCount     << Sum.NameOffset;
			Ar << Sum.ExportCount   << Sum.ExportOffset;
			Ar << Sum.ImportCount   << Sum.ImportOffset;

			if(Sum.GetFileVersion() >= 68){
				INT GenerationCount = Sum.Generations.Num();
				Ar << Sum.Guid << GenerationCount;
				//!!67 had: return
				if(Ar.IsLoading())
					Sum.Generations = TArray<FGenerationInfo>(GenerationCount);
				for(INT i=0; i<GenerationCount; i++)
					Ar << Sum.Generations[i];
			}else{ // oldver
				INT HeritageCount, HeritageOffset;
				Ar << HeritageCount << HeritageOffset;
				INT Saved = Ar.Tell();

				if(HeritageCount){
					Ar.Seek(HeritageOffset);
					for(INT i=0; i<HeritageCount; i++)
						Ar << Sum.Guid;
				}

				Ar.Seek(Saved);

				if(Ar.IsLoading()){
					Sum.Generations.Empty(1);
					new(Sum.Generations)FGenerationInfo(Sum.ExportCount,Sum.NameCount);
				}
			}
		}

		return Ar;

		unguard;
	}
};

/*----------------------------------------------------------------------------
	ULinker.
----------------------------------------------------------------------------*/

/**
 * A file linker.
 */
class CORE_API ULinker : public UObject{
	DECLARE_CLASS(ULinker,UObject,CLASS_Transient,Core)
	NO_DEFAULT_CONSTRUCTOR(ULinker)

	// Variables.
	UObject*				LinkerRoot;		//!< The linker's root object.
	FPackageFileSummary		Summary;		//!< File summary.
	TArray<FName>			NameMap;		//!< Maps file name indices to name table indices.
	TArray<FObjectImport>	ImportMap;		//!< Maps file object indices >=0 to external object names.
	TArray<FObjectExport>	ExportMap;		//!< Maps file object indices >=0 to external object names.
	INT						Success;		//!< Whether the object was constructed successfully.
	FString					Filename;		//!< Filename.
	DWORD					_ContextFlags;	//!< Load flag mask.

	// Constructors.
	ULinker(UObject* InRoot, const TCHAR* InFilename);
	void Serialize(FArchive& Ar);
	FString GetImportFullName(INT i);
	FString GetExportFullName(INT i, const TCHAR* FakeRoot=NULL);

	// Cheat Protection

	// The QuickMD5 hash is a check of 4 major tables for this package.  It looks at
	// the Header, Name, Import and Export tables.  Any changes here will result in a failure.

	FString QuickMD5(); //!< Returns the Quick MD5 hash for this package

	virtual UBOOL LinksToCode(); //!< True if this Linker contains code

protected:

	BYTE QuickMD5Digest[16];	// Holds a MD5 of the 3 Tables and Summary};
};

/*----------------------------------------------------------------------------
	ULinkerLoad.
----------------------------------------------------------------------------*/

/**
 * @brief A file loader.
 */
class ULinkerLoad : public ULinker, public FArchive{
	DECLARE_CLASS(ULinkerLoad,ULinker,CLASS_Transient,Core)
	NO_DEFAULT_CONSTRUCTOR(ULinkerLoad)

	// Friends.
	friend class UObject;
	friend class UPackageMap;

	// Variables.
	DWORD					LoadFlags;
	UBOOL					Verified;
	INT						ExportHash[256];
	TArray<FLazyLoader*>	LazyLoaders;
	FArchive*				Loader;

	ULinkerLoad(UObject* InParent, const TCHAR* InFilename, DWORD InLoadFlags);

	void Verify();
	FName GetExportClassPackage(INT i);
	FName GetExportClassName(INT i);
	void VerifyImport(INT i);
	void LoadAllObjects();
	INT FindExportIndex(FName ClassName, FName ClassPackage, FName ObjectName, INT PackageIndex);
	UObject* Create(UClass* ObjectClass, FName ObjectName, DWORD LoadFlags, UBOOL Checked);
	void Preload(UObject* Object);

	virtual UBOOL LinksToCode();	//!< True if this Linker contains code


private:
	UObject* CreateExport(INT Index);
	UObject* CreateImport(INT Index);

	UObject* IndexToObject(INT Index);
	void DetachExport(INT i);

	// UObject interface.
	void Serialize(FArchive& Ar);
	void Destroy();

	// FArchive interface.
	void AttachLazyLoader(FLazyLoader* LazyLoader);
	void DetachLazyLoader(FLazyLoader* LazyLoader);
	void DetachAllLazyLoaders(UBOOL Load);
	void Seek(INT InPos);
	INT Tell();
	INT TotalSize();
	void Serialize(void* V, INT Length);

	FArchive& operator<<(UObject*& Object){
		guard(ULinkerLoad<<UObject);

		INT Index;

		*Loader << AR_INDEX(Index);

		UObject* Temporary = IndexToObject(Index);

		appMemcpy(&Object, &Temporary, sizeof(UObject*));

		return *this;

		unguardf(("(%s %i))", GetFullName(), Tell()));
	}

	FArchive& operator<<(FName& Name){
		guard(ULinkerLoad<<FName);

		NAME_INDEX NameIndex;

		*Loader << AR_INDEX(NameIndex);

		if(!NameMap.IsValidIndex(NameIndex))
			appErrorf(TEXT("Bad name index %i/%i"), NameIndex, NameMap.Num());

		FName Temporary = NameMap[NameIndex];

		appMemcpy(&Name, &Temporary, sizeof(FName));

		return *this;

		unguardf((TEXT("(%s %i))"), GetFullName(), Tell()));
	}
};

/*----------------------------------------------------------------------------
	ULinkerSave.
----------------------------------------------------------------------------*/

/**
 * @brief A file saver.
 */
class ULinkerSave : public ULinker, public FArchive{
	DECLARE_CLASS(ULinkerSave,ULinker,CLASS_Transient,Core);
	NO_DEFAULT_CONSTRUCTOR(ULinkerSave);

	// Variables.
	FArchive* Saver;
	TArray<INT> ObjectIndices;
	TArray<INT> NameIndices;

	// Constructor.
	ULinkerSave(UObject* InParent, const TCHAR* InFilename);
	void Destroy();

	// FArchive interface.
	INT MapName(FName* Name);
	INT MapObject(UObject* Object);

	FArchive& operator<<(FName& Name){
		guardSlow(ULinkerSave<<FName);

		INT Save = NameIndices[Name.GetIndex()];

		return static_cast<FArchive&>(*this) << AR_INDEX(Save);

		unguardobjSlow;
	}

	FArchive& operator<<(UObject*& Obj){
		guardSlow(ULinkerSave<<UObject);

		INT Save = Obj ? ObjectIndices[Obj->GetIndex()] : 0;

		return static_cast<FArchive&>(*this) << AR_INDEX(Save);

		unguardobjSlow;
	}

	void Seek(INT InPos);
	INT Tell();
	void Serialize(void* V, INT Length);
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
