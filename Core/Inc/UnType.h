/*=============================================================================
	UnType.h: Unreal engine base type definitions.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UProperty.
-----------------------------------------------------------------------------*/

// Property exporting flags.
enum EPropertyPortFlags{
	PPF_Localized = 1,
	PPF_Delimited = 2,
	PPF_CheckReferences = 4,
};

/*
 * An UnrealScript variable.
 */
class CORE_API UProperty : public UField{
	DECLARE_ABSTRACT_CLASS(UProperty,UField,CLASS_IsAUProperty,Core)
	DECLARE_WITHIN(UField)

	// Persistent variables.
	DWORD PropertyFlags;
	FName Category;
	_WORD RepOffset;
	_WORD RepIndex;
	_WORD ArrayDim;
	_WORD Offset;
	char  Padding[4]; // Padding

	// Constructors.
	UProperty();
	UProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const = 0;
	virtual INT GetAlign() const;
	virtual void CleanupDestroyed(BYTE* Data) const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const = 0;
	virtual BYTE* GetAddress(const void* Data, INT ArrayIndex) const;
	virtual void ExportCpp(FOutputDevice& Out, UBOOL IsLocal, UBOOL IsParm, UBOOL IsEvent, UBOOL IsStruct) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent = 0) const = 0;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes = 0, INT IDONTKNOWWHATTHISIS = 0) const = 0; // TODO: Find out what the last parameter is
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const = 0;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const = 0;
	virtual UBOOL ExportText(INT ArrayElement, TCHAR* ValueStr, BYTE* Data, BYTE* Delta, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject = NULL) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject = NULL) const;
	virtual void DestroyValue(void* Dest) const;
	virtual UBOOL Port() const;
	virtual BYTE GetID() const;

	UField* GetOuterField() const{ return static_cast<UField*>(GetOuter()); }
	INT GetSize() const{ return ArrayDim * GetElementSize(); }
	INT DataOffset(INT ArrayIndex) const{ return Offset + ArrayIndex * GetElementSize(); }
	BYTE* DataAddr(void* Data, INT ArrayIndex) const{ return static_cast<BYTE*>(Data) + DataOffset(ArrayIndex); }
	bool Matches(const void* A, const void* B, INT ArrayIndex) const{
		INT Ofs = DataOffset(ArrayIndex);

		return Identical(static_cast<const BYTE*>(A) + Ofs, B ? (static_cast<const BYTE*>(B) + Ofs) : NULL) != 0;
	}
	bool ShouldSerializeValue(FArchive& Ar) const;/*{
		bool Skip = ((PropertyFlags & CPF_Native) != 0) || ((PropertyFlags & CPF_Transient) != 0 && Ar.IsPersistent());

		return !Skip;
	}*/
};

/*-----------------------------------------------------------------------------
	UByteProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes an unsigned byte value or 255-value enumeration variable.
 */
class CORE_API UByteProperty : public UProperty{
	DECLARE_CLASS(UByteProperty,UProperty,CLASS_IsAUProperty,Core)

	// Variables.
	UEnum* Enum;

	// Constructors.
	UByteProperty(){}
	UByteProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags, UEnum* InEnum = NULL) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags),
																											  Enum(InEnum){}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UIntProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a 32-bit signed integer variable.
 */
class CORE_API UIntProperty : public UProperty{
	DECLARE_CLASS(UIntProperty,UProperty,CLASS_IsAUProperty,Core)

	// Constructors.
	UIntProperty(){}
	UIntProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags){}

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UBoolProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a single bit flag variable residing in a 32-bit unsigned double word.
 */
class CORE_API UBoolProperty : public UProperty{
	DECLARE_CLASS(UBoolProperty,UProperty,CLASS_IsAUProperty,Core)

	// Variables.
	BYTE BitIndex;

	// Constructors.
	UBoolProperty(){}
	UBoolProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags),
																						BitIndex(0){}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UFloatProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes an IEEE 32-bit floating point variable.
 */
class CORE_API UFloatProperty : public UProperty{
	DECLARE_CLASS(UFloatProperty,UProperty,CLASS_IsAUProperty,Core)

	// Constructors.
	UFloatProperty(){}
	UFloatProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags){}

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UObjectProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a reference variable to another object which may be nil.
 */
class CORE_API UObjectProperty : public UProperty{
	DECLARE_CLASS(UObjectProperty,UProperty,CLASS_IsAUProperty,Core)

	// Variables.
	class UClass* PropertyClass;

	// Constructors.
	UObjectProperty(){}
	UObjectProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags, UClass* InClass);
	//:	UProperty(EC_CppProperty, InOffset, InCategory, InFlags)
	//,	PropertyClass(InClass)
	//{}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void CleanupDestroyed(BYTE* Data) const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UObjectProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a reference variable to another object which may be nil.
 */
class CORE_API UClassProperty : public UObjectProperty{
	DECLARE_CLASS(UClassProperty,UObjectProperty,0,Core)

	// Variables.
	class UClass* MetaClass;

	// Constructors.
	UClassProperty(){}
	UClassProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags, UClass* InMetaClass) : UObjectProperty(EC_CppProperty, InOffset, InCategory, InFlags, UClass::StaticClass()),
																											  MetaClass(InMetaClass){}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual BYTE GetID() const;
};

