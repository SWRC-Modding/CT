/*=============================================================================
	Core.h: Unreal core public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_CORE
#define _INC_CORE

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/

//API definition.
#ifndef CORE_API
#define CORE_API DLL_IMPORT
#endif

//=============================================================================================================================
/*
*	Can be defined if the headers (the whole folders like 'Core' or 'Engine') are not in the same folder as the source code
*	to help find the appropriate .lib file
*/
#ifndef HEADER_DIRECTORY
#define HEADER_DIRECTORY "../"
#endif

//Used to automatically link the proper lib for the dlls whose headers are currently included
#define LINK_LIB(name) __pragma(comment(lib, HEADER_DIRECTORY#name"/lib/"#name".lib"))
//=============================================================================================================================

LINK_LIB(Core)

//Build options.
#include "UnBuild.h"

//Time.
#define FIXTIME 4294967296.f
class FTime{
#define TIMETYP __int64
public:

	        FTime      ()               {v = 0;}
	        FTime      (float f)        {v=(TIMETYP)(f*FIXTIME);}
	        FTime      (double d)       {v=(TIMETYP)(d*FIXTIME);}
	float   GetFloat   ()               {return v/FIXTIME;}
	FTime   operator+  (float f) const  {return FTime(v+(TIMETYP)(f*FIXTIME));}
	float   operator-  (FTime t) const  {return (v-t.v)/FIXTIME;}
	FTime   operator*  (float f) const  {return FTime(v*f);}
	FTime   operator/  (float f) const  {return FTime(v/f);}
	FTime&  operator+= (float f)        {v=v+(TIMETYP)(f*FIXTIME); return *this;}
	FTime&  operator*= (float f)        {v=(TIMETYP)(v*f); return *this;}
	FTime&  operator/= (float f)        {v=(TIMETYP)(v/f); return *this;}
	int     operator== (FTime t)        {return v==t.v;}
	int     operator!= (FTime t)        {return v!=t.v;}
	int     operator>  (FTime t)        {return v>t.v;}
	FTime&  operator=  (const FTime& t) {v=t.v; return *this;}
private:
	FTime (TIMETYP i) {v=i;}
	TIMETYP v;
};

//Compiler specific include. Can't use a different compiler anyway...
#include "UnVcWin32.h"

//Global constants.
enum{MAXBYTE		= 0xff       };
enum{MAXWORD		= 0xffffU    };
enum{MAXDWORD		= 0xffffffffU};
enum{MAXSBYTE		= 0x7f       };
enum{MAXSWORD		= 0x7fff     };
enum{MAXINT		= 0x7fffffff };
enum{INDEX_NONE	= -1         };
enum{UNICODE_BOM   = 0xfeff     };
enum ENoInit{E_NoInit = 0};

//Single byte character set mappings. No unicode in Republic Commando...
#undef _TCHAR_DEFINED
#ifndef _TCHAR_DEFINED
	typedef ANSICHAR  TCHAR;
	typedef ANSICHARU TCHARU;
#endif
#undef TEXT
#define TEXT(s) s
#undef US
#define US FString("")
inline TCHAR    FromAnsi   (ANSICHAR In) { return In;                              }
inline TCHAR    FromUnicode(UNICHAR In) { return (_WORD)In<0x100 ? In : MAXSBYTE; }
inline ANSICHAR ToAnsi     (TCHAR In  ) { return (_WORD)In<0x100 ? In : MAXSBYTE; }
inline UNICHAR  ToUnicode  (TCHAR In  ) { return (BYTE)In;                        }

/*----------------------------------------------------------------------------
	Forward declarations.
----------------------------------------------------------------------------*/

//Objects.
class	UObject;
class		UExporter;
class		UFactory;
class		UField;
class			UConst;
class			UEnum;
class			UProperty;
class				UByteProperty;
class				UIntProperty;
class				UBoolProperty;
class				UFloatProperty;
class				UObjectProperty;
class					UClassProperty;
class				UNameProperty;
class				UStructProperty;
class               UStrProperty;
class               UArrayProperty;
class			UStruct;
class				UFunction;
class				UState;
class					UClass;
class		ULinker;
class			ULinkerLoad;
class			ULinkerSave;
class		UPackage;
class		USubsystem;
class			USystem;
class		UTextBuffer;
class       URenderDevice;
class		UPackageMap;

