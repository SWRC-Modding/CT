/*=============================================================================
	UnEngineWin.h: Unreal engine windows-specific code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#pragma DISABLE_OPTIMIZATION /* Avoid VC++ code generation bug */

/*-----------------------------------------------------------------------------
	Splash screen.
-----------------------------------------------------------------------------*/

//
// Splash screen, implemented with old-style Windows code so that it
// can be opened super-fast before initialization.
//
BOOL CALLBACK SplashDialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg==WM_DESTROY )
		PostQuitMessage(0);
	return 0;
}
HWND    hWndSplash = NULL;
HBITMAP hBitmap    = NULL;
INT     BitmapX    = 0;
INT     BitmapY    = 0;
DWORD   ThreadId   = 0;
HANDLE  hThread    = 0;
DWORD WINAPI ThreadProc( VOID* Parm )
{
	hWndSplash = TCHAR_CALL_OS(CreateDialogW(hInstance,MAKEINTRESOURCEW(IDDIALOG_Splash), NULL, SplashDialogProc),CreateDialogA(hInstance, MAKEINTRESOURCEA(IDDIALOG_Splash), NULL, SplashDialogProc) );
	if( hWndSplash )
	{
		HWND hWndLogo = GetDlgItem(hWndSplash,IDC_Logo);
		if( hWndLogo )
		{
			SetWindowPos(hWndSplash,HWND_TOPMOST,(GetSystemMetrics(SM_CXSCREEN)-BitmapX)/2,(GetSystemMetrics(SM_CYSCREEN)-BitmapY)/2,BitmapX,BitmapY,SWP_SHOWWINDOW);
			SetWindowPos(hWndSplash,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			SendMessageX( hWndLogo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap );
			UpdateWindow( hWndSplash );
			MSG Msg;
			while( TCHAR_CALL_OS(GetMessageW(&Msg,NULL,0,0),GetMessageA(&Msg,NULL,0,0)) )
				DispatchMessageX(&Msg);
		}
	}
	return 0;
}
void InitSplash( const TCHAR* Filename )
{
	FWindowsBitmap Bitmap(1);
	if( Filename )
	{
		verify(Bitmap.LoadFile(Filename) );
		hBitmap = Bitmap.GetBitmapHandle();
		BitmapX = Bitmap.SizeX;
		BitmapY = Bitmap.SizeY;
	}
	hThread=CreateThread(NULL,0,&ThreadProc,NULL,0,&ThreadId);
}
void ExitSplash()
{
	if( ThreadId )
		TCHAR_CALL_OS(PostThreadMessageW(ThreadId,WM_QUIT,0,0),PostThreadMessageA(ThreadId,WM_QUIT,0,0));
}

/*-----------------------------------------------------------------------------
	System Directories.
-----------------------------------------------------------------------------*/

TCHAR SysDir[256]=TEXT(""), WinDir[256]=TEXT(""), ThisFile[256]=TEXT("");
void InitSysDirs()
{
#if UNICODE
	if( !GUnicodeOS )
	{
		ANSICHAR ASysDir[256]="", AWinDir[256]="", AThisFile[256]="";
		GetSystemDirectoryA( ASysDir, ARRAY_COUNT(ASysDir) );
		GetWindowsDirectoryA( AWinDir, ARRAY_COUNT(AWinDir) );
		GetModuleFileNameA( NULL, AThisFile, ARRAY_COUNT(AThisFile) );
		appStrcpy( SysDir, ANSI_TO_TCHAR(ASysDir) );
		appStrcpy( WinDir, ANSI_TO_TCHAR(AWinDir) );
		appStrcpy( ThisFile, ANSI_TO_TCHAR(AThisFile) );
	}
	else
#endif
	{
		GetSystemDirectory( SysDir, ARRAY_COUNT(SysDir) );
		GetWindowsDirectory( WinDir, ARRAY_COUNT(WinDir) );
		GetModuleFileName( NULL, ThisFile, ARRAY_COUNT(ThisFile) );
	}
	if( !appStricmp( &ThisFile[appStrlen(ThisFile) - 4], TEXT(".ICD") ) )
		appStrcpy( &ThisFile[appStrlen(ThisFile) - 4], TEXT(".EXE") );
}

/*-----------------------------------------------------------------------------
	Config wizard.
-----------------------------------------------------------------------------*/

class WConfigWizard : public WWizardDialog
{
	DECLARE_WINDOWCLASS(WConfigWizard,WWizardDialog,Startup)
	WLabel LogoStatic;
	FWindowsBitmap LogoBitmap;
	UBOOL Cancel;
	FString Title;
	WConfigWizard()
	: LogoStatic(this,IDC_Logo)
	, Cancel(0)
	{
		InitSysDirs();
	}
	void OnInitDialog()
	{
		guard(WStartupWizard::OnInitDialog);
		WWizardDialog::OnInitDialog();
		SendMessageX( *this, WM_SETICON, ICON_BIG, (WPARAM)LoadIconIdX(hInstance,IDICON_Mainframe) );
		LogoBitmap.LoadFile( TEXT("..\\Help\\Logo.bmp") );
		SendMessageX( LogoStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LogoBitmap.GetBitmapHandle() );
		SetText( *Title );
		SetForegroundWindow( hWnd );
		unguard;
	}
};

class WConfigPageFirstTime : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageFirstTime,WWizardPage,Startup)
	WConfigWizard* Owner;
	WConfigPageFirstTime( WConfigWizard* InOwner )
	: WWizardPage( TEXT("ConfigPageFirstTime"), IDDIALOG_ConfigPageFirstTime, InOwner )
	, Owner(InOwner)
	{}
	const TCHAR* GetNextText()
	{
		return LocalizeGeneral(TEXT("Run"),TEXT("Startup"));
	}
	WWizardPage* GetNext()
	{
		Owner->EndDialog(1);
		return NULL;
	}	
};

