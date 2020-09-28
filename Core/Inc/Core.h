/*=============================================================================
	Core.h: Unreal core public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#pragma once

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/

// API definition.
#ifndef CORE_API
#define CORE_API DLL_IMPORT
#endif

//===========================================================================================
#ifdef DISABLE_LINK_LIB
#define LINK_LIB(name)
#else
// Used to automatically link the proper lib for the dlls whose headers are currently included
#define LINK_LIB(name) __pragma(comment(lib, "../"#name"/lib/"#name".lib"))
#endif
//===========================================================================================

LINK_LIB(Core)

// Build options.
#include "UnBuild.h"

// Time.
#define FIXTIME 4294967296.0f
class FTime{
	typedef __int64 TIMETYP;
public:
			FTime      ()              { v = 0; }
			FTime      (float f)       { v = (TIMETYP)(f * FIXTIME); }
			FTime      (double d)      { v = (TIMETYP)(d * FIXTIME); }
	float   GetFloat   ()              { return v / FIXTIME; }
	FTime   operator+  (float f) const { return FTime(v + (TIMETYP)(f * FIXTIME)); }
	float   operator-  (FTime t) const { return (v - t.v) / FIXTIME; }
	FTime   operator*  (float f) const { return FTime(v * f); }
	FTime   operator/  (float f) const { return FTime(v / f); }
	FTime&  operator+= (float f)       { v = v + (TIMETYP)(f * FIXTIME); return *this; }
	FTime&  operator*= (float f)       { v = (TIMETYP)(v * f); return *this; }
	FTime&  operator/= (float f)       { v = (TIMETYP)(v / f); return *this; }
	int     operator== (FTime t)       { return v == t.v; }
	int     operator!= (FTime t)       { return v != t.v; }
	int     operator>  (FTime t)       { return v > t.v; }
	FTime&  operator=  (const FTime& t){ v=t.v; return *this; }

private:
	TIMETYP v;

	FTime(TIMETYP i) : v(i){}
};

// Compiler specific include.
#ifdef _MSC_VER
	#include "UnVcWin32.h"
#else
	#error Must use Visual Studio (Ideally .NET 2003)
#endif

// Global constants.
enum{ MAXBYTE       = 0xff        };
enum{ MAXWORD       = 0xffffU     };
enum{ MAXDWORD      = 0xffffffffU };
enum{ MAXSBYTE      = 0x7f        };
enum{ MAXSWORD      = 0x7fff      };
enum{ MAXINT        = 0x7fffffff  };
enum{ INDEX_NONE    = -1          };
enum{ UNICODE_BOM   = 0xfeff      };
enum ENoInit{ E_NoInit = 0 };

enum ERunningOS{
	OS_WIN95,
	OS_WIN98,
	OS_WINME,
	OS_WIN2K,
	OS_WINXP,
	OS_WINNT,
	OS_UNKNOWN = 255
};

// Multi byte character set mappings. No wchar_t in Republic Commando...
typedef ANSICHAR  TCHAR;
typedef ANSICHARU TCHARU;

#undef TEXT
#define TEXT(s) s

inline TCHAR    FromUnicode(UNICHAR In){ return (_WORD)In < 0x100 ? In : MAXSBYTE; }
inline ANSICHAR	ToAnsi(TCHAR In){ return (_WORD)In < 0x100 ? In : MAXSBYTE; }

/*----------------------------------------------------------------------------
	Forward declarations.
----------------------------------------------------------------------------*/

// Objects.
class UObject;
class	UExporter;
class	UFactory;
class	UField;
class		UConst;
class		UEnum;
class		UProperty;
class			UByteProperty;
class			UIntProperty;
class			UBoolProperty;
class			UFloatProperty;
class			UObjectProperty;
class				UClassProperty;
class			UNameProperty;
class			UStructProperty;
class				UStrProperty;
class				UArrayProperty;
class			UDelegateProperty;
class		UStruct;
class			UFunction;
class			UState;
class				UClass;
class	ULinker;
class		ULinkerLoad;
class		ULinkerSave;
class	UPackage;
class	USubsystem;
class		USystem;
class	UTextBuffer;
class	 URenderDevice;
class	UPackageMap;
class	UDebugger; //DEBUGGER

