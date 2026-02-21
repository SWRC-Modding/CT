/*=============================================================================
	UnClass.h: UClass definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney.
=============================================================================*/

/*-----------------------------------------------------------------------------
	FRepRecord
-----------------------------------------------------------------------------*/

/*
 * Information about a property to replicate.
 */
struct FRepRecord{
	UProperty* Property;
	INT Index;

	FRepRecord(UProperty* InProperty,INT InIndex) : Property(InProperty),
													Index(InIndex){}
};

/*-----------------------------------------------------------------------------
	FDependency
-----------------------------------------------------------------------------*/

/*
 * One dependency record, for incremental compilation.
 */
class CORE_API FDependency{
public:
	// Variables.
	UClass* Class;
	UBOOL   Deep;
	DWORD   ScriptTextCRC;

	// Functions.
	FDependency();
	FDependency(UClass* InClass, UBOOL InDeep);
	UBOOL IsUpToDate();
	CORE_API friend FArchive& operator<<(FArchive& Ar, FDependency& Dep);
};

/*-----------------------------------------------------------------------------
	UField
-----------------------------------------------------------------------------*/

/*
 * Base class of UnrealScript language objects.
 */
class CORE_API UField : public UObject{
	DECLARE_ABSTRACT_CLASS(UField,UObject,0,Core)
	NO_DEFAULT_CONSTRUCTOR(UField)
public:
	// Constants.
	enum{ HASH_COUNT = 128 };

	// Variables.
	UField* Next;

	// Constructors.
	UField(ENativeConstructor, UClass* InClass, const TCHAR* InName, const TCHAR* InPackageName, DWORD InFlags);

	// UObject interface.
	void Serialize(FArchive& Ar);
	void PostLoad();
	void Register();

	// UField interface.
	virtual void AddCppProperty(UProperty* Property);
	virtual UBOOL MergeBools();
	virtual void Bind();
	virtual UClass* GetOwnerClass();
	virtual INT GetPropertiesSize();
};

/*-----------------------------------------------------------------------------
	TFieldIterator
-----------------------------------------------------------------------------*/

/*
 * For iterating through a linked list of fields.
 */
template<typename T>
class TFieldIterator{
public:
	TFieldIterator(UStruct* InStruct) : Struct(InStruct),
										Field(InStruct ? InStruct->Children : NULL)
										{
		IterateToNext();
	}

	void operator++()
	{
		Field = Field->Next;

		IterateToNext();
	}

	T* operator*(){ return static_cast<T*>(Field); }
	T* operator->(){ return static_cast<T*>(Field); }
	operator bool(){ return Field != NULL; }
	UStruct* GetStruct(){ return Struct; }

protected:
	UStruct* Struct;
	UField* Field;

	void IterateToNext()
	{
		while(Struct)
		{
			while(Field)
			{
				if(Field->IsA(T::StaticClass()))
					return;

				Field = Field->Next;
			}

			Struct = Struct->GetInheritanceSuper();

			if(Struct)
				Field = Struct->Children;
		}
	}
};

template<typename T, EClassFlags Flag>
class TFieldFlagIterator{
public:
	TFieldFlagIterator(UStruct* InStruct) : Struct(InStruct),
											Field(InStruct ? InStruct->Children : NULL)
											{
		IterateToNext();
	}

	inline void operator++()
	{
		Field = Field->Next;

		IterateToNext();
	}

	inline operator bool(){ return Field != NULL; }
	inline T* operator*(){ return (T*)Field; }
	inline T* operator->(){ return (T*)Field; }
	inline UStruct* GetStruct(){ return Struct; }

protected:
	UStruct* Struct;
	UField*  Field;

	inline void IterateToNext()
	{
		while(Struct)
		{
			while(Field)
			{
                if(Field->GetClass()->ClassFlags & Flag)
                    return;

				Field = Field->Next;
			}

			Struct = Struct->GetInheritanceSuper();

			if(Struct)
				Field = Struct->Children;
		}
	}
};

/*-----------------------------------------------------------------------------
	UStruct
-----------------------------------------------------------------------------*/

/*
 * State flags.
 */