class WConfigPageSafeOptions : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageSafeOptions,WWizardPage,Startup)
	WConfigWizard* Owner;
	WButton NoSoundButton, No3DSoundButton, No3DVideoButton, WindowButton, ResButton, ResetConfigButton, NoProcessorButton, NoJoyButton;
	WConfigPageSafeOptions( WConfigWizard* InOwner )
	: WWizardPage		( TEXT("ConfigPageSafeOptions"), IDDIALOG_ConfigPageSafeOptions, InOwner )
	, Owner				(InOwner)
	, NoSoundButton		(this,IDC_NoSound)
	, No3DSoundButton	(this,IDC_No3DSound)
	, No3DVideoButton	(this,IDC_No3dVideo)
	, WindowButton		(this,IDC_Window)
	, ResButton			(this,IDC_Res)
	, ResetConfigButton	(this,IDC_ResetConfig)
	, NoProcessorButton	(this,IDC_NoProcessor)
	, NoJoyButton		(this,IDC_NoJoy)
	{}
	void OnInitDialog()
	{
		WWizardPage::OnInitDialog();
		SendMessageX( NoSoundButton,     BM_SETCHECK, 1, 0 );
		SendMessageX( No3DSoundButton,   BM_SETCHECK, 1, 0 );
		SendMessageX( No3DVideoButton,   BM_SETCHECK, 1, 0 );
		SendMessageX( WindowButton,      BM_SETCHECK, 1, 0 );
		SendMessageX( ResButton,         BM_SETCHECK, 1, 0 );
		SendMessageX( ResetConfigButton, BM_SETCHECK, 0, 0 );
		SendMessageX( NoProcessorButton, BM_SETCHECK, 1, 0 );
		SendMessageX( NoJoyButton,       BM_SETCHECK, 1, 0 );
	}
	const TCHAR* GetNextText()
	{
		return LocalizeGeneral(TEXT("Run"),TEXT("Startup"));
	}
	WWizardPage* GetNext()
	{
		FString CmdLine;
		if( SendMessageX(NoSoundButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -nosound");
		if( SendMessageX(No3DSoundButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -no3dsound");
		if( SendMessageX(No3DSoundButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -nohard");
		if( SendMessageX(No3DSoundButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -nohard -noddraw");
		if( SendMessageX(No3DSoundButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -defaultres");
		if( SendMessageX(NoProcessorButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -nommx -nokni -nok6");
		if( SendMessageX(NoJoyButton,BM_GETCHECK,0,0)==BST_CHECKED )
			CmdLine+=TEXT(" -nojoy");
		if( SendMessageX(ResetConfigButton,BM_GETCHECK,0,0)==BST_CHECKED )
			GFileManager->Delete( *(FString(appPackage())+TEXT(".ini")) );
		ShellExecuteX( NULL, TEXT("open"), ThisFile, *CmdLine, appBaseDir(), SW_SHOWNORMAL );
		Owner->EndDialog(0);
		return NULL;
	}
};

class WConfigPageDetail : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageDetail,WWizardPage,Startup)
	WConfigWizard* Owner;
	WEdit DetailEdit;
	WConfigPageDetail( WConfigWizard* InOwner )
	: WWizardPage( TEXT("ConfigPageDetail"), IDDIALOG_ConfigPageDetail, InOwner )
	, Owner(InOwner)
	, DetailEdit(this,IDC_DetailEdit)
	{}
	void OnInitDialog()
	{
		WWizardPage::OnInitDialog();
		FString Info;

		INT DescFlags=0;
		FString Driver = GConfig->GetStr(TEXT("Engine.Engine"),TEXT("GameRenderDevice"));
		GConfig->GetInt(*Driver,TEXT("DescFlags"),DescFlags);

		// Frame rate dependent LOD.
		if( Driver==TEXT("SoftDrv.SoftwareRenderDevice") || 280.0*1000.0*1000.0*GSecondsPerCycle>1.f )
		{
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("MinDesiredFrameRate"), TEXT("20") );
		}
		else if( Driver==TEXT("D3DDrv.D3DRenderDevice") )
		{
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("MinDesiredFrameRate"), TEXT("28") );
		}

		// Sound quality.
		if( !GIsMMX || GPhysicalMemory <= 32*1024*1024 )
		{
			Info = Info + LocalizeGeneral(TEXT("SoundLow"),TEXT("Startup")) + TEXT("\r\n");
			GConfig->SetString( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("UseReverb"),       TEXT("False") );
			GConfig->SetString( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("OutputRate"),      TEXT("11025Hz") );
			GConfig->SetString( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("UseSpatial"),      TEXT("False") );
			GConfig->SetString( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("UseFilter"),       TEXT("False") );
			GConfig->SetString( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("EffectsChannels"), TEXT("8") );
			GConfig->SetString( TEXT("Botpack.TournamentPlayer"),    TEXT("AnnouncerVolume"), TEXT("false") );
			GConfig->SetString( TEXT("Botpack.TournamentPlayer"),    TEXT("bNoVoiceTaunts"),  TEXT("true") );		
			GConfig->SetBool( TEXT("Galaxy.GalaxyAudioSubsystem"), TEXT("LowSoundQuality"), 1 );
		}
		else
		{
			Info = Info + LocalizeGeneral(TEXT("SoundHigh"),TEXT("Startup")) + TEXT("\r\n");
		}

		// Skins.
		if( (GPhysicalMemory < 96*1024*1024) || (DescFlags&RDDESCF_LowDetailSkins) )
		{
			Info = Info + LocalizeGeneral(TEXT("SkinsLow"),TEXT("Startup")) + TEXT("\r\n");
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("SkinDetail"), TEXT("Medium") );
		}
		else
		{
			Info = Info + LocalizeGeneral(TEXT("SkinsHigh"),TEXT("Startup")) + TEXT("\r\n");
		}

		// World.
		if( (GPhysicalMemory < 64*1024*1024) || (DescFlags&RDDESCF_LowDetailWorld) )
		{
			Info = Info + LocalizeGeneral(TEXT("WorldLow"),TEXT("Startup")) + TEXT("\r\n");
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("TextureDetail"), TEXT("Medium") );
		}
		else
		{
			Info = Info + LocalizeGeneral(TEXT("WorldHigh"),TEXT("Startup")) + TEXT("\r\n");
		}

		// Resolution.
		if( (!GIsMMX || !GIsPentiumPro) && Driver==TEXT("SoftDrv.SoftwareRenderDevice") )
		{
			Info = Info + LocalizeGeneral(TEXT("ResLow"),TEXT("Startup")) + TEXT("\r\n");
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedViewportX"),  TEXT("320") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedViewportY"),  TEXT("240") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedColorBits"),  TEXT("16") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenViewportX"), TEXT("320") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenViewportY"), TEXT("240") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenColorBits"), TEXT("16") );
		}
		else if( Driver==TEXT("SoftDrv.SoftwareRenderDevice") || (DescFlags&RDDESCF_LowDetailWorld) )
		{
			Info = Info + LocalizeGeneral(TEXT("ResLow"),TEXT("Startup")) + TEXT("\r\n");
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedViewportX"),  TEXT("512") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedViewportY"),  TEXT("384") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("WindowedColorBits"),  TEXT("16") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenViewportX"), TEXT("512") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenViewportY"), TEXT("384") );
			GConfig->SetString( TEXT("WinDrv.WindowsClient"), TEXT("FullscreenColorBits"), TEXT("16") );
		}
		else
		{
			Info = Info + LocalizeGeneral(TEXT("ResHigh"),TEXT("Startup")) + TEXT("\r\n");
		}
		DetailEdit.SetText(*Info);
	}
	WWizardPage* GetNext()
	{
		return new WConfigPageFirstTime(Owner);
	}
};

class WConfigPageDriver : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageDriver,WWizardPage,Startup)
	WConfigWizard* Owner;
	WUrlButton WebButton;
	WLabel Card;
	WConfigPageDriver( WConfigWizard* InOwner )
	: WWizardPage( TEXT("ConfigPageDriver"), IDDIALOG_ConfigPageDriver, InOwner )
	, Owner(InOwner)
	, WebButton(this,LocalizeGeneral(TEXT("Direct3DWebPage"),TEXT("Startup")),IDC_WebButton)
	, Card(this,IDC_Card)
	{}
	void OnInitDialog()
	{
		WWizardPage::OnInitDialog();
		FString CardName=GConfig->GetStr(TEXT("D3DDrv.D3DRenderDevice"),TEXT("Description"));
		if( CardName!=TEXT("") )
			Card.SetText(*CardName);
	}
	WWizardPage* GetNext()
	{
		return new WConfigPageDetail(Owner);
	}	
};

class WConfigPageRenderer : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageRenderer,WWizardPage,Startup)
	WConfigWizard* Owner;
	WListBox RenderList;
	WButton ShowCompatible, ShowAll;
	WLabel RenderNote;
	INT First;
	TArray<FRegistryObjectInfo> Classes;
	WConfigPageRenderer( WConfigWizard* InOwner )
	: WWizardPage( TEXT("ConfigPageRenderer"), IDDIALOG_ConfigPageRenderer, InOwner )
	, Owner(InOwner)
	, RenderList(this,IDC_RenderList)
	, ShowCompatible(this,IDC_Compatible,FDelegate(this,(TDelegate)RefreshList))
	, ShowAll(this,IDC_All,FDelegate(this,(TDelegate)RefreshList))
	, RenderNote(this,IDC_RenderNote)
	, First(0)
	{}
	void RefreshList()
	{
		RenderList.Empty();
		INT All=(SendMessageX(ShowAll,BM_GETCHECK,0,0)==BST_CHECKED), BestPriority=0;
		FString Default;
		Classes.Empty();
		UObject::GetRegistryObjects( Classes, UClass::StaticClass(), URenderDevice::StaticClass(), 0 );
		for( TArray<FRegistryObjectInfo>::TIterator It(Classes); It; ++It )
		{
			FString Path=It->Object, Left, Right, Temp;
			if( Path.Split(TEXT("."),&Left,&Right) )
			{
				INT DoShow=All, Priority=0;
				INT DescFlags=0;
				GConfig->GetInt(*Path,TEXT("DescFlags"),DescFlags);
				if
				(	It->Autodetect!=TEXT("")
				&& (GFileManager->FileSize(*FString::Printf(TEXT("%s\\%s"), SysDir, *It->Autodetect))>=0
				||  GFileManager->FileSize(*FString::Printf(TEXT("%s\\%s"), WinDir, *It->Autodetect))>=0) )
					DoShow = Priority = 3;
				else if( DescFlags & RDDESCF_Certified )
					DoShow = Priority = 2;
				else if( Path==TEXT("SoftDrv.SoftwareRenderDevice") )
					DoShow = Priority = 1;
				if( DoShow )
				{
					RenderList.AddString( *(Temp=Localize(*Right,TEXT("ClassCaption"),*Left)) );
					if( Priority>=BestPriority )
						{Default=Temp; BestPriority=Priority;}
				}
			}
		}
		if( Default!=TEXT("") )
			RenderList.SetCurrent(RenderList.FindStringChecked(*Default),1);
		CurrentChange();
	}
	void CurrentChange()
	{
		RenderNote.SetText(Localize(TEXT("Descriptions"),*CurrentDriver(),TEXT("Startup"),NULL,1));
	}
	void OnPaint()
	{
		if( !First++ )
		{
			UpdateWindow( *this );
			GConfig->Flush( 1 );
			if( !ParseParam(appCmdLine(),TEXT("nodetect")) )
			{
				GFileManager->Delete(TEXT("Detected.ini"));
				ShellExecuteX( NULL, TEXT("open"), ThisFile, TEXT("testrendev=D3DDrv.D3DRenderDevice log=Detected.log"), appBaseDir(), SW_SHOWNORMAL );
				for( INT MSec=30000; MSec>0 && GFileManager->FileSize(TEXT("Detected.ini"))<0; MSec-=100 )
					Sleep(100);
			}
			RefreshList();
		}
	}
	void OnCurrent()
	{
		guard(WFilerPageInstallProgress::OnCurrent);
		unguard;
	}
	void OnInitDialog()
	{
		WWizardPage::OnInitDialog();
		SendMessageX(ShowCompatible,BM_SETCHECK,BST_CHECKED,0);
		RenderList.SelectionChangeDelegate = FDelegate(this,(TDelegate)CurrentChange);
		RenderList.DoubleClickDelegate = FDelegate(Owner,(TDelegate)WWizardDialog::OnNext);
		RenderList.AddString( LocalizeGeneral(TEXT("Detecting"),TEXT("Startup")) );
	}
	FString CurrentDriver()
	{
		if( RenderList.GetCurrent()>=0 )
		{
			FString Name = RenderList.GetString(RenderList.GetCurrent());
			for( TArray<FRegistryObjectInfo>::TIterator It(Classes); It; ++It )
			{
				FString Path=It->Object, Left, Right, Temp;
				if( Path.Split(TEXT("."),&Left,&Right) )
					if( Name==Localize(*Right,TEXT("ClassCaption"),*Left) )
						return Path;
			}
		}
		return TEXT("");
	}
	WWizardPage* GetNext()
	{
		if( CurrentDriver()!=TEXT("") )
			GConfig->SetString(TEXT("Engine.Engine"),TEXT("GameRenderDevice"),*CurrentDriver());
		if( CurrentDriver()==TEXT("D3DDrv.D3DRenderDevice") )
			return new WConfigPageDriver(Owner);
		else
			return new WConfigPageDetail(Owner);
	}
};

class WConfigPageSafeMode : public WWizardPage
{
	DECLARE_WINDOWCLASS(WConfigPageSafeMode,WWizardPage,Startup)
	WConfigWizard* Owner;
	WCoolButton RunButton, VideoButton, SafeModeButton, WebButton;
	WConfigPageSafeMode( WConfigWizard* InOwner )
	: WWizardPage    ( TEXT("ConfigPageSafeMode"), IDDIALOG_ConfigPageSafeMode, InOwner )
	, RunButton      ( this, IDC_Run,      FDelegate(this,(TDelegate)OnRun) )
	, VideoButton    ( this, IDC_Video,    FDelegate(this,(TDelegate)OnVideo) )
	, SafeModeButton ( this, IDC_SafeMode, FDelegate(this,(TDelegate)OnSafeMode) )
	, WebButton      ( this, IDC_Web,      FDelegate(this,(TDelegate)OnWeb) )
	, Owner          (InOwner)
	{}
	void OnRun()
	{
		Owner->EndDialog(1);
	}
	void OnVideo()
	{
		Owner->Advance( new WConfigPageRenderer(Owner) );
	}
	void OnSafeMode()
	{
		Owner->Advance( new WConfigPageSafeOptions(Owner) );
	}
	void OnWeb()
	{
		ShellExecuteX( *this, TEXT("open"), LocalizeGeneral(TEXT("WebPage"),TEXT("Startup")), TEXT(""), appBaseDir(), SW_SHOWNORMAL );
		Owner->EndDialog(0);
	}
	const TCHAR* GetNextText()
	{
		return NULL;
	}
};