/*-----------------------------------------------------------------------------
	UNameProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a name variable pointing into the global name table.
 */
class CORE_API UNameProperty : public UProperty{
	DECLARE_CLASS(UNameProperty,UProperty,CLASS_IsAUProperty,Core)

	// Constructors.
	UNameProperty(){}
	UNameProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags){}

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	UStrProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a dynamic string variable.
 */
class CORE_API UStrProperty : public UProperty{
	DECLARE_CLASS(UStrProperty,UProperty,CLASS_IsAUProperty,Core)

	// Constructors.
	UStrProperty(){}
	UStrProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags){}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void DestroyValue(void* Dest) const;
};

/*-----------------------------------------------------------------------------
	UArrayProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a dynamic array.
 */
class CORE_API UArrayProperty : public UProperty{
	DECLARE_CLASS(UArrayProperty,UProperty,CLASS_IsAUProperty,Core)

	// Variables.
	UProperty* Inner;

	// Constructors.
	UArrayProperty()
	{}
	UArrayProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags)
	:	UProperty(EC_CppProperty, InOffset, InCategory, InFlags)
	{}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void CleanupDestroyed(BYTE* Data) const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual BYTE* GetAddress(const void* Data, INT ArrayIndex) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void DestroyValue(void* Dest) const;

	// UArrayProperty interface.
	void AddCppProperty(UProperty* Property);
};

/*-----------------------------------------------------------------------------
	UStructProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a structure variable embedded in (as opposed to referenced by)
 * an object.
 */
class CORE_API UStructProperty : public UProperty{
	DECLARE_CLASS(UStructProperty,UProperty,CLASS_IsAUProperty,Core)

	// Variables.
	class UStruct* Struct;

	// Constructors.
	UStructProperty(){}
	UStructProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags, UStruct* InStruct);
	//:	UProperty(EC_CppProperty, InOffset, InCategory, InFlags)
	//,	Struct(InStruct)
	//{}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual INT GetAlign() const;
	virtual void CleanupDestroyed(BYTE* Data) const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void DestroyValue(void* Dest) const;
};

/*-----------------------------------------------------------------------------
	UDelegateProperty.
-----------------------------------------------------------------------------*/

/*
 * Describes a pointer to a function bound to an Object
 */
class CORE_API UDelegateProperty : public UProperty{
	DECLARE_CLASS(UDelegateProperty,UProperty,CLASS_IsAUProperty,Core)

	UFunction* Function;

	// Constructors.
	UDelegateProperty(){}
	UDelegateProperty(ECppProperty, INT InOffset, const TCHAR* InCategory, DWORD InFlags) : UProperty(EC_CppProperty, InOffset, InCategory, InFlags){}

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UProperty interface.
	virtual INT GetElementSize() const;
	virtual void CleanupDestroyed(BYTE* Data) const;
	virtual void Link(FArchive& Ar, UProperty* Prev);
	virtual UBOOL Identical(const void* A, const void* B) const;
	virtual void ExportCppItem(FOutputDevice& Out, UBOOL IsEvent) const;
	virtual void SerializeItem(FArchive& Ar, void* Value, INT MaxReadBytes, INT IDONTKNOWWHATTHISIS) const;
	virtual UBOOL NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data) const;
	virtual void ExportTextItem(TCHAR* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, INT PortFlags) const;
	virtual const TCHAR* ImportText(const TCHAR* Buffer, BYTE* Data, INT PortFlags) const;
	virtual void CopySingleValue(void* Dest, void* Src, UObject* SuperObject) const;
	virtual void CopyCompleteValue(void* Dest, void* Src, UObject* SuperObject) const;
};

/*-----------------------------------------------------------------------------
	Field templates.
-----------------------------------------------------------------------------*/

/*
 * Find a typed field in a struct.
 */
template<typename T>
T* FindField(UStruct* Owner, const TCHAR* FieldName){
	guard(FindField);

	for(TFieldIterator<T> It(Owner); It; ++It){
		if(appStricmp(It->GetName(), FieldName) == 0)
			return *It;
	}

	return NULL;

	unguard;
}

/*-----------------------------------------------------------------------------
	UObject accessors that depend on UClass.
-----------------------------------------------------------------------------*/

/*
 * See if this object belongs to the specified class.
 */
inline UBOOL UObject::IsA(UClass* SomeBase) const{
	for(UClass* TempClass = Class; TempClass; TempClass = static_cast<UClass*>(TempClass->SuperStruct)){
		if(TempClass == SomeBase)
			return 1;
	}

	return SomeBase == NULL;
}

/*
 * See if this object is in a certain package.
 */
inline UBOOL UObject::IsIn(UObject* SomeOuter) const{
	for(UObject* It = GetOuter(); It; It = It->GetOuter()){
		if(It == SomeOuter)
			return 1;
	}

	return SomeOuter == NULL;
}

/*-----------------------------------------------------------------------------
	C++ property macros.
-----------------------------------------------------------------------------*/

#define CPP_PROPERTY(name) \
	EC_CppProperty, (BYTE*)&((ThisClass*)NULL)->name - (BYTE*)NULL

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