//Structs.
class FName;
class FArchive;
class FCompactIndex;
class FExec;
class FGuid;
class FMemCache;
class FMemStack;
class FPackageInfo;
class FTransactionBase;
class FUnknown;
class FRepLink;
class FArray;
class FLazyLoader;
class FString;
class FMalloc;

//Templates.
template<typename T>
class TArray;

template<typename T>
class TTransArray;

template<typename T>
class TLazyArray;

template<typename TK, typename TI>
class TMap;

template<typename TK, typename TI>
class TMultiMap;

//Globals.
CORE_API extern class FOutputDevice* GNull;

//EName definition.
#include "UnNames.h"

/*-----------------------------------------------------------------------------
	Abstract interfaces.
-----------------------------------------------------------------------------*/

//An output device.
class FOutputDevice{
public:
	//FOutputDevice interface.
	virtual void Serialize(const TCHAR* V, EName Event) = 0;
	virtual void Flush(){}

	//Simple text printing.
	void Log(EName Event, const TCHAR* Str);
	void Log(const TCHAR* Str);
	void Log(const FString& S);
	void Log(enum EName Type, const FString& S);
	void Logf(EName Event, const TCHAR* Fmt, ...);
	void Logf(const TCHAR* Fmt, ...);
};

//Error device.
class FOutputDeviceError : public FOutputDevice{
public:
	virtual void HandleError() = 0;
};

//Memory allocator.
class FMalloc{
public:
	virtual void Init(){}
	virtual void* Malloc(DWORD) = 0;
	virtual void* Realloc(void*, DWORD, DWORD) = 0;
	virtual void Free(void*) = 0;
	virtual DWORD GetAllocationSize(void*) = 0;
	virtual void TrackMemory(bool){}
	virtual void Compact(){}
	virtual void DumpAllocs(){}
	virtual void DumpAllocsBySize(int){}
	virtual void StartAllocProfile(int){}
	virtual void DumpAllocProfile(){}
	virtual void DumpFreeList(){}
	virtual void HeapCheck(){}
	virtual void SetWatermark(int){}
	virtual void IncrementWatermark(){}
	virtual void DecrementWatermark(){}
	virtual void SetAuxMalloc(FMalloc*){}
	virtual void LockMem(void*, unsigned long, bool){}
	virtual void vtpad1(){}
	virtual void vtpad2(){}
	virtual void Tick(){}
};

