/*=============================================================================
	UnCorObj.h: Standard core object definitions.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPackage.
-----------------------------------------------------------------------------*/

//
// A package.
//
class CORE_API UPackage : public UObject
{
	DECLARE_CLASS(UPackage,UObject,0,Core)

	// Variables.
	void* DllHandle;
	UBOOL AttemptedBind;
	DWORD PackageFlags;

	// Constructors.
	UPackage();

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// UPackage interface.
	void* GetDllExport( const TCHAR* ExportName, UBOOL Checked );
};

/*-----------------------------------------------------------------------------
	USubsystem.
-----------------------------------------------------------------------------*/

//
// A subsystem.
//
class CORE_API USubsystem : public UObject, public FExec{
	DECLARE_ABSTRACT_CLASS(USubsystem,UObject,CLASS_Transient,Core)
	NO_DEFAULT_CONSTRUCTOR(USubsystem)

	// USubsystem interface.
	virtual void ActorDestroyedCallback(UObject*);
	virtual void CleanupLevel();
	virtual void Tick(FLOAT DeltaTime);
	virtual unsigned long GetMemory() const;
};

/*-----------------------------------------------------------------------------
	UCommandlet.
-----------------------------------------------------------------------------*/

//
// A command-line applet.
//
struct UCommandlet_eventMain_Parms{
	FString InParms;
	INT ReturnValue;
};
class CORE_API UCommandlet : public UObject{
	DECLARE_CLASS(UCommandlet,UObject,CLASS_Transient|CLASS_Abstract|CLASS_Localized,Core)
	FString HelpCmd, HelpOneLiner, HelpUsage, HelpWebLink;
	FStringNoInit HelpParm[16], HelpDesc[16];
	BITFIELD LogToStdout   :1;
	BITFIELD IsServer      :1;
	BITFIELD IsClient      :1;
	BITFIELD IsEditor      :1;
	BITFIELD LazyLoad      :1;
	BITFIELD ShowErrorCount:1;
	BITFIELD ShowBanner    :1;

	virtual INT Main(const TCHAR* Parms);

	/*
	*	For some reason LucasArts renamed this to 'Main' which
	*	results in infinite recursion when calling UCommandlet::Main(const TCHAR*)
	*	of a Commandlet that is written in UnrealScript.
	*	This Rewritten main function checks for a UnrealScript main function and
	*	calls it if found. If not it calls the virtual main function above.
	*	forceinline because otherwise the function from the dll gets called which is bad...
	*/
	__forceinline INT Main(const FString& InParms){
		UCommandlet_eventMain_Parms Parms;

		Parms.InParms = InParms;
		
		UFunction* MainFunc = FindFunction(NAME_Main);

		if(MainFunc)
			ProcessEvent(MainFunc, &Parms);
		else
			return Main(*InParms);

		return Parms.ReturnValue;
    }
};

/*-----------------------------------------------------------------------------
	ULanguage.
-----------------------------------------------------------------------------*/

//
// A language (special case placeholder class).
//
class CORE_API ULanguage : public UObject
{
	DECLARE_ABSTRACT_CLASS(ULanguage,UObject,CLASS_Transient,Core)
	NO_DEFAULT_CONSTRUCTOR(ULanguage)
	ULanguage* SuperLanguage;
};

/*-----------------------------------------------------------------------------
	UTextBuffer.
-----------------------------------------------------------------------------*/

//
// An object that holds a bunch of text.  The text is contiguous and, if
// of nonzero length, is terminated by a NULL at the very last position.
//
class CORE_API UTextBuffer : public UObject, public FOutputDevice
{
	DECLARE_CLASS(UTextBuffer,UObject,0,Core)

	// Variables.
	INT Pos, Top;
	FString Text;

	// Constructors.
	UTextBuffer( const TCHAR* Str=TEXT("") );

	// UObject interface.
	void Serialize( FArchive& Ar );

	// FOutputDevice interface.
	void Serialize( const TCHAR* Data, EName Event );
};

/*----------------------------------------------------------------------------
	USystem.
----------------------------------------------------------------------------*/

class CORE_API USystem : public USubsystem
{
	DECLARE_CLASS(USystem,USubsystem,CLASS_Config,Core)

	// Variables.
#if 1 //LMode added by Legend on 4/12/2000
	//
	// a licensee configurable INI setting that can be used to augment
	// the behavior of LicenseeVer -- especially useful in supporting
	// the transition from "before LiceseeVer" to the new build; for
	// salvaging maps that have licensee-specific data.
	//
	INT LicenseeMode; 
#endif
	INT PurgeCacheDays;
	FString SavePath;
	FString CachePath;
	FString CacheExt;
	TArray<FString> Paths;
	TArray<FName> Suppress;

	// Constructors.
	void StaticConstructor();
	USystem();

	// FExec interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
