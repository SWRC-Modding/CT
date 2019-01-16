/*=============================================================================
	UExporter.cpp: Exporter class definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	UExporter.
----------------------------------------------------------------------------*/

//
// An object responsible for exporting other objects to files.
//
class CORE_API UExporter : public UObject{
	DECLARE_ABSTRACT_CLASS(UExporter,UObject,0,Core)

	// Variables.
	UClass*         SupportedClass;
	TArray<FString> Formats;
	INT				TextIndent;
	BITFIELD		bText  : 1;
	BITFIELD        bMulti : 1;

	// Constructor.
	UExporter();
	void StaticConstructor();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// UExporter interface.
	virtual UBOOL ExportText(UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn ){ return 0; }
	virtual UBOOL ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn){ return 0; }

	static UExporter* FindExporter(UObject* Object, const TCHAR* Filetype);
	static INT ExportToFile(UObject* Object, UExporter* Exporter, const TCHAR* Filename, UBOOL NoReplaceIdentical = 0, UBOOL Prompt = 0);
	static void ExportToArchive(UObject* Object, UExporter* Exporter, FArchive& Ar, const TCHAR* FileType);
	static void ExportToOutputDevice(UObject* Object, UExporter* Exporter, FOutputDevice& Out, const TCHAR* FileType, INT Indent);
};

class CORE_API UObjectExporterT3D : public UExporter{
	DECLARE_CLASS(UObjectExporterT3D,UExporter,0,Core)

	// Constructor.
	void StaticConstructor();

	// UExporter interface.
	virtual UBOOL ExportText(UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn);
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
