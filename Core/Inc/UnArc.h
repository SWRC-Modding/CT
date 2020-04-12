/*=============================================================================
	UnArc.h: Unreal archive class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Archive.
-----------------------------------------------------------------------------*/

/*
 * Archive class.
 *
 * Used for loading, saving, and garbage collecting
 * in a byte order neutral way.
 */
class /*CORE_API*/ FArchive{
	// Friend archivers.
	friend FArchive& operator<<(FArchive& Ar, ANSICHAR& C);
	friend FArchive& operator<<(FArchive& Ar, BYTE& B);
	friend FArchive& operator<<(FArchive& Ar, SBYTE& B);
	friend FArchive& operator<<(FArchive& Ar, _WORD& W);
	friend FArchive& operator<<(FArchive& Ar, SWORD& S);
	friend FArchive& operator<<(FArchive& Ar, DWORD& D);
	friend FArchive& operator<<(FArchive& Ar, INT& I);
	friend FArchive& operator<<(FArchive& Ar, FLOAT& F);
	friend FArchive& operator<<(FArchive &Ar, QWORD& Q);
	friend FArchive& operator<<(FArchive& Ar, SQWORD& S);
	friend FArchive& operator<<(FArchive& Ar, FTime& F);
public:
	// FArchive interface.
	virtual ~FArchive(){}
	virtual void Serialize(void* V, INT Length){}
	virtual void SerializeBits(void* V, INT LengthBits){
		Serialize(V, (LengthBits + 7) / 8);

		if(IsLoading())
			((BYTE*)V)[LengthBits / 8] &= ((1 << (LengthBits & 7)) - 1);
	}
	virtual void SerializeInt(DWORD& Value, DWORD Max){
		*this << *(DWORD*)Value;
	}
	virtual void SerializeText(char const*);
	virtual void SerializeText(FString const&);
	virtual void Preload(UObject*){}
	virtual void CountBytes(char, const struct FMemCount&);
	virtual void CountBytes(void*, unsigned long);
	virtual FArchive& operator<<(FName& N){ return *this; }
	virtual FArchive& operator<<(UObject*& Res){ return *this; }
	virtual INT MapName(FName*){ return 0; }
	virtual INT MapObject(UObject*){ return 0; }
	virtual INT Tell(){ return INDEX_NONE; }
	virtual INT TotalSize(){ return INDEX_NONE; }
	virtual UBOOL AtEnd(){
		INT Pos = Tell();

		return Pos != INDEX_NONE && Pos >= TotalSize();
	}
	virtual void Seek(INT InPos){}
	virtual void AttachLazyLoader(FLazyLoader* LazyLoader){}
	virtual void DetachLazyLoader(FLazyLoader* LazyLoader){}
	virtual void Precache(INT HintCount){}
	virtual void Flush(){}
	virtual UBOOL Close(){ return !ArIsError; }
	virtual UBOOL GetError(){ return ArIsError; }
	virtual INT GetMaxSerializeSize(){ return 0; }
	virtual UObject* GetResourceObject(){ return NULL; }

	// Constructor.
	FArchive();

	// Status accessors.
	bool ContainsCode(){ return ArContainsCode; }
	bool ForClient(){ return ArForClient; }
	bool ForEdit(){ return ArForEdit; }
	bool ForServer(){ return ArForServer; }
	bool IsCountingMem(){ return ArIsCountingMem; }
	bool IsCriticalError(){ return ArIsCriticalError; }
	bool IsError(){ return ArIsError; }
	bool IsGarbageCollecting(){ return ArIsGarbageCollecting; }
	bool IsLinear(){ return ArIsLinear; }
	bool IsLoading(){ return ArIsLoading; }
	bool IsNet(){ return (ArNetVer & 0x80000000) != 0 ; }
	bool IsPersistent(){ return ArIsPersistent; }
	bool IsSaving(){ return ArIsSaving; }
	bool IsSkipping(){ return ArIsSkipping; }
	bool IsTrans(){ return ArIsTrans; }
	int LicenseeVer(){ return ArLicenseeVer; }
	int NetVer(){ return ArNetVer & 0x7fffffff; }
	void Skip(bool bSkip){ ArIsSkipping = bSkip; }
	void ThisContainsCode(){ ArContainsCode = true; }
	int Ver(){ return ArVer; }

protected:
	// Status variables.
	INT ArVer;
	INT ArNetVer;
	INT ArLicenseeVer;
	bool ArIsLoading;
	bool ArIsSaving;
	bool ArIsGarbageCollecting;
	bool ArIsCountingMem;
	bool ArIsTrans;
	bool ArIsPersistent;
	bool ArIsLinear;
	bool ArIsSkipping;
	bool ArForEdit;
	bool ArForClient;
	bool ArForServer;
	bool ArIsError;
	bool ArIsCriticalError;
	bool ArContainsCode;
};

inline FArchive& operator<<(FArchive& Ar, ANSICHAR& C){
	Ar.Serialize(&C, 1);

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, BYTE& B){
	Ar.Serialize(&B, 1);

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, SBYTE& B){
	Ar.Serialize(&B, 1);

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, _WORD& W){
	Ar.Serialize(&W, sizeof(W));

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, SWORD& S){
	Ar.Serialize(&S, sizeof(S));

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, DWORD& D){
	Ar.Serialize(&D, sizeof(D));

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, INT& I){
	Ar.Serialize(&I, sizeof(I));

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, FLOAT& F){
	Ar.Serialize(&F, sizeof(F));

	return Ar;
}

inline FArchive& operator<<(FArchive &Ar, QWORD& Q){
	Ar.Serialize(&Q, sizeof(Q));

	return Ar;
}

inline FArchive& operator<<(FArchive& Ar, SQWORD& S){
	Ar.Serialize(&S, sizeof(S));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FTime& F);

/*-----------------------------------------------------------------------------
	FArchive macros.
-----------------------------------------------------------------------------*/

/*
 * Class for serializing objects compactly, mapping small values
 * to fewer bytes.
 */
class CORE_API FCompactIndex{
public:
	INT Value;

	void Serialize(FArchive& Ar);

	friend FArchive& operator<<(FArchive& Ar, FCompactIndex& I){
		I.Serialize(Ar);

		return Ar;
	}
};

// Archive constructor.
template<typename T>
T Arctor(FArchive& Ar){
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