enum EStructFlags{
	STRUCT_Native  = 0x00000001,
	STRUCT_Export  = 0x00000002,
	STRUCT_Long    = 0x00000004, // will get shown as "..." in editactor until expanded.
};

/*
 * An UnrealScript structure definition.
 */
class CORE_API UStruct : public UField{
	DECLARE_CLASS(UStruct,UField,0,Core)
	NO_DEFAULT_CONSTRUCTOR(UStruct)

	// Variables.
	UStruct*     HashNext;
	UStruct*     SuperStruct;
	UTextBuffer* ScriptText;
	UTextBuffer* CppText;
	UField*      Children;
	INT          PropertiesSize;
	INT          PropertiesSizeAligned; // 0 for native classes
	INT          MinAlignment;
	FName        FriendlyName;
	TArray<BYTE> Script;

	// Compiler info.
	INT          TextPos;
	INT          Line;
	DWORD        StructFlags;
	UProperty*   Properties;

	// Constructors.
	UStruct(ENativeConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, DWORD InFlags, UStruct* InSuperStruct);
	UStruct(UStruct* InSuperStruct);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void Destroy();
	virtual void Register();

	// UField interface.
	virtual void AddCppProperty(UProperty* Property);
	virtual INT GetPropertiesSize();

	// UStruct interface.
	virtual UStruct* GetInheritanceSuper();
	virtual void Link(FArchive& Ar, UBOOL Props);
	virtual void SerializeBin(FArchive& Ar, BYTE* Data, INT MaxReadBytes);
	virtual void SerializeTaggedProperties(FArchive& Ar, BYTE* Data, UClass* DefaultsClass, INT);
	virtual void CleanupDestroyed(BYTE* Data);
	virtual EExprToken SerializeExpr(INT& iCode, FArchive& Ar);
	virtual TCHAR* GetNameCPP() const;

	FString FunctionMD5();
	INT GetPropertiesAlign();
	void SetPropertiesSize(INT NewSize);
	bool IsChildOf(const UStruct* SomeBase) const;
	UStruct* GetSuperStruct() const;
	bool StructCompare(const void* A, const void* B);

	template<typename T>
	bool IsChildOf()
	{
		return IsChildOf(T::StaticClass());
	}

protected:
	// Cheat Protection
	BYTE FunctionMD5Digest[16]; // Holds a MD5 digest for this function
};

/*-----------------------------------------------------------------------------
	UFunction
-----------------------------------------------------------------------------*/

/*
 * An UnrealScript function.
 */
class CORE_API UFunction : public UStruct{
	DECLARE_CLASS(UFunction,UStruct,0,Core)
	DECLARE_WITHIN(UState)
	NO_DEFAULT_CONSTRUCTOR(UFunction)

	// Persistent variables.
	DWORD  FunctionFlags;
	_WORD  iNative;
	_WORD  RepOffset;
	BYTE   OperPrecedence;

	// Variables in memory only.
	BYTE   NumParms;
	_WORD  ParmsSize;
	_WORD  ReturnValueOffset;
	Native Func;

	// Constructors.
	UFunction(UFunction* InSuperFunction);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();

	// UField interface.
	virtual void Bind();

	// UStruct interface.
	virtual UBOOL MergeBools(){ return 0; }
	virtual UStruct* GetInheritanceSuper(){ return NULL; }
	virtual void Link(FArchive& Ar, UBOOL Props);

	// UFunction interface.
	UFunction* GetSuperFunction() const;
	UProperty* GetReturnProperty();
};

/*-----------------------------------------------------------------------------
	UState
-----------------------------------------------------------------------------*/

/*
 * An UnrealScript state.
 */
class CORE_API UState : public UStruct{
	DECLARE_CLASS(UState,UStruct,0,Core)
	NO_DEFAULT_CONSTRUCTOR(UState)

	// Variables.
	QWORD   ProbeMask;
	QWORD   IgnoreMask;
	DWORD   StateFlags;
	_WORD   LabelTableOffset;
	UField* VfHash;

	// Constructors.
	UState(ENativeConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, DWORD InFlags, UState* InSuperState);
	UState(UState* InSuperState);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void Destroy();

	// UStruct interface.
	virtual UBOOL MergeBools() {return 1;}
	virtual UStruct* GetInheritanceSuper() {return GetSuperState();}
	virtual void Link(FArchive& Ar, UBOOL Props);