/*-----------------------------------------------------------------------------
	Launch mplayer.com.
	- by Jack Porter
	- Based on mp_launch2.c by Rich Rice --rich@mpath.com
-----------------------------------------------------------------------------*/

#define MPI_FILE TEXT("mput.mpi")
#define MPLAYNOW_EXE TEXT("mplaynow.exe")

static int GetMplayerDirectory(TCHAR *mplayer_directory)
{
	HKEY hkey;
	HKEY key = HKEY_LOCAL_MACHINE;
	TCHAR subkey[]=TEXT("software\\mpath\\mplayer\\main");
	TCHAR valuename[]=TEXT("root directory");
	TCHAR buffer[MAX_PATH];
	DWORD dwType, dwSize;
	
	if( RegOpenKeyExX(key, subkey, 0, KEY_READ, &hkey) == ERROR_SUCCESS )
	{
		dwSize = MAX_PATH;
		if( RegQueryValueExX(hkey, valuename, 0, &dwType, (LPBYTE) buffer, &dwSize) == ERROR_SUCCESS )
		{
			appSprintf(mplayer_directory, TEXT("%s"), buffer);
			return 1;
		}
		RegCloseKey(hkey);
	}

	return 0;
}

static void LaunchMplayer()
{
	TCHAR mplaunch_exe[MAX_PATH], mplayer_directory[MAX_PATH];

	if( GetMplayerDirectory(mplayer_directory) )
	{
		appSprintf( mplaunch_exe, TEXT("%s\\programs\\mplaunch.exe"), mplayer_directory );
		if( GFileManager->FileSize(mplaunch_exe)>0 )
		{
			appLaunchURL( mplaunch_exe, MPI_FILE );
			return;
		}
	}

	appLaunchURL( MPLAYNOW_EXE, TEXT("") );
}

#undef MPI_FILE
#undef MPLAYNOW_EXE

/*-----------------------------------------------------------------------------
	Exec hook.
-----------------------------------------------------------------------------*/

// FExecHook.
class FExecHook : public FExec, public FNotifyHook
{
private:
	WConfigProperties* Preferences;
	void NotifyDestroy( void* Src )
	{
		if( Src==Preferences )
			Preferences = NULL;
	}
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		guard(FExecHook::Exec);
		if( ParseCommand(&Cmd,TEXT("ShowLog")) )
		{
			if( GLogWindow )
			{
				GLogWindow->Show(1);
				SetFocus( *GLogWindow );
				GLogWindow->Display.ScrollCaret();
			}
			return 1;
		}
		else if( ParseCommand(&Cmd,TEXT("TakeFocus")) )
		{
			TObjectIterator<UEngine> EngineIt;
			if
			(	EngineIt
			&&	EngineIt->Client
			&&	EngineIt->Client->Viewports.Num() )
				SetForegroundWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
			return 1;
		}
		else if( ParseCommand(&Cmd,TEXT("EditActor")) )
		{
#if 1 //Fix added by Legend on 4/12/2000
			UClass* Class;
			FName ActorName;
			TObjectIterator<UEngine> EngineIt;

			AActor* Found = NULL;

			if( EngineIt && ParseObject<UClass>( Cmd, TEXT("Class="), Class, ANY_PACKAGE ) )
			{
				AActor* Player  = EngineIt->Client ? EngineIt->Client->Viewports(0)->Actor : NULL;
				FLOAT   MinDist = 999999.0;
				for( TObjectIterator<AActor> It; It; ++It )
				{
					FLOAT Dist = Player ? FDist(It->Location,Player->Location) : 0.0f;
					if
					(	(!Player || It->GetLevel()==Player->GetLevel())
					&&	(!It->bDeleteMe)
					&&	(It->IsA( Class) )
					&&	(Dist<MinDist) )
					{
						MinDist = Dist;
						Found   = *It;
					}
				}
			}
			else if( EngineIt && Parse( Cmd, TEXT("Name="), ActorName ) )
			{
				// look for actor by name
				for( TObjectIterator<AActor> It; It; ++It )
				{
					if( !It->bDeleteMe && It->GetName() == *ActorName )
					{
						Found = *It;
						break;
					}
				}
			}

			if( Found )
			{
				WObjectProperties* P = new WObjectProperties( TEXT("EditActor"), 0, TEXT(""), NULL, 1 );
				P->OpenWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
				P->Root.SetObjects( (UObject**)&Found, 1 );
				P->Show(1);
			}
			else Ar.Logf( TEXT("Bad or missing class or name") );
#else
			UClass* Class;
			TObjectIterator<UEngine> EngineIt;
			if( EngineIt && ParseObject<UClass>( Cmd, TEXT("Class="), Class, ANY_PACKAGE ) )
			{
				AActor* Player  = EngineIt->Client ? EngineIt->Client->Viewports(0)->Actor : NULL;
				AActor* Found   = NULL;
				FLOAT   MinDist = 999999.0;
				for( TObjectIterator<AActor> It; It; ++It )
				{
					FLOAT Dist = Player ? FDist(It->Location,Player->Location) : 0.0;
					if
					(	(!Player || It->GetLevel()==Player->GetLevel())
					&&	(!It->bDeleteMe)
					&&	(It->IsA( Class) )
					&&	(Dist<MinDist) )
					{
						MinDist = Dist;
						Found   = *It;
					}
				}
				if( Found )
				{
					WObjectProperties* P = new WObjectProperties( TEXT("EditActor"), 0, TEXT(""), NULL, 1 );
					P->OpenWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
					P->Root.SetObjects( (UObject**)&Found, 1 );
					P->Show(1);
				}
				else Ar.Logf( TEXT("Actor not found") );
			}
			else Ar.Logf( TEXT("Missing class") );
#endif
			return 1;
		}
		else if( ParseCommand(&Cmd,TEXT("HideLog")) )
		{
			if( GLogWindow )
				GLogWindow->Show(0);
			return 1;
		}
		else if( ParseCommand(&Cmd,TEXT("Preferences")) && !GIsClient )
		{
			if( !Preferences )
			{
				Preferences = new WConfigProperties( TEXT("Preferences"), LocalizeGeneral("AdvancedOptionsTitle",TEXT("Window")) );
				Preferences->SetNotifyHook( this );
				Preferences->OpenWindow( GLogWindow ? GLogWindow->hWnd : NULL );
				Preferences->ForceRefresh();
			}
			Preferences->Show(1);
			SetFocus( *Preferences );
			return 1;
		}
		else if( ParseCommand(&Cmd,TEXT("MPLAYER")) && GIsClient )
		{
			LaunchMplayer();			
			return 1;			
		}
		else if( ParseCommand(&Cmd,TEXT("HEAT")) && GIsClient )
		{
			appLaunchURL( TEXT("GotoHEAT.exe"), TEXT("5193") );
			return 1;			
		}
		else return 0;
		unguard;
	}
public:
	FExecHook()
	: Preferences( NULL )
	{}
};

/*-----------------------------------------------------------------------------
	Startup and shutdown.
-----------------------------------------------------------------------------*/

