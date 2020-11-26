/*=============================================================================
	FConfigCacheIni.h: Unreal config file reading/writing.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Config cache.
-----------------------------------------------------------------------------*/

/*
 * Config files work slightly different in SWRC than they do in other unreal games:
 * - Default ini files are stored in the System directory. Those are considered read only.
 * - When a config file is saved, it is written to the Save directory.
 * - Values from the config files in the Save directory override the default values from System.
 * - A saved config file has each value that is identical to the default commented out.
 * - In order to actually be used, the file must exist in 'System'. Only 'Save' is not enough.
 * - Array values are assigned using '+='. Using '=' will not add a new value but overwrite the most recently added item.
 */

// Single config file.
class FConfigFile : public TMap<FString, FConfigSection>{
public:
	UBOOL Dirty;
	UBOOL NoSave;

	FConfigFile() : Dirty(0),
                    NoSave(0){}

	void Read(const TCHAR* Filename, FConfigFile* DefaultsOverride = NULL){
		guard(FConfigFile::Read);

		Empty();

		FString Text;

		if(!appLoadFileToString(Text, Filename))
			return;

		TCHAR* Ptr = const_cast<TCHAR*>(*Text);
		FConfigSection* CurrentSection = NULL;
		bool Done = false;

		while(!Done){
			while(*Ptr == '\r' || *Ptr == '\n')
				Ptr++;

			TCHAR* Start = Ptr;

			while(*Ptr && *Ptr != '\r' && *Ptr != '\n')
				Ptr++;

			Done = *Ptr == '\0';
			*Ptr++ = '\0';

			// Skip comments
			if(*Start == ';')
				continue;

			if(Start[0] == '[' && Start[appStrlen(Start) - 1] == ']'){
				Start++;
				Start[appStrlen(Start) - 1] = '\0';
				CurrentSection = &(*this)[Start];
			}else if(CurrentSection && *Start){
				TCHAR* Value      = appStrstr(Start, "=");
				bool IsArrayValue = false;

				if(Value){
					IsArrayValue = Value[-1] == '+'; // RC uses '+=' for arrays

					if(IsArrayValue)
						Value[-1] = '\0';

					*Value++ = '\0';

					if(*Value == '\"' && Value[appStrlen(Value) - 1] == '\"'){
						Value++;
						Value[appStrlen(Value) - 1] = '\0';
					}
				}else{
					Value = "";
				}

				if(IsArrayValue){
					CurrentSection->Add(Start, FConfigString(Value, true, DefaultsOverride == NULL));
				}else{
					/*
					 * The following prevents a value from being tagged as modified if it is assigned more than once in the same file.
					 * e.g.
					 * Value=123
					 * Value=456
					 */
					FConfigString& NewValue = (*CurrentSection)[Start];

					NewValue = FConfigString(Value, false);
					NewValue.SetModified(DefaultsOverride == NULL);
				}
			}
		}

		if(!DefaultsOverride)
			return;

		// Insert values from override
		for(TIterator SectionIt(*DefaultsOverride); SectionIt; ++SectionIt){
			FConfigSection* Section = Find(SectionIt.Key());
			FConfigSection* OverrideSection = &SectionIt.Value();

			if(Section){
				TArray<FConfigString> Values(0, true);

				for(FConfigSection::TIterator It(*OverrideSection); It; ++It){
					OverrideSection->MultiFind(It.Key(), Values);

					for(INT i = 0; i < Values.Num(); ++i){
						if(Values[i].IsArrayValue())
							Section->AddUnique(It.Key(), Values[i]);
						else
							Section->Set(It.Key(), Values[i]);
					}

					Values.Empty();
				}
			}else{
				// Section doesn't exist in the default config so just take the entire one from the override
				Set(*SectionIt.Key(), SectionIt.Value());
			}
		}

		unguard;
	}

	UBOOL Write(const TCHAR* Filename){
		guard(FConfigFile::Write);

		if(!Dirty || NoSave)
			return 1;

		Dirty = 0;

		FString Text;
		char Temp[1024];

		for(TIterator SectionIt(*this); SectionIt; ++SectionIt){
			appSprintf(Temp, "[%s]\r\n", *SectionIt.Key());
			Text += Temp;

			for(FConfigSection::TIterator It(SectionIt.Value()); It; ++It){
				if(It.Value().IsArrayValue()){
					if(It.Value().WasModified())
						appSprintf(Temp, "%s+=%s\r\n", *It.Key(), *It.Value());
					else
						appSprintf(Temp, ";  %s+=%s\r\n", *It.Key(), *It.Value());
				}else{
					if(It.Value().WasModified())
						appSprintf(Temp, "%s=%s\r\n", *It.Key(), *It.Value());
					else
						appSprintf(Temp, ";  %s=%s\r\n", *It.Key(), *It.Value());
				}

				Text += Temp;
			}

			Text += "\r\n";
		}

		return appSaveStringToFile(Text, Filename);

		unguard;
	}
};

