/*=============================================================================
	UnCorObj.h: Standard core object definitions.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPackage.
-----------------------------------------------------------------------------*/

/**
 * @brief A package.
 */
class CORE_API UPackage : public UObject{
	DECLARE_CLASS(UPackage,UObject,0,Core)

	// Variables.
	void* DllHandle;
	UBOOL AttemptedBind;
	DWORD PackageFlags;
	UBOOL bDirty; //!< Used by the editor to determine if a package has been changed

	// Constructors.
	UPackage();

	// UObject interface.
	virtual void Destroy();
	virtual void Serialize(FArchive& Ar);

	// UPackage interface.
	void* GetDllExport(const TCHAR* ExportName, UBOOL Checked);
};

/*-----------------------------------------------------------------------------
	USubsystem.
-----------------------------------------------------------------------------*/

/**
 * @brief A subsystem.
 */
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

struct UCommandlet_eventMain_Parms{
	FString InParms;
	INT ReturnValue;
};

/**
 * @brief A command-line applet.
 */
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

	/**
	* @brief Entry point.
	*
	* For some reason LucasArts removed the 'event' prefix
	* from functions that call UnrealScript events which
	* results in UCommandlet::Main calling itself recursively.
	* This rewritten main function checks for an UnrealScript function
	* called main and calls it if found. Else it calls the virtual main function.
	* It needs to be forceinline because otherwise the function from the dll is
	* called which is bad...
	*/
	FORCEINLINE INT Main(const FString& InParms){
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

/**
 * @brief A language (special case placeholder class).
 */
class CORE_API ULanguage : public UObject{
	DECLARE_ABSTRACT_CLASS(ULanguage,UObject,CLASS_Transient,Core)
	NO_DEFAULT_CONSTRUCTOR(ULanguage)

	ULanguage* SuperLanguage;
};

/*-----------------------------------------------------------------------------
	UTextBuffer.
-----------------------------------------------------------------------------*/

/**
 * @brief An object that holds a bunch of text.
 *
 * The text is contiguous and, if of nonzero length,
 * is terminated by a NULL at the very last position.
 */
class CORE_API UTextBuffer : public UObject, public FOutputDevice{
	DECLARE_CLASS(UTextBuffer,UObject,0,Core)

	// Variables.
	INT Pos, Top;
	FString Text;

	// Constructors.
	UTextBuffer(const TCHAR* Str = "");

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

	// FOutputDevice interface.
	virtual void Serialize(const TCHAR* Data, EName Event);
};

/*----------------------------------------------------------------------------
	USystem.
----------------------------------------------------------------------------*/

class CORE_API USystem : public USubsystem{
	DECLARE_CLASS(USystem,USubsystem,CLASS_Config,Core)

	// Variables.
	INT LicenseeMode;
	INT PurgeCacheDays;
	FString SavePath;
	FString CachePath;
	FString CacheExt;
	FString SourcePath;
	TArray<FString> Paths;
	TArray<FName> Suppress;
	UBOOL bLinearLoad;
	UBOOL bLinearSave;

	// Constructors.
	void StaticConstructor();
	USystem();

	// FExec interface.
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar = *GLog);
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