//Configuration database cache.
class FConfigCache{
public:
	virtual UBOOL GetBool(const TCHAR* Section, const TCHAR* Key, UBOOL& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetInt(const TCHAR* Section, const TCHAR* Key, INT& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetFloat(const TCHAR* Section, const TCHAR* Key, FLOAT& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetString(const TCHAR* Section, const TCHAR* Key, TCHAR* Value, INT Size, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetString(const TCHAR* Section, const TCHAR* Key, FString& Str, const TCHAR* Filename = NULL) = 0;
	virtual const TCHAR* GetStr(const TCHAR* Section, const TCHAR* Key, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetSection(const TCHAR* Section, TCHAR* Value, INT Size, const TCHAR* Filename = NULL) = 0;
	virtual TMultiMap<FName, FString>* GetSectionPrivate(const TCHAR* Section, UBOOL Force, UBOOL Const, const TCHAR* Filename = NULL) = 0;
	virtual void EmptySection(const TCHAR* Section, const TCHAR* Filename = NULL) = 0;
	virtual void SetBool(const TCHAR* Section, const TCHAR* Key, UBOOL Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetInt(const TCHAR* Section, const TCHAR* Key, INT Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetFloat(const TCHAR* Section, const TCHAR* Key, FLOAT Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value, const TCHAR* Filename = NULL) = 0;
	virtual void Flush(UBOOL Read, const TCHAR* Filename = NULL) = 0;
	virtual void UnloadFile(const TCHAR* Filename) = 0;
	virtual void UnloadInts(const TCHAR* Filename) = 0;
	virtual void Detach(const TCHAR* Filename) = 0;
	virtual void Init(const TCHAR* InSystem, const TCHAR* InUser, UBOOL RequireConfig) = 0;
	virtual void Exit() = 0;
	virtual void Dump(FOutputDevice& Ar) = 0;
	virtual void Serialize(FArchive& Ar) = 0;
	virtual ~FConfigCache(){}
};

//Any object that is capable of taking commands.
class CORE_API FExec{
public:
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) = 0;
};

//Notification hook.
class CORE_API FNotifyHook{
public:
	virtual void NotifyDestroy(void* Src){}
	virtual void NotifyPreChange(void* Src){}
	virtual void NotifyPostChange(void* Src){}
	virtual void NotifyExec(void* Src, const TCHAR* Cmd){}
};

//Interface for returning a context string.
class FContextSupplier{
public:
	virtual FString GetContext() = 0;
};

//A context for displaying modal warning messages.
class CORE_API FFeedbackContext : public FOutputDevice{
public:
	virtual UBOOL VARARGS YesNof(const TCHAR* Fmt, ...) = 0;
	virtual void BeginSlowTask(const TCHAR* Task, UBOOL StatusWindow) = 0;
	virtual void EndSlowTask() = 0;
	virtual UBOOL VARARGS StatusUpdatef(INT Numerator, INT Denominator, const TCHAR* Fmt, ...) = 0;
	virtual void SetContext(FContextSupplier* InSupplier) = 0;
	virtual void SetProgressRange(float, float);
	virtual void MapCheck_Show(){}
	virtual void MapCheck_ShowConditionally(){}
	virtual void MapCheck_Hide(){}
	virtual void MapCheck_Clear(){}
	virtual void MapCheck_Add(INT InType, void* InActor, const TCHAR* InMessage){}
};

//Class for handling undo/redo transactions among objects.
typedef void(*STRUCT_AR)(FArchive& Ar, void* TPtr);
typedef void(*STRUCT_DTOR)(void* TPtr);
class CORE_API FTransactionBase{
public:
	virtual void SaveObject(UObject* Object) = 0;
	virtual void SaveArray(UObject* Object, FArray* Array, INT Index, INT Count, INT Oper, INT ElementSize, STRUCT_AR Serializer, STRUCT_DTOR Destructor) = 0;
	virtual void Apply() = 0;
};

//File manager.
enum EFileTimes{
	FILETIME_Create,
	FILETIME_LastAccess,
	FILETIME_LastWrite,
};

enum EFileWrite{
	FILEWRITE_NoFail            = 0x01,
	FILEWRITE_NoReplaceExisting = 0x02,
	FILEWRITE_EvenIfReadOnly    = 0x04,
	FILEWRITE_Unbuffered        = 0x08,
	FILEWRITE_Append			= 0x10,
	FILEWRITE_AllowRead         = 0x20,
};

enum EFileRead{
	FILEREAD_NoFail             = 0x01,
};

class CORE_API FFileManager{
public:
	virtual void Init(UBOOL Startup){}
	virtual FArchive* CreateFileReader(const TCHAR* Filename, DWORD ReadFlags = 0, FOutputDevice* Error = GNull) = 0;
	virtual FArchive* CreateFileWriter(const TCHAR* Filename, DWORD WriteFlags = 0, FOutputDevice* Error = GNull) = 0;
	virtual INT FileSize(const TCHAR* Filename) = 0;
	virtual UBOOL Delete(const TCHAR* Filename, UBOOL RequireExists = 0, UBOOL EvenReadOnly = 0) = 0;
	virtual UBOOL Copy(const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly = 0, UBOOL Attributes = 0, void (*Progress)(FLOAT Fraction) = NULL) = 0;
	virtual UBOOL Move(const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly = 0, UBOOL Attributes = 0) = 0;
	virtual SQWORD GetGlobalTime(const TCHAR* Filename) = 0;
	virtual UBOOL SetGlobalTime(const TCHAR* Filename) = 0;
	virtual UBOOL MakeDirectory(const TCHAR* Path, UBOOL Tree = 0) = 0;
	virtual UBOOL DeleteDirectory(const TCHAR* Path, UBOOL RequireExists = 0, UBOOL Tree = 0) = 0;
	virtual TArray<FString> FindFiles(const TCHAR* Filename, UBOOL Files, UBOOL Directories) = 0;
	virtual UBOOL SetDefaultDirectory(const TCHAR* Filename) = 0;
	virtual FString GetDefaultDirectory() = 0;
	virtual const TCHAR* CalcHomeDir();
	virtual int Cache(const TCHAR*);
	virtual void vtpad(){}
	virtual QWORD GetFreeDiskSpace(const TCHAR*);
	virtual struct FFileStats GetStats(bool);
};

/*----------------------------------------------------------------------------
	Global variables.
----------------------------------------------------------------------------*/

//Core globals.
CORE_API extern FMemStack				GMem;
CORE_API extern FOutputDevice*			GLog;
CORE_API extern FOutputDevice*			GNull;
CORE_API extern FOutputDevice*		    GThrow;
CORE_API extern FOutputDeviceError*		GError;
CORE_API extern FFeedbackContext*		GWarn;
CORE_API extern FConfigCache*			GConfig;
CORE_API extern FTransactionBase*		GUndo;
CORE_API extern FOutputDevice*			GLogHook;
CORE_API extern FExec*					GExec;
CORE_API extern FMalloc*				GMalloc;
CORE_API extern FFileManager*			GFileManager;
CORE_API extern USystem*				GSys;
CORE_API extern UProperty*				GProperty;
CORE_API extern BYTE*					GPropAddr;
CORE_API extern USubsystem*				GWindowManager;
CORE_API extern TCHAR				    GErrorHist[4096];
CORE_API extern TCHAR                   GTrue[64], GFalse[64], GYes[64], GNo[64], GNone[64];
CORE_API extern TCHAR					GCdPath[];
CORE_API extern	DOUBLE					GSecondsPerCycle;
CORE_API extern	FTime					GTempTime;
CORE_API extern void					(*GTempFunc)(void*);
CORE_API extern SQWORD					GTicks;
CORE_API extern INT                     GScriptCycles;
CORE_API extern DWORD					GPageSize;
CORE_API extern DWORD					GProcessorCount;
CORE_API extern DWORD					GPhysicalMemory;
CORE_API extern DWORD                   GUglyHackFlags;
CORE_API extern UBOOL					GIsScriptable;
CORE_API extern UBOOL					GIsEditor;
CORE_API extern UBOOL					GIsUCC;
CORE_API extern UBOOL					GIsBenchmarking;
CORE_API extern UBOOL					GIsClient;
CORE_API extern UBOOL					GIsServer;
CORE_API extern UBOOL					GIsCriticalError;
CORE_API extern UBOOL					GIsStarted;
CORE_API extern UBOOL					GIsRunning;
CORE_API extern UBOOL					GIsSlowTask;
CORE_API extern UBOOL					GIsGuarded;
CORE_API extern UBOOL					GIsRequestingExit;
CORE_API extern UBOOL					GIsStrict;
CORE_API extern UBOOL                   GScriptEntryTag;
CORE_API extern UBOOL                   GLazyLoad;
CORE_API extern UBOOL					GUnicode;
CORE_API extern UBOOL					GUnicodeOS;
CORE_API extern UBOOL					GUseFrontEnd;
CORE_API extern class FGlobalMath		GMath;
CORE_API extern class FArchive*         GDummySave;
CORE_API extern DWORD					GCurrentViewport;
CORE_API extern FString					GSavePath;
CORE_API extern FString					GGlobalSettingsPath;
CORE_API extern FString					GGlobalSettingsSaveName;


//Per module globals.
extern "C" DLL_EXPORT TCHAR GPackage[];

//Normal includes.
#include "UnFile.h"			//Low level utility code.
#include "UnObjVer.h"		//Object version info.
#include "UnArc.h"			//Archive class.
#include "UnTemplate.h"     //Dynamic arrays.
#include "UnName.h"			//Global name subsystem.
#include "UnStack.h"		//Script stack definition.
#include "UnObjBas.h"		//Object base class.
#include "UnCoreNet.h"		//Core networking.
#include "UnCorObj.h"		//Core object class definitions.
#include "UnClass.h"		//Class definition.
#include "UnType.h"			//Base property type.
#include "UnScript.h"		//Script class.
#include "UFactory.h"		//Factory definition.
#include "UExporter.h"		//Exporter definition.
#include "UnCache.h"		//Cache based memory management.
#include "UnMem.h"			//Stack based memory management.
#include "UnCId.h"          //Cache ID's.
#include "UnBits.h"         //Bitstream archiver.
#include "UnMath.h"         //Vector math functions.

#include "Core.inl"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif