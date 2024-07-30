#include "Editor.h"
#include "Window.h"
#include "FOutputDeviceWindowsError.h"
#include "FOutputDeviceFile.h"
#include "FConfigCacheIni.h"
#include "../Res/resource.h"

IMPLEMENT_PACKAGE(ModEd);

/*
 * Custom configuration that converts absolute paths to relative ones, making it easier to copy or move the game directory and access the correct files.
 */
class FConfigCacheIniEditor : public FConfigCacheIni{
public:
	static FConfigCache* Factory()
	{
		return new FConfigCacheIniEditor();
	}

	void Init(const TCHAR* InSystem, const TCHAR* InUser, UBOOL RequireConfig)
	{
		FConfigCacheIni::Init(InSystem, InUser, RequireConfig);
		NormalizeMRUPaths();
	}

	void Flush(UBOOL Read, const TCHAR* Filename, const TCHAR* Section)
	{
		NormalizeMRUPaths();
		FConfigCacheIni::Flush(Read, Filename, Section);
	}

private:
	void NormalizeMRUPaths()
	{
		const TCHAR* BaseDir = appBaseDir();
		INT BaseDirLen = appStrlen(BaseDir);

		// Remove the system directory from the path so the base dir points to GameData

		while(BaseDirLen > 0 && (BaseDir[BaseDirLen - 1] == '\\' || BaseDir[BaseDirLen - 1] == '/'))
			--BaseDirLen;

		if(BaseDirLen > 6 && appStrnicmp(BaseDir + BaseDirLen - 6, "System", 6) == 0)
		{
			BaseDirLen -= 6;

			for(INT i = 0; i < 8; ++i)
			{
				FString Key = FString::Printf("MRUItem%i", i);
				FString MRUItem;

				if(GetFString("MRU", *Key, MRUItem, "UnrealEd.ini"))
				{
					if(MRUItem.Len() > BaseDirLen && appStrnicmp(*MRUItem, BaseDir, BaseDirLen) == 0)
					{
						MRUItem = FStringTemp("..") * MRUItem.Right(MRUItem.Len() - BaseDirLen);
						SetString("MRU", *Key, *MRUItem, "UnrealEd.ini");
					}
				}
			}
		}
	}
};

/*
 * HACK:
 * The main window is not exported but it can be obtained using its address.
 */
static WWindow* GetMainWindow()
{
	return *reinterpret_cast<WWindow**>(0x10FE39D4);
}

static WNDPROC OriginalMainWindowProc = NULL;

LRESULT CALLBACK MainWindowProcOverride(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(Msg == WM_INPUT)
	{
		RAWINPUT RawInput;
		UINT RawInputSize = sizeof(RawInput);

		if(GET_RAWINPUT_CODE_WPARAM(wParam) != RIM_INPUT)
			return 0;

		if(GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, &RawInput, &RawInputSize, sizeof(RAWINPUTHEADER)) != (UINT)-1)
		{
			if(RawInput.header.dwType == RIM_TYPEMOUSE)
			{

			}
		}

		DefWindowProcA(hWnd, Msg, wParam, lParam);
		return 0;
	}

	return OriginalMainWindowProc(hWnd, Msg, wParam, lParam);
}

static void(__fastcall*OriginalUUnrealEdEngineTick)(UEditorEngine*, DWORD, FLOAT) = NULL;

/*
 * Override for UEditorEngine::Tick used to do initial setup because the engine is fully loaded when it is called for the first time.
 */