// Structs.
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
struct FFrame;

struct FMemCount{
	DWORD Allocations;
	DWORD Used;
	DWORD Reserved;
};

// Templates.
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

// Native function.
typedef void(UObject::*Native)(FFrame& TheStack, void* const Result);

// Single entry in a UClass' native funtion table.
template<typename T>
struct FNativeEntry{
	const TCHAR* name;
	void(T::*Func)(FFrame&, void*);
	INT Num;
};

// Globals.
CORE_API extern class FOutputDevice* GNull;

// EName definition.
#include "UnNames.h"

/*-----------------------------------------------------------------------------
	Abstract interfaces.
-----------------------------------------------------------------------------*/

// An output device.
class CORE_API FOutputDevice{
public:
	// FOutputDevice interface.
	virtual void Serialize(const TCHAR* V, EName Event) = 0;
	virtual void Flush(){}

	// Simple text printing.
	void Log(EName Event, const TCHAR* Str);
	void Log(const TCHAR* Str);
	void Log(const FString& S);
	void Log(enum EName Type, const FString& S);
	void Logf(EName Event, const TCHAR* Fmt, ...);
	void Logf(const TCHAR* Fmt, ...);
};

// Error device.
class CORE_API FOutputDeviceError : public FOutputDevice{
public:
	virtual void HandleError() = 0;
};

// Memory allocator.
class CORE_API FMalloc{
public:
	void AddStat(FMemCount& Count, void* Ptr, DWORD Size, UBOOL IsUsed);

	virtual void Init(){}
	virtual void* Malloc(DWORD Size) = 0;
	virtual void* Realloc(void* Ptr, DWORD Size, DWORD Slack) = 0;
	virtual void Free(void* Ptr) = 0;
	virtual DWORD GetAllocationSize(void* Ptr) = 0;
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
	virtual void LockMem(void* Ptr, DWORD Size, bool bLock){}
	virtual void vtpad1() = 0;
	virtual void vtpad2() = 0;
	virtual void Tick(){}
};

// Single section in a config file.
typedef TMultiMap<FName, FString> FConfigSection;