	// UState interface.
	UState* GetSuperState() const;
};

/*-----------------------------------------------------------------------------
	UEnum
-----------------------------------------------------------------------------*/

/*
 * An enumeration, a list of names usable by UnrealScript.
 */
class CORE_API UEnum : public UField{
	DECLARE_CLASS(UEnum,UField,0,Core)
	DECLARE_WITHIN(UStruct)
	NO_DEFAULT_CONSTRUCTOR(UEnum)

	// Variables.
	TArray<FName> Names;

	// Constructors.
	UEnum(UEnum* InSuperEnum);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
};

/*-----------------------------------------------------------------------------
	UClass
-----------------------------------------------------------------------------*/

/*
 * An object class.
 */
class CORE_API UClass : public UState{
	DECLARE_CLASS(UClass,UState,0,Core)
	DECLARE_WITHIN(UPackage)

	typedef void(*Constructor)(void*);
	typedef void(UObject::*StaticConstructor)();

	// Variables.
	DWORD                  ClassFlags;
	INT                    ClassUnique;
	FGuid                  ClassGuid;
	DWORD                  ClassCRC;
	UClass*                ClassWithin;
	FName                  ClassConfigName;
	INT                    NativeSize; // 0 if not a native class
	TArray<FRepRecord>     ClassReps;
	TArray<UField*>        NetFields;
	TArray<FDependency>    Dependencies;
	TArray<FName>          PackageImports;
	TArray<BYTE>           Defaults;
	TArray<FName>          HideCategories;
	TArray<FName>          DependentOn;
	TArray<UClass*>        Subclasses; // Only set temporarily when exporting the header file for native classes
	FNativeEntry<UObject>* NativeFunctions;
	Constructor            ClassConstructor;
	StaticConstructor      ClassStaticConstructor;

	// In memory only.
	FString                DefaultPropText;
	UClass*                DefaultsClass; // If GIsEditor is set this will be the property override class if one exists (Base class name + "Defaults")

	// Constructors.
	UClass();
	UClass(UClass* InSuperClass);
	UClass(ENativeConstructor, DWORD InSize, DWORD InClassFlags, UClass* InBaseClass, UClass* InWithinClass, FGuid InGuid, const TCHAR* InNameStr, const TCHAR* InPackageName, const TCHAR* InClassConfigName, DWORD InFlags, Constructor InClassConstructor, StaticConstructor InClassStaticConstructor);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void Destroy();
	virtual void Register();
	virtual bool IsDefaultValue(const struct FPropertyInstance&);

	// UField interface.
	virtual void Bind();

	// UStruct interface.
	virtual UBOOL MergeBools();
	virtual UStruct* GetInheritanceSuper();
	virtual TCHAR* GetNameCPP() const;
	virtual void Link(FArchive& Ar, UBOOL Props);

	// UClass interface.
	void AddDependency(UClass* InClass, UBOOL InDeep);
	void AddSubclass(UClass*);
	void EditDefaultProps();
	Native FindNativeFunc(const TCHAR*, int);
	DWORD GetClassCRC();
	class AActor* GetDefaultActor() const;
	UObject* GetDefaultObject() const;
	BYTE* GetDefaults() const;
	UClass* GetSuperClass() const;
	bool HasNativesToExport();
	bool IsDefaultsOverride() const;
	bool IsUselessName(const TCHAR*) const;
	void PropagateNewValue(const struct FPropertyInstance&, unsigned char*);
	void RegisterNatives();

private:
	// Hide IsA because calling IsA on a class almost always indicates
	// an error where the caller should use IsChildOf.
	UBOOL IsA(UClass* Parent) const{ return UObject::IsA(Parent); }
};

/*-----------------------------------------------------------------------------
	UConst
-----------------------------------------------------------------------------*/

/*
 * An UnrealScript constant.
 */
class CORE_API UConst : public UField{
	DECLARE_CLASS(UConst,UField,0,Core)
	DECLARE_WITHIN(UStruct)
	NO_DEFAULT_CONSTRUCTOR(UConst)

	// Variables.
	FString Value;

	// Constructors.
	UConst(UConst* InSuperConst, const TCHAR* InValue);

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
