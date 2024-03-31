/*=============================================================================
	UFactory.h: Factory class definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	UFactory.
----------------------------------------------------------------------------*/

class ULevel;

//
// An object responsible for creating and importing new objects.
//
class CORE_API UFactory : public UObject{
	DECLARE_ABSTRACT_CLASS(UFactory,UObject,0,Core)

	// Per-class variables.
	UClass*         SupportedClass;
	UClass*         ContextClass;
	FString         Description;
	FString         InContextCommand;
	FString         OutOfContextCommand;
	TArray<FString> Formats;
	BITFIELD        bCreateNew         : 1;
	BITFIELD        bShowPropertySheet : 1;
	BITFIELD        bShowCategories    : 1;
	BITFIELD        bText              : 1;
	BITFIELD        bMulti             : 1;
	INT             AutoPriority;

	// Constructors.
	UFactory();
	void StaticConstructor();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UFactory interface.
	virtual UObject* FactoryCreateText(ULevel* InLevel, UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn ){ return NULL; }
	virtual UObject* FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const BYTE*& Buffer, const BYTE* BufferEnd, FFeedbackContext* Warn ){ return NULL; }
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, FFeedbackContext* Warn){ return NULL; }

	// UFactory functions.
	static UObject* StaticImportObject(UClass* Class, UObject* InOuter, FName Name, DWORD Flags, const TCHAR* Filename = "", UObject* Context = NULL, UFactory* Factory = NULL, const TCHAR* Parms = NULL, FFeedbackContext* Warn = GWarn);
};

// Import an object using a UFactory.
template<typename T>
T* ImportObject(ULevel* InLevel, UObject* Outer, FName Name, DWORD Flags, const TCHAR* Filename = "", UObject* Context = NULL, UFactory* Factory = NULL, const TCHAR* Parms = NULL, FFeedbackContext* Warn = GWarn)
{
	return static_cast<T*>(UFactory::StaticImportObject(T::StaticClass(), Outer, Name, Flags, Filename, Context, Factory, Parms, Warn));
}

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
