/*=============================================================================
	UnArc.h: Unreal archive class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Archive.
-----------------------------------------------------------------------------*/

//
// Archive class. Used for loading, saving, and garbage collecting
// in a byte order neutral way.
//
class CORE_API FArchive{
public:
	// FArchive interface.
	virtual ~FArchive(){}

	virtual void Serialize(void*, int) = 0;
	virtual void SerializeBits(void*, int);
	virtual void SerializeInt(unsigned long&, unsigned long);
	virtual void SerializeText(char const*);
	virtual void SerializeText(FString const&);
	virtual void Preload(UObject*){}
	virtual void CountBytes(char, const struct FMemCount&);
	virtual void CountBytes(void*, unsigned long);
	virtual FArchive& operator<<(FName&);
	virtual FArchive& operator<<(UObject*&);
	virtual int MapName(FName*);
	virtual int MapObject(UObject*);
	virtual int Tell();
	virtual int TotalSize();
	virtual int AtEnd();
	virtual void Seek(int);	
	virtual void AttachLazyLoader(FLazyLoader*);
	virtual void DetachLazyLoader(FLazyLoader*);
	virtual void Precache(int);
	virtual void Flush();
	virtual int Close();	
	virtual int GetError();
	virtual int GetMaxSerializeSize();
	virtual UObject* GetResourceObject();

	// Constructor.
	FArchive();

	// Status accessors.
	bool ContainsCode();
	bool ForClient();
	bool ForEdit();
	bool ForServer();
	bool IsCountingMem();
	bool IsCriticalError();
	bool IsError();
	bool IsGarbageCollecting();
	bool IsLinear();
	bool IsLoading();
	bool IsNet();
	bool IsPersistent();
	bool IsSaving();
	bool IsSkipping();
	bool IsTrans();
	int LicenseeVer();
	int NetVer();
	void Skip(bool);
	void ThisContainsCode();
	int Ver();

	// Friend archivers.
	friend FArchive& operator<<(FArchive& Ar, ANSICHAR& C){
		Ar.Serialize(&C, 1);

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, BYTE& B){
		Ar.Serialize(&B, 1);

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, SBYTE& B){
		Ar.Serialize(&B, 1);

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, _WORD& W){
		Ar.Serialize(&W, sizeof(W));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, SWORD& S){
		Ar.Serialize(&S, sizeof(S));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, DWORD& D){
		Ar.Serialize(&D, sizeof(D));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, INT& I){
		Ar.Serialize(&I, sizeof(I));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, FLOAT& F){
		Ar.Serialize(&F, sizeof(F));

		return Ar;
	}

	friend FArchive& operator<<(FArchive &Ar, QWORD& Q){
		Ar.Serialize(&Q, sizeof(Q));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, SQWORD& S){
		Ar.Serialize(&S, sizeof(S));

		return Ar;
	}

	friend FArchive& operator<<(FArchive& Ar, FTime& F);
protected:
	// Status variables.
	INT ArVer;
	INT ArNetVer;
	INT ArLicenseeVer;
	UBOOL ArIsLoading;
	UBOOL ArIsSaving;
	UBOOL ArIsTrans;
	UBOOL ArIsPersistent;
	UBOOL ArForEdit;
	UBOOL ArForClient;
	UBOOL ArForServer;
	UBOOL ArIsError;
};

/*-----------------------------------------------------------------------------
	FArchive macros.
-----------------------------------------------------------------------------*/

//
// Class for serializing objects in a compactly, mapping small values
// to fewer bytes.
//
class CORE_API FCompactIndex{
public:
	INT Value;

	void Serialize(FArchive& Ar);

	friend FArchive& operator<<(FArchive& Ar, FCompactIndex& I){
		I.Serialize(Ar);

		return Ar;
	}
};

//
// Archive constructor.
//
template
<typename T> T Arctor(FArchive& Ar){
	T Tmp;
	Ar << Tmp;

	return Tmp;
}

// Macro to serialize an integer as a compact index.
#define AR_INDEX(intref) \
	(*(FCompactIndex*)&(intref))

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/