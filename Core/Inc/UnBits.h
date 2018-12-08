/*=============================================================================
	UnBits.h: Unreal bitstream manipulation classes.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FBitWriter.
-----------------------------------------------------------------------------*/

//
// Writes bitstreams.
//
struct CORE_API FBitWriter : public FArchive{
	friend struct FBitWriterMark;
public:
	FBitWriter(INT InMaxBits);

	// FArchive interface.
	virtual void Serialize(void* Src, INT LengthBytes);
	virtual void SerializeBits(void* Src, INT LengthBits);
	virtual void SerializeInt(DWORD& Value, DWORD Max);

	void WriteInt(DWORD Result, DWORD Max);
	void WriteBit(BYTE In);
	BYTE* GetData();
	INT GetNumBytes();
	INT GetNumBits();
	void SetOverflowed();

private:
	TArray<BYTE> Buffer;
	INT   Num;
	INT   Max;
};

//
// For pushing and popping FBitWriter positions.
//
struct CORE_API FBitWriterMark{
public:
	FBitWriterMark() : Num(0){}
	FBitWriterMark(FBitWriter& Writer) : Overflowed(Writer.ArIsError),
										 Num(Writer.Num){}

	INT GetNumBits(){ return Num; }
	void Pop(FBitWriter& Writer);

private:
	UBOOL			Overflowed;
	INT				Num;
};

/*-----------------------------------------------------------------------------
	FBitReader.
-----------------------------------------------------------------------------*/

//
// Reads bitstreams.
//
struct CORE_API FBitReader : public FArchive{
public:
	FBitReader(BYTE* Src = NULL, INT CountBits = 0);

	// FArchive interface.
	virtual void Serialize(void* Dest, INT LengthBytes);
	virtual void SerializeBits(void* Dest, INT LengthBits);
	virtual void SerializeInt(DWORD& Value, DWORD Max);

	void SetData(FBitReader& Src, INT CountBits);
	DWORD ReadInt(DWORD Max);
	BYTE ReadBit();
	BYTE* GetData();
	UBOOL AtEnd();
	void SetOverflowed();
	INT GetNumBytes();
	INT GetNumBits();
	INT GetPosBits();

private:
	TArray<BYTE> Buffer;
	INT   Num;
	INT   Pos;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