// Configuration database cache.
class FConfigCache{
public:
	virtual UBOOL GetBool(const TCHAR* Section, const TCHAR* Key, UBOOL& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetInt(const TCHAR* Section, const TCHAR* Key, INT& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetFloat(const TCHAR* Section, const TCHAR* Key, FLOAT& Value, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetString(const TCHAR* Section, const TCHAR* Key, TCHAR* Value, INT Size, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetString(const TCHAR* Section, const TCHAR* Key, FString& Str, const TCHAR* Filename = NULL) = 0;
	virtual const TCHAR* GetStr(const TCHAR* Section, const TCHAR* Key, const TCHAR* Filename = NULL) = 0;
	virtual UBOOL GetSection(const TCHAR* Section, TCHAR* Value, INT Size, const TCHAR* Filename = NULL) = 0;
	virtual FConfigSection* GetSectionPrivate(const TCHAR* Section, UBOOL Force, UBOOL Const, const TCHAR* Filename = NULL) = 0;
	virtual void EmptySection(const TCHAR* Section, const TCHAR* Filename = NULL) = 0;
	virtual void SetBool(const TCHAR* Section, const TCHAR* Key, UBOOL Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetInt(const TCHAR* Section, const TCHAR* Key, INT Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetFloat(const TCHAR* Section, const TCHAR* Key, FLOAT Value, const TCHAR* Filename = NULL) = 0;
	virtual void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value, const TCHAR* Filename = NULL) = 0;
	virtual void Flush(UBOOL Read, const TCHAR* Filename = NULL, const char* Section = NULL) = 0;
	virtual void UnloadFile(const TCHAR* Filename) = 0;
	virtual void UnloadInts(const TCHAR* Filename) = 0;
	virtual void Detach(const TCHAR* Filename) = 0;
	virtual void Init(const TCHAR* InSystem, const TCHAR* InUser, UBOOL RequireConfig) = 0;
	virtual void Exit() = 0;
	virtual void Dump(FOutputDevice& Ar) = 0;
	virtual void Serialize(FArchive& Ar) = 0;
	virtual ~FConfigCache(){}
};

// Any object that is capable of taking commands.
class CORE_API FExec{
public:
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) = 0;
};

// Notification hook.
class CORE_API FNotifyHook{
public:
	virtual void NotifyDestroy(void* Src){}
	virtual void NotifyPreChange(void* Src){}
	virtual void NotifyPostChange(void* Src){}
	virtual void NotifyExec(void* Src, const TCHAR* Cmd){}
};

// Interface for returning a context string.
class FContextSupplier{
public:
	virtual FString GetContext() = 0;
};

// A context for displaying modal warning messages.
class FFeedbackContext : public FOutputDevice{
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

// Class for handling undo/redo transactions among objects.
typedef void(*STRUCT_AR)(FArchive& Ar, void* TPtr);
typedef void(*STRUCT_DTOR)(void* TPtr);
class CORE_API FTransactionBase{
public:
	virtual void SaveObject(UObject* Object) = 0;
	virtual void SaveArray(UObject* Object, FArray* Array, INT Index, INT Count, INT Oper, INT ElementSize, STRUCT_AR Serializer, STRUCT_DTOR Destructor) = 0;
	virtual void Apply() = 0;
};

// File manager.

enum EFileWrite{
	FILEWRITE_NoFail            = 0x01,
	FILEWRITE_NoReplaceExisting = 0x02,
	FILEWRITE_EvenIfReadOnly    = 0x04,
	FILEWRITE_Unbuffered        = 0x08,
	FILEWRITE_Append            = 0x10,
	FILEWRITE_AllowRead         = 0x20,
};

enum EFileRead{
	FILEREAD_NoFail             = 0x01,
};

enum ECopyCompress{
	FILECOPY_Normal             = 0x00,
	FILECOPY_Compress           = 0x01,
	FILECOPY_Decompress         = 0x02,
};

enum ECopyResult{
	COPY_OK                     = 0x00,
	COPY_MiscFail               = 0x01,
	COPY_ReadFail               = 0x02,
	COPY_WriteFail              = 0x03,
	COPY_CompFail               = 0x04,
	COPY_DecompFail             = 0x05,
	COPY_Canceled               = 0x06,
};

#define COMPRESSED_EXTENSION	".uz2"

struct FCopyProgress{
	virtual UBOOL Poll(FLOAT Fraction) = 0;
};

struct FFileStats{
	INT NumOpens;
	INT NumSeeks;
	INT NumSkips;
	INT NumReads;
	DWORD ReadsSize;
	INT NumSerialized;
	DWORD SerializedSize;

	FFileStats();

	void Clear();
	FString Describe() const;
};

class CORE_API FFileManager{
public:
	virtual void Init(UBOOL Startup){}
	virtual FArchive* CreateFileReader(const TCHAR* Filename, DWORD ReadFlags = 0, FOutputDevice* Error = GNull) = 0;
	virtual FArchive* CreateFileWriter(const TCHAR* Filename, DWORD WriteFlags = 0, FOutputDevice* Error = GNull) = 0;
	virtual INT FileSize(const TCHAR* Filename) = 0;
	virtual UBOOL Delete(const TCHAR* Filename, UBOOL RequireExists = 0, UBOOL EvenReadOnly = 0) = 0;
	virtual UBOOL Copy(const TCHAR* Dest, const TCHAR* Src, UBOOL Replace = 1, UBOOL EvenIfReadOnly = 0, UBOOL Attributes = 0, void(*Progress)(FLOAT Fraction) = NULL) = 0;
	virtual UBOOL Move(const TCHAR* Dest, const TCHAR* Src, UBOOL Replace = 1, UBOOL EvenIfReadOnly = 0, UBOOL Attributes = 0) = 0;
	virtual SQWORD GetGlobalTime(const TCHAR* Filename) = 0;
	virtual UBOOL SetGlobalTime(const TCHAR* Filename) = 0;
	virtual UBOOL MakeDirectory(const TCHAR* Path, UBOOL Tree = 0) = 0;
	virtual UBOOL DeleteDirectory(const TCHAR* Path, UBOOL RequireExists = 0, UBOOL Tree = 0) = 0;
	virtual void FindFiles(TArray<FString>& Result, const TCHAR* Filename, UBOOL Files, UBOOL Directories) = 0;
	virtual UBOOL SetDefaultDirectory(const TCHAR* Filename) = 0;
	virtual FString GetDefaultDirectory() = 0;
	virtual const TCHAR* CalcHomeDir(); // Returns appBaseDir()
	virtual int Cache(const TCHAR*){ return 0; }
	virtual FFileStats GetStats(bool){ return FFileStats(); }
	virtual QWORD GetFreeDiscSpace(const TCHAR*){ return 0; }
};

//
// File Streaming.
//

enum EFileStreamType{
	ST_Regular,
	ST_Ogg,
	ST_OggLooping
};

struct FStream{
	void* Data;
	void* Handle;
	void* TDD;		// type dependent data
	INT   FileSeek;
	INT   ChunkSize;
	INT   ChunksRequested;
	INT   Locked;
	INT   Used;
	INT   EndOfFile;
	EFileStreamType	Type;
};

class CORE_API FFileStream{
public:
	static FFileStream* Init(INT MaxStreams);
	static void Destroy();

	// Interface functions.
	INT CreateStream(const TCHAR* Filename, INT ChunkSize, INT InitialChunks, void* Data, EFileStreamType Type, void* TDD);
	void RequestChunks(INT StreamId, INT Chunks, void* Data);
	UBOOL QueryStream(INT StreamId, INT& ChunksQueued);
	void DestroyStream(INT StreamId, UBOOL ReadQueuedChunks);

	// Only use the below functions in the thread's main loop.
	UBOOL Read(INT StreamId, INT Bytes);
	UBOOL Create(INT StreamId, const TCHAR* Filename);
	UBOOL Destroy(INT StreamId);
	void Enter(INT StreamId);
	void Leave(INT StreamId);

	static FFileStream* Instance;
	static INT StreamIndex;
	static INT MaxStreams;
	static FStream* Streams;
	static INT Destroyed;

private:
	FFileStream(){}
	~FFileStream(){}
};

class FEdLoadError;

/*----------------------------------------------------------------------------
	Global variables.
----------------------------------------------------------------------------*/

// Core globals.
CORE_API extern FMemStack               GMem;                       // Global memory stack
CORE_API extern FOutputDevice*          GLog;                       // Regular logging
CORE_API extern FOutputDeviceError*     GError;                     // Critical errors
CORE_API extern FOutputDevice*          GNull;                      // Log to nowhere
CORE_API extern FOutputDevice*          GThrow;                     // Exception thrower
CORE_API extern FFeedbackContext*       GWarn;                      // User interaction and non critical warnings
CORE_API extern FConfigCache*           GConfig;                    // Configuration database cache
CORE_API extern FTransactionBase*       GUndo;                      // Transaction tracker, non-NULL when a transaction is in progress
CORE_API extern FOutputDevice*          GLogHook;                   // Launch log output hook
CORE_API extern FExec*                  GExec;                      // Launch command-line exec hook
CORE_API extern FMalloc*                GMalloc;                    // Memory allocator
CORE_API extern FFileManager*           GFileManager;               // File manager
CORE_API extern USystem*                GSys;                       // System control code
CORE_API extern UProperty*              GProperty;                  // Property for UnrealScript interpreter
CORE_API extern BYTE*                   GPropAddr;                  // Property address for UnrealScript interpreter
CORE_API extern UObject*                GPropObject;                // Object with Property for UnrealScript interpreter
CORE_API extern DWORD                   GRuntimeUCFlags;            // Property for storing flags between calls to bytecode functions
CORE_API extern USubsystem*             GWindowManager;             // Window update routine called once per tick
CORE_API extern TCHAR                   GErrorHist[4096];           // For building call stack text dump in guard/unguard mechanism
CORE_API extern TCHAR                   GYes[64];                   // Localized "yes" text
CORE_API extern TCHAR                   GNo[64];                    // Localized "no" text
CORE_API extern TCHAR                   GTrue[64];                  // Localized "true" text
CORE_API extern TCHAR                   GFalse[64];                 // Localized "false" text
CORE_API extern TCHAR                   GNone[64];                  // Localized "none" text
CORE_API extern TCHAR                   GCdPath[];                  // Cd path, if any
CORE_API extern DOUBLE                  GSecondsPerCycle;           // Seconds per CPU cycle for this PC
CORE_API extern DOUBLE                  GLastFNamePurgeTime;
CORE_API extern DOUBLE                  GTempDouble;                // Used during development for timing
CORE_API extern void                    (*GTempFunc)(void*);        // Used during development for debug hooks
CORE_API extern SQWORD                  GTicks;                     // Number of non-persistent ticks thus far in this level, for profiling
CORE_API extern DWORD                   GPageSize;                  // Operating system page size
CORE_API extern DWORD                   GProcessorCount;            // Number of CPUs in this PC
CORE_API extern DWORD                   GPhysicalMemory;            // Bytes of physical memory in this PC
CORE_API extern DWORD                   GUglyHackFlags;             // Flags for passing around globally hacked stuff
CORE_API extern UBOOL                   GIsBenchmarking;            // Whether we are in benchmark mode or not
CORE_API extern UBOOL                   GIsClient;                  // Whether engine was launched as a client
CORE_API extern UBOOL                   GIsCriticalError;           // An appError() has occured
CORE_API extern UBOOL                   GIsEditor;                  // Whether engine was launched for editing
CORE_API extern UBOOL                   GIsGarbageCollecting;
CORE_API extern UBOOL                   GIsGuarded;                 // Whether execution is happening within main()/WinMain()'s try/catch handler
CORE_API extern UBOOL                   GIsLoadingLevel;
CORE_API extern UBOOL                   GIsOpenGL;
CORE_API extern UBOOL                   GIsPixomatic;
CORE_API extern UBOOL                   GIsPurgingFNames;
CORE_API extern UBOOL                   GIsRequestingExit;          // Indicates that MainLoop() should be exited at the end of the current iteration
CORE_API extern UBOOL                   GIsRunning;                 // Whether execution is happening within MainLoop()
CORE_API extern UBOOL                   GIsScriptable;              // Whether script execution is allowed
CORE_API extern UBOOL                   GIsServer;                  // Whether engine was launched as a server, true if GIsClient
CORE_API extern UBOOL                   GIsSlowTask;                // Whether there is a slow task in progress
CORE_API extern UBOOL                   GIsStarted;                 // Whether execution is happening from within main()/WinMain()
CORE_API extern UBOOL                   GScriptEntryTag;            // Number of recursive UnrealScript calls currently on the stack
CORE_API extern UBOOL                   GIsStrict;                  // Causes all UnrealScript execution warnings to be fatal errors
CORE_API extern UBOOL                   GLazyLoad;                  // Whether TLazyLoad arrays should be lazy-loaded or not
CORE_API extern UBOOL                   GIsUCC;                     // Is UCC running?
CORE_API extern UBOOL                   GUseSmallPools;
CORE_API extern UBOOL                   GUseFrontEnd;
CORE_API extern UBOOL                   GEdSelectionLock;           // Are selections locked? (you can't select/deselect additional actors)
CORE_API extern UBOOL                   GEdShowFogInViewports;      // Show distance fog in viewports?
CORE_API extern UBOOL                   GBuildingScripts;
CORE_API extern UBOOL                   GIsUTracing;
CORE_API extern class FGlobalMath       GMath;                      // Math code
CORE_API extern class FArchive*         GDummySave;                 // No-op save archive
CORE_API extern FFileStream*            GFileStream;                // File streaming
CORE_API extern FLOAT                   GAudioMaxRadiusMultiplier;  // Max distance = Radius * GAudioMaxRadiusMultiplier
CORE_API extern FLOAT                   GAudioDefaultRadius;        // Default radius for PlayOwnedSound
CORE_API extern UDebugger*              GDebugger;                  // Unrealscript Debugger
CORE_API extern QWORD                   GMakeCacheIDIndex;          // Cache ID
CORE_API extern FString                 GBuildLabel;
CORE_API extern FString                 GMachineOS;
CORE_API extern FString                 GMachineCPU;
CORE_API extern FString                 GMachineVideo;
CORE_API extern FString                 GSavePath;
CORE_API extern FString                 GCurrProfilePath;
CORE_API extern FString                 GGlobalSettingsPath;
CORE_API extern FString                 GGlobalSettingsSaveName;
CORE_API extern FLOAT                   NEAR_CLIPPING_PLANE;
CORE_API extern FLOAT                   FAR_CLIPPING_PLANE;
CORE_API extern ERunningOS              GRunningOS;

// Per module globals.
extern "C" DLL_EXPORT TCHAR GPackage[];

// Normal includes.
#include "UnFile.h"	     // Low level utility code.
#include "UnObjVer.h"    // Object version info.
#include "UnArc.h"	     // Archive class.
#include "UnTemplate.h"  // Dynamic arrays.
#include "UnName.h"	     // Global name subsystem.
#include "UnStack.h"     // Script stack definition.
#include "UnObjBas.h"    // Object base class.
#include "UnCoreNet.h"   // Core networking.
#include "UnCorObj.h"    // Core object class definitions.
#include "UnClass.h"     // Class definition.
#include "UnType.h"      // Base property type.
#include "UnScript.h"    // Script class.
#include "UFactory.h"    // Factory definition.
#include "UExporter.h"   // Exporter definition.
#include "UnCache.h"     // Cache based memory management.
#include "UnMem.h"       // Stack based memory management.
#include "UnCId.h"       // Cache ID's.
#include "UnBits.h"      // Bitstream archiver.
#include "UnMath.h"      // Vector math functions.

// Worker class for tracking loading errors in the editor
class CORE_API FEdLoadError{
public:
	FEdLoadError(){}

	FEdLoadError(INT InType, TCHAR* InDesc){
		Type = InType;
		Desc = InDesc;
	}

	~FEdLoadError(){}

	// The types of things that could be missing.
	enum{
		TYPE_FILE,		// A totally missing file
		TYPE_RESOURCE	// Texture/Sound/StaticMesh/etc
	};

	INT Type;		// TYPE_
	FString Desc;	// Description of the error

	UBOOL operator==( const FEdLoadError& LE ) const{
		return Type==LE.Type && Desc==LE.Desc;
	}

	FEdLoadError& operator=(const FEdLoadError Other){
		Type = Other.Type;
		Desc = Other.Desc;

		return *this;
	}
};

CORE_API extern TArray<FEdLoadError>	GEdLoadErrors; /*  For keeping track of load errors in the editor */

/*
 * Archive for counting memory usage.
 */
class CORE_API FArchiveCountMem : public FArchive{
public:
	FArchiveCountMem(UObject* Src);

	// Overrides
	virtual void CountBytes(char, const FMemCount&);
	virtual void CountBytes(void*, unsigned long);
	virtual UObject* GetResourceObject(){ return ResourceObject; }

	FMemCount TotalMem() const;

protected:
	FMemCount Count;
	char Padding2[84];
	UObject* ResourceObject;
};

enum{
	MCTYPE_ERROR,
	MCTYPE_WARNING,
	MCTYPE_NOTE
};

typedef struct{
	INT Type;
	AActor* Actor;
	FString Message;
} MAPCHECK;

// A convenience to allow referring to axis' by name instead of number
enum EAxis{
	AXIS_X,
	AXIS_Y,
	AXIS_Z
};

// Very basic abstract debugger class.
class UDebugger{ //DEBUGGER
public:
	virtual void DebugInfo(UObject* Debugee, FFrame* Stack, FString InfoType, int LineNumber, int InputPos) = 0;
	virtual void NotifyAccessedNone() = 0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
