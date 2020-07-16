/*=============================================================================
	FOutputDeviceFile.h: ANSI file output device.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
// ANSI file output device.
//
class FOutputDeviceFile : public FOutputDevice{
public:
	FArchive* LogAr;
	UBOOL     Timestamp;
	UBOOL     Opened;
	UBOOL     Dead;
	UBOOL     Unbuffered;
	UBOOL     CallLogHook;
	TCHAR     Filename[1024];

	FOutputDeviceFile(const TCHAR* InFilename = NULL) : LogAr(NULL),
	                                                    Timestamp(0),
	                                                    Opened(0),
	                                                    Unbuffered(0),
	                                                    CallLogHook(1),
	                                                    Dead(0){
		if(InFilename)
			SetFilename(InFilename);
		else
			Filename[0] = 0;
	}

	~FOutputDeviceFile(){
		Close();
	}

	void SetFilename(const TCHAR* InFilename){
		Close();
		appStrcpy(Filename, InFilename);
	}

	void Close(){
		if(LogAr){
			Logf(NAME_Log, "Log file closed, %s", appTimestamp(true, true));
			delete LogAr;
			LogAr = NULL;
			Opened = 0;
		}
	}

	void Serialize(const TCHAR* Data, enum EName Event){
		static UBOOL Entry = 0;

		if(!GIsCriticalError || Entry){
			if(!FName::SafeSuppressed(Event)){
				if(!LogAr && !Dead){
					// Make log filename.
					if(!Filename[0]){
						appStrcpy(Filename, appBaseDir());
						if
						(	!Parse(appCmdLine(), "LOG=", Filename+appStrlen(Filename), ARRAY_COUNT(Filename)-appStrlen(Filename))
						&&	!Parse(appCmdLine(), "ABSLOG=", Filename, ARRAY_COUNT(Filename)))
						{
							appStrcat(Filename, appPackage());
							appStrcat(Filename, ".log");
						}
					}

					// Open log file.
					LogAr = GFileManager->CreateFileWriter(Filename, FILEWRITE_AllowRead|FILEWRITE_Unbuffered|(Opened?FILEWRITE_Append:0));

					if(LogAr){
						Opened = 1;
						Logf(NAME_Log, "Log file open, %s", appTimestamp(true, true));
					}else{
						Dead = 1;
					}
				}

				if(LogAr && Event != NAME_Title){
					if(Timestamp){
						WriteRaw("[");
						WriteRaw(appTimestamp(true, true));
						WriteRaw("] ");
					}

					WriteRaw(FName::SafeString(Event));
					WriteRaw(": ");
					WriteRaw(Data);
					WriteRaw(LINE_TERMINATOR);

					if(Unbuffered)
						Flush();
				}

				if(CallLogHook && GLogHook && GLogHook != this)
					GLogHook->Serialize(Data, Event);
			}
		}else{
			Entry = 1;

			try{
				// Ignore errors to prevent infinite-recursive exception reporting.
				Serialize(Data, Event);
			}catch(...){}

			Entry = 0;
		}
	}

	virtual void Flush(){
		if(LogAr)
			LogAr->Flush();
	}

private:
	void WriteRaw(const TCHAR* C){
		LogAr->Serialize(const_cast<TCHAR*>(C), appStrlen(C)*sizeof(TCHAR));
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