//
// Initialize.
//
#ifndef _EDITOR_
static UEngine* InitEngine()
{
	guard(InitEngine);
	FTime LoadTime = appSeconds();

	// Set exec hook.
	static FExecHook GLocalHook;
	GExec = &GLocalHook;

	// Create mutex so installer knows we're running.
	CreateMutexX( NULL, 0, TEXT("UnrealIsRunning"));
	UBOOL AlreadyRunning = (GetLastError()==ERROR_ALREADY_EXISTS);

	// First-run menu.
	INT FirstRun=0;
	GConfig->GetInt( TEXT("FirstRun"), TEXT("FirstRun"), FirstRun );
	if( ParseParam(appCmdLine(),TEXT("FirstRun")) )
		FirstRun=0;
	if( FirstRun<220 )
	{
		// Migrate savegames.
		TArray<FString> Saves = GFileManager->FindFiles( TEXT("..\\Save\\*.usa"), 1, 0 );
		for( TArray<FString>::TIterator It(Saves); It; ++It )
		{
			INT Pos = appAtoi(**It+4);
			FString Section = TEXT("UnrealShare.UnrealSlotMenu");
			FString Key     = FString::Printf(TEXT("SlotNames[%i]"),Pos);
			if( appStricmp(GConfig->GetStr(*Section,*Key,TEXT("user")),TEXT(""))==0 )
				GConfig->SetString(*Section,*Key,TEXT("Saved game"),TEXT("user"));
		}
	}

	// Commandline (for mplayer/heat)
	FString Command;
	if( Parse(appCmdLine(),TEXT("consolecommand="), Command) )
	{
		debugf(TEXT("Executing console command %s"),*Command);
		GExec->Exec( *Command, *GLog );
		return NULL;
	}

	// Test render device.
	FString Device;
	if( Parse(appCmdLine(),TEXT("testrendev="),Device) )
	{
		debugf(TEXT("Detecting %s"),*Device);
		try
		{
			UClass* Cls = LoadClass<URenderDevice>( NULL, *Device, NULL, 0, NULL );
			GConfig->SetInt(*Device,TEXT("DescFlags"),RDDESCF_Incompatible);
			GConfig->Flush(0);
			if( Cls )
			{
				URenderDevice* RenDev = ConstructObject<URenderDevice>(Cls);
				if( RenDev )
				{
					if( RenDev->Init(NULL,0,0,0,0) )
					{
						debugf(TEXT("Successfully detected %s"),*Device);
					}
					else delete RenDev;
				}
			}
		} catch( ... ) {}
		FArchive* Ar = GFileManager->CreateFileWriter(TEXT("Detected.ini"),0);
		if( Ar )
			delete Ar;
		return NULL;
	}

	// Config UI.
	guard(ConfigUI);
	if( !GIsEditor && GIsClient )
	{
		WConfigWizard D;
		WWizardPage* Page = NULL;
		if( ParseParam(appCmdLine(),TEXT("safe")) || appStrfind(appCmdLine(),TEXT("readini")) )
			{Page = new WConfigPageSafeMode(&D); D.Title=LocalizeGeneral(TEXT("SafeMode"),TEXT("Startup"));}
		else if( FirstRun<ENGINE_VERSION )
			{Page = new WConfigPageRenderer(&D); D.Title=LocalizeGeneral(TEXT("FirstTime"),TEXT("Startup"));}
		else if( ParseParam(appCmdLine(),TEXT("changevideo")) )
			{Page = new WConfigPageRenderer(&D); D.Title=LocalizeGeneral(TEXT("Video"),TEXT("Startup"));}
		else if( !AlreadyRunning && GFileManager->FileSize(TEXT("Running.ini"))>=0 )
			{Page = new WConfigPageSafeMode(&D); D.Title=LocalizeGeneral(TEXT("RecoveryMode"),TEXT("Startup"));}
		if( Page )
		{
			ExitSplash();
			D.Advance( Page );
			if( !D.DoModal() )
				return NULL;
			InitSplash(NULL);
		}
	}
	unguard;

	// Create is-running semaphore file.
	FArchive* Ar = GFileManager->CreateFileWriter(TEXT("Running.ini"),0);
	if( Ar )
		delete Ar;

	// Update first-run.
	if( FirstRun<ENGINE_VERSION )
		FirstRun = ENGINE_VERSION;
	GConfig->SetInt( TEXT("FirstRun"), TEXT("FirstRun"), FirstRun );

	// Cd check.
	FString CdPath;
	GConfig->GetString( TEXT("Engine.Engine"), TEXT("CdPath"), CdPath );
	if
	(	CdPath!=TEXT("")
	&&	GFileManager->FileSize(TEXT("..\\Textures\\Palettes.utx"))<=0 )//oldver
	{
		FString Check = CdPath * TEXT("Textures\\Palettes.utx");
		while( !GIsEditor && GFileManager->FileSize(*Check)<=0 )
		{
			if( MessageBox
			(
				NULL,
				LocalizeGeneral("InsertCdText",TEXT("Window")),
				LocalizeGeneral("InsertCdTitle",TEXT("Window")),
				MB_TASKMODAL|MB_OKCANCEL
			)==IDCANCEL )
			{
				GIsCriticalError = 1;
				ExitProcess( 0 );
			}
		}
	}

#if ENGINE_VERSION<230
	// Display the damn story to appease the German censors.
	UBOOL CanModifyGore=1;
	GConfig->GetBool( TEXT("UnrealI.UnrealGameOptionsMenu"), TEXT("bCanModifyGore"), CanModifyGore );
	if( !CanModifyGore && !GIsEditor )
	{
		FString S;
		if( appLoadFileToString( S, TEXT("Story.txt") ) )
		{
			WTextScrollerDialog Dlg( TEXT("The Story"), *S );
			Dlg.DoModal();
		}
	}
#endif

	// Create the global engine object.
	UClass* EngineClass;
	if( !GIsEditor )
	{
		// Create game engine.
		EngineClass = UObject::StaticLoadClass( UGameEngine::StaticClass(), NULL, TEXT("ini:Engine.Engine.GameEngine"), NULL, LOAD_NoFail, NULL );
	}
	else
	{
		// Editor.
		EngineClass = UObject::StaticLoadClass( UEngine::StaticClass(), NULL, TEXT("ini:Engine.Engine.EditorEngine"), NULL, LOAD_NoFail, NULL );
	}
	UEngine* Engine = ConstructObject<UEngine>( EngineClass );
	Engine->Init();
	debugf( TEXT("Startup time: %f seconds"), appSeconds()-LoadTime );

	return Engine;
	unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
static void MainLoop( UEngine* Engine )
{
	guard(MainLoop);
	check(Engine);

	// Enter main loop.
	guard(EnterMainLoop);
	if( GLogWindow )
		GLogWindow->SetExec( Engine );
	unguard;

	// Loop while running.
	GIsRunning = 1;
	DWORD ThreadId = GetCurrentThreadId();
	HANDLE hThread = GetCurrentThread();
	FTime OldTime = appSeconds();
	FTime SecondStartTime = OldTime;
	INT TickCount = 0;
	while( GIsRunning && !GIsRequestingExit )
	{
		// Update the world.
		guard(UpdateWorld);
		FTime NewTime   = appSeconds();
		FLOAT DeltaTime = NewTime - OldTime;
		Engine->Tick( DeltaTime );
		if( GWindowManager )
			GWindowManager->Tick( DeltaTime );
		OldTime = NewTime;
		TickCount++;
		if( OldTime > SecondStartTime + 1 )
		{
			Engine->CurrentTickRate = (FLOAT)TickCount / (OldTime - SecondStartTime);
			SecondStartTime = OldTime;
			TickCount = 0;
		}
		unguard;

		// Enforce optional maximum tick rate.
		guard(EnforceTickRate);
		FLOAT MaxTickRate = Engine->GetMaxTickRate();
		if( MaxTickRate>0.0 )
		{
			FLOAT Delta = (1.0f/MaxTickRate) - (appSeconds()-OldTime);
			appSleep( Max(0.f,Delta) );
		}
		unguard;

		// Handle all incoming messages.
		guard(MessagePump);
		MSG Msg;
		while( PeekMessageX(&Msg,NULL,0,0,PM_REMOVE) )
		{
			if( Msg.message == WM_QUIT )
				GIsRequestingExit = 1;

			guard(TranslateMessage);
			TranslateMessage( &Msg );
			unguardf(( TEXT("%08X %i"), (INT)Msg.hwnd, Msg.message ));

			guard(DispatchMessage);
			DispatchMessageX( &Msg );
			unguardf(( TEXT("%08X %i"), (INT)Msg.hwnd, Msg.message ));
		}
		unguard;

		// If editor thread doesn't have the focus, don't suck up too much CPU time.
		if( GIsEditor )
		{
			guard(ThrottleEditor);
			static UBOOL HadFocus=1;
			UBOOL HasFocus = (GetWindowThreadProcessId(GetForegroundWindow(),NULL) == ThreadId );
			if( HadFocus && !HasFocus )
			{
				// Drop our priority to speed up whatever is in the foreground.
				SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
			}
			else if( HasFocus && !HadFocus )
			{
				// Boost our priority back to normal.
				SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
			}
			if( !HasFocus )
			{
				// Surrender the rest of this timeslice.
				Sleep(0);
			}
			HadFocus = HasFocus;
			unguard;
		}
	}
	GIsRunning = 0;

	// Exit main loop.
	guard(ExitMainLoop);
	if( GLogWindow )
		GLogWindow->SetExec( NULL );
	GExec = NULL;
	unguard;

	unguard;
}
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