static void __fastcall UnrealEdEngineTickOverride(UEditorEngine* Self, DWORD Edx, FLOAT DeltaTime)
{
	OriginalUUnrealEdEngineTick(Self, Edx, DeltaTime);

	UBOOL LoadMRU = 1;

	if(!GConfig->GetBool("ModEd", "LoadMRU", LoadMRU, "UnrealEd.ini"))
		GConfig->SetBool("ModEd", "LoadMRU", LoadMRU, "UnrealEd.ini");

	if(LoadMRU)
	{
		FString MapPath;

		if(GConfig->GetFString("MRU", "MRUItem0", MapPath, "UnrealEd.ini") && GFileManager->FileSize(*MapPath) > 0)
		{
			/*
			 * HACK:
			 * Opening the map using the command MAP LOAD FILE=... works but it does not set the global map name.
			 * The map will be treated as "Untitled" and you'll always have to manually select the file to save to.
			 * This is circumvented by calling the command on the editor window itself.
			 * 20001 is the id for the MRU1 command.
			 */
			GetMainWindow()->OnCommand(20001);
		}
	}

	// Set a custom icon for the main window

	HICON hIcon = LoadIconA(GModuleInstance, MAKEINTRESOURCEA(IDI_ICON1));

	if(hIcon)
	{
		SendMessageA(GetMainWindow()->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessageA(GetMainWindow()->hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}

	OriginalMainWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongA(GetMainWindow()->hWnd, GWLP_WNDPROC));
	SetWindowLongA(GetMainWindow()->hWnd, GWLP_WNDPROC, reinterpret_cast<LONG>(MainWindowProcOverride));

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC  ((USHORT)0x01)
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

	RAWINPUTDEVICE RawInput;
	RawInput.usUsagePage = HID_USAGE_PAGE_GENERIC;
	RawInput.usUsage     = HID_USAGE_GENERIC_MOUSE;
	RawInput.dwFlags     = 0;
	RawInput.hwndTarget  = GetMainWindow()->hWnd;

	RegisterRawInputDevices(&RawInput, 1, sizeof(RAWINPUTDEVICE));

	// Restore original tick function now that the initial setup is done.
	PatchVTable(Self, 32, OriginalUUnrealEdEngineTick);
}

// Init SWRCFix if it exists
static void InitSWRCFix()
{
	void* ModDLL = appGetDllHandle("Mod.dll");

	if(ModDLL)
	{
		void(CDECL*InitFunc)(void) = static_cast<void(CDECL*)(void)>(appGetDllExport(ModDLL, "InitSWRCFix"));

		if(InitFunc)
		{
			InitFunc();

			// Set render devices for the viewports

			for(INT i = 0; i < 8; ++i)
				GConfig->SetString(*FString::Printf("U2Viewport%i", i), "Device", "Mod.ModRenderDevice", "UnrealEd.ini");

			GConfig->SetString("Texture Browser",         "Device",       "Mod.ModRenderDevice", "UnrealEd.ini");
			GConfig->SetString("Static Mesh Browser",     "Device",       "Mod.ModRenderDevice", "UnrealEd.ini");
			GConfig->SetString("Particle System Browser", "Device",       "Mod.ModRenderDevice", "UnrealEd.ini");
			GConfig->SetString("Engine.Engine",           "RenderDevice", "Mod.ModRenderDevice", "System.ini");
		}
	}
}

/*
 * appInit hook.
 * This requires a modified executable that calls the ModEdInit function instead of appInit.
 */
DLL_EXPORT void ModEdInit(const TCHAR* InPackage, const TCHAR* InCmdLine, FOutputDevice*, FOutputDeviceError*, FFeedbackContext* InWarn, FConfigCache*(*)(), UBOOL RequireConfig)
{
	/*
	 * The output devices are dynamically allocated because otherwise weird stuff happens when an error occurs: For some reason the CRT calls the destructor of the file output device.
	 * However, it is then used again to by UObject::ExitProperties and reopened, deleting the file on disk and losing all previous output.
	 * No need to free the allocations since they live as long as the process does.
	 */
	FOutputDeviceError* Error = new FOutputDeviceWindowsError;
	FOutputDeviceFile*  Log   = new FOutputDeviceFile;
	Log->Unbuffered = 1; // Always unbuffered during appInit

	appInit(InPackage, InCmdLine, Log, Error, InWarn, FConfigCacheIniEditor::Factory, RequireConfig);

	if(!GConfig->GetBool("ModEd", "UnbufferedLog", Log->Unbuffered, "UnrealEd.ini"))
	{
		GConfig->SetBool("ModEd", "UnbufferedLog", 0, "UnrealEd.ini");
		Log->Unbuffered = 0;
	}

	InitSWRCFix();

	// Hook UUnrealEdEngine::Tick for initialization
	OriginalUUnrealEdEngineTick = static_cast<void(__fastcall*)(UEditorEngine*, DWORD, FLOAT)>(PatchDllClassVTable(*(FString(appPackage()) + ".exe"), "UUnrealEdEngine", "UObject", 32, UnrealEdEngineTickOverride));
}