// Set of all cached config files.
class FConfigCacheIni : public FConfigCache, public TMap<FString, FConfigFile>{
protected:
	FConfigCacheIni(){} // Private constructor. Use FConfigCacheIni::Factory instead

public:
	// Basic functions.
	FString SystemIni;
	FString UserIni;

	FStringTemp GetWritableFilePath(const TCHAR* Filename){
		if(UserIni == Filename) // The User.ini is stored in the directory of the current profile if there is one
			return (GCurrProfilePath.Len() > 0 ? GCurrProfilePath : GGlobalSettingsPath) * FFilename(Filename).GetCleanFilename();

		return GGlobalSettingsPath * FFilename(Filename).GetCleanFilename();
	}

	FConfigFile* Find(const TCHAR* InFilename, UBOOL CreateIfNotFound){
		guard(FConfigCacheIni::Find);

		// If filename not specified, use default.
		FFilename Filename(InFilename ? InFilename : SystemIni);

		// Add .ini extension.
		if(Filename.GetExtension() == "")
			Filename += ".ini";

		// Automatically translate generic filenames.
		if(Filename == "User.ini")
			Filename = UserIni;
		else if(Filename == "System.ini")
			Filename = SystemIni;

		// Get file.
		FConfigFile* Result = TMap<FString, FConfigFile>::Find(Filename.GetCleanFilename());

		if(!Result && (CreateIfNotFound || GFileManager->FileSize(*Filename) >= 0)){
			FConfigFile DefaultsOverride;

			DefaultsOverride.Read(*GetWritableFilePath(*Filename));

			Result = &Set(*Filename.GetCleanFilename(), FConfigFile());
			Result->Read(*Filename, &DefaultsOverride);
		}

		return Result;

		unguard;
	}

	// Overrides

	UBOOL GetBool(
		const TCHAR*	Section,
		const TCHAR*	Key,
		UBOOL&			Value,
		const TCHAR*	Filename
	){
		guard(FConfigCacheIni::GetBool);

		TCHAR Text[80];
		if(GetString(Section, Key, Text, ARRAY_COUNT(Text), Filename)){
			if(appStricmp(Text, "True") == 0)
				Value = 1;
			else
				Value = appAtoi(Text) == 1;

			return 1;
		}
		return 0;

		unguard;
	}

	UBOOL GetInt(
		const TCHAR*	Section,
		const TCHAR*	Key,
		INT&			Value,
		const TCHAR*	Filename
	){
		guard(FConfigCacheIni::GetInt);

		TCHAR Text[80];
		if(GetString(Section, Key, Text, ARRAY_COUNT(Text), Filename)){
			Value = appAtoi(Text);

			return 1;
		}

		return 0;

		unguard;
	}

	UBOOL GetFloat(
		const TCHAR*	Section,
		const TCHAR*	Key,
		FLOAT&			Value,
		const TCHAR*	Filename
	){
		guard(FConfigCacheIni::GetFloat);

		TCHAR Text[80];
		if(GetString(Section, Key, Text, ARRAY_COUNT(Text), Filename)){
			Value = appAtof(Text);

			return 1;
		}

		return 0;

		unguard;
	}

	UBOOL GetFString(const TCHAR* Section, const TCHAR* Key, FString& Str, const TCHAR* Filename){
		guard(FConfigCacheIni::GetString);

		Str = "";

		FConfigFile* File = Find(Filename, 1);

		if(!File )
			return 0;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec)
			return 0;

		FString* PairString = Sec->Find(Key);

		if(!PairString)
			return 0;

		Str = **PairString;

		return 1;

		unguard;
	}

	UBOOL GetString(const TCHAR* Section, const TCHAR* Key, TCHAR* Value, INT Size, const TCHAR* Filename){
		guard(FConfigCacheIni::GetString);

		*Value = 0;
		FConfigFile* File = Find(Filename, 1);

		if(!File)
			return 0;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec)
			return 0;

		FString* PairString = Sec->Find(Key);

		if(!PairString)
			return 0;

		appStrncpy(Value, **PairString, Size);

		return 1;

		unguard;
	}

	const TCHAR* GetStr(const TCHAR* Section, const TCHAR* Key, const TCHAR* Filename){
		guard(FConfigCacheIni::GetStr);

		// Seems like LucasArts changed the behavior of appStaticString1024.
		// It looks like it somehow overflows when used repeatedly which was the case here.
		static char Buffer[1024];

		TCHAR* Result = Buffer;//appStaticString1024();

		if(GetString(Section, Key, Result, ARRAY_COUNT(Buffer), Filename)){}
			return Result;

		return NULL;

		unguard;
	}

	UBOOL GetSection(const TCHAR* Section, TCHAR* Result, INT Size, const TCHAR* Filename){
		guard(FConfigCacheIni::GetSection);

		*Result = 0;
		FConfigFile* File = Find(Filename, 0);

		if(!File)
			return 0;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec)
			return 0;

		TCHAR* End = Result;

		for(FConfigSection::TIterator It(*Sec); It && End-Result+appStrlen(*It.Key())+1<Size; ++It)
			End += appSprintf(End, "%s=%s", *It.Key(), *It.Value()) + 1;

		*End++ = 0;

		return 1;

		unguard;
	}

	FConfigSection* GetSectionPrivate(const TCHAR* Section, UBOOL Force, UBOOL Const, const TCHAR* Filename){
		guard(FConfigCacheIni::GetSectionPrivate);

		FConfigFile* File = Find(Filename, Force);

		if(!File)
			return NULL;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec && Force)
			Sec = &File->Set(Section, FConfigSection());

		if(Sec && (Force || !Const))
			File->Dirty = 1;

		return Sec;

		unguard;
	}

	void EmptySection(const TCHAR* Section, const TCHAR* Filename){
		guard(FConfigCacheIni::EmptySection);

		FConfigFile* File = Find(Filename, 0);

		if(File){
			FConfigSection* Sec = File->Find(Section);

			if(Sec && FConfigSection::TIterator(*Sec)){
				Sec->Empty();
				File->Dirty = 1;
			}
		}

		unguard;
	}

	void SetBool(
		const TCHAR* Section,
		const TCHAR* Key,
		UBOOL		 Value,
		const TCHAR* Filename
	){
		guard(FConfigCacheIni::SetBool);

		SetString(Section, Key, Value ? "True" : "False", Filename);

		unguard;
	}

	void SetInt(
		const TCHAR* Section,
		const TCHAR* Key,
		INT			 Value,
		const TCHAR* Filename
	){
		guard(FConfigCacheIni::SetInt);

		TCHAR Text[30];
		appSprintf(Text, "%i", Value);
		SetString(Section, Key, Text, Filename);

		unguard;
	}

	void SetFloat(
		const TCHAR*	Section,
		const TCHAR*	Key,
		FLOAT			Value,
		const TCHAR*	Filename
	){
		guard(FConfigCacheIni::SetFloat);

		TCHAR Text[30];
		appSprintf(Text, "%f", Value);
		SetString(Section, Key, Text, Filename);

		unguard;
	}

	void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value, const TCHAR* Filename){
		guard(FConfigCacheIni::SetString);

		FConfigFile* File = Find(Filename, 1);
		FConfigSection* Sec  = File->Find(Section);

		if(!Sec)
			Sec = &File->Set(Section, FConfigSection());

		FConfigString* Str = Sec->Find(Key);

		if(!Str){
			Sec->Add(Key, Value);
			File->Dirty = 1;
		}else if(appStricmp(**Str,Value)!=0){
			File->Dirty = (appStrcmp(**Str,Value)!=0);
			*Str = Value;
		}

		unguard;
	}

	void Flush(UBOOL Read, const TCHAR* Filename = NULL, const char* Section = NULL){
		guard(FConfigCacheIni::Flush);

		for(TIterator It(*this); It; ++It){
			if(It.Value().Dirty && (!Filename || It.Key() == Filename)){
				FString OutFilename =  GetWritableFilePath(*It.Key());

				if(Section){ // Only the specified section is updated in the file on disk
					FConfigSection* Sec = GetSectionPrivate(Section, 0, 1, *It.Key());

					if(Sec){
						// Read file from disk, update specified section only and write it back
						FConfigFile OverrideConfig;

						OverrideConfig.Read(*OutFilename);
						OverrideConfig.Set(Section, *Sec);
						OverrideConfig.Write(*OutFilename);
					}
				}else{
					// No section specified so just write the entire file
					It.Value().Write(*OutFilename);
				}
			}
		}

		if(Read){
			if(Filename)
				Remove(Filename);
			else
				Empty();
		}

		unguard;
	}

	void UnloadFile(const TCHAR* Filename){
		guard(FConfigCacheIni::UnloadFile);

		FConfigFile* File = Find(Filename, 1);

		if(File)
			Remove(Filename);

		unguard;
	}

	void UnloadInts(const TCHAR* Filename){
		guard(FConfigCacheIni::UnloadInts);

		TCHAR Buffer[1024];

		appSprintf(Buffer, "%s.%s", Filename, UObject::GetLanguage());
		UnloadFile(Buffer);

		unguard;
	}

	void Detach(const TCHAR* Filename){
		guard(FConfigCacheIni::Cancel);

		FConfigFile* File = Find(Filename, 1);

		if(File)
			File->NoSave = 1;

		unguard;
	}

	void Init(const TCHAR* InSystem, const TCHAR* InUser, UBOOL RequireConfig = 0){
		guard(FConfigCacheIni::Init);

		SystemIni = InSystem;
		UserIni   = InUser;

		unguard;
	}

	void Exit(){
		guard(FConfigCacheIni::Exit);
		Flush(1);
		unguard;
	}

	void Dump(FOutputDevice& Ar){
		guard(FConfigCacheIni::Dump);

		Ar.Log("Files map:");
		TMap<FString,FConfigFile>::Dump(Ar);

		unguard;
	}

	void Serialize(FArchive& Ar){
		Ar << *this;
	}

	//Static allocator.
	static FConfigCache* Factory(){
		return new FConfigCacheIni();
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
