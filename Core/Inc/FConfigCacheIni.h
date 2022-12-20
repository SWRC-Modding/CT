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
	bool Dirty;
	bool NoSave;
	bool Quotes;

	FConfigFile() : Dirty(false),
	                NoSave(ParseParam(appCmdLine(), "nosaveconfig") != 0),
	                Quotes(false){}

	void Read(const TCHAR* Filename, bool IsDefault){
		guard(FConfigFile::Read);

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

			if(Start[0] == '[' && Start[appStrlen(Start) - 1] == ']'){
				Start++;
				Start[appStrlen(Start) - 1] = '\0';
				CurrentSection = &(*this)[Start];
			}else if(CurrentSection && *Start){
				TCHAR* Value        = appStrstr(Start, "=");
				bool IsArrayElement = false;

				if(Value){
					IsArrayElement = Value > Start && Value[-1] == '+'; // RC uses '+=' for arrays

					if(IsArrayElement)
						Value[-1] = '\0';

					*Value++ = '\0';

					if(*Value == '\"' && Value[appStrlen(Value) - 1] == '\"'){
						Value++;
						Value[appStrlen(Value) - 1] = '\0';
					}
				}else{
					Value = "";
				}

				SetString(CurrentSection, Start, Value, IsArrayElement, IsDefault);
			}
		}

		unguard;
	}

	UBOOL Write(const TCHAR* Filename, const TCHAR* Section = NULL){
		guard(FConfigFile::Write);

		if(!Dirty || NoSave)
			return 1;

		Dirty = false;

		FString Text;

		for(TIterator SectionIt(*this); SectionIt; ++SectionIt){
			bool IsSpecifiedSection = (Section == NULL || SectionIt.Key() != Section);

			Text += FString::Printf("[%s]\r\n", *SectionIt.Key());

			for(FConfigSection::TIterator It(SectionIt.Value()); It; ++It){
				if(!It.Value().Dirty() && IsSpecifiedSection)
					Text += ";  ";

				Text += *It.Key();

				TArray<FConfigString> Values;

				SectionIt.Value().MultiFind(It.Key(), Values);

				if(Values.Num() > 1)
					Text += "+=";
				else
					Text += "=";

				if(Quotes){
					Text += "\"";
					Text += It.Value();
					Text += "\"\r\n";
				}else{
					Text += It.Value() + "\r\n";
				}
			}

			Text += "\r\n";
		}

		return appSaveStringToFile(Text, Filename);

		unguard;
	}

	void SetString(FConfigSection* Section, const TCHAR* Key, const TCHAR* Value, bool IsArrayElement = false, bool IsDefault = false){
		guard(FConfigFile::SetString);

		if(Key[0] == ';') // Skip comments
			return;

		FName          KeyName(Key);
		FConfigString* Str = Section->Find(KeyName);

		if(!Str || IsArrayElement){
			Str = &Section->Add(KeyName, Value);
		}else{
			if(appStricmp(**Str, Value) == 0)
				return;

			FLOAT F1 = appAtof(Value);
			FLOAT F2 = appAtof(**Str);

			if(F1 == F2 && F2 != 0.0f)
				return;

			*Str = Value;
		}

		Dirty = !IsDefault;
		Str->SetDirty(!IsDefault);

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

	FConfigFile* Find(const TCHAR* InFilename, bool CreateIfNotFound){
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
		FConfigFile* Result = TMap<FString, FConfigFile>::Find(*Filename.GetCleanFilename());

		if(!Result && (CreateIfNotFound || GFileManager->FileSize(*Filename) >= 0)){
			Result = &Set(*Filename.GetCleanFilename(), FConfigFile());
			Result->Read(*Filename, true);

			if(Filename.GetExtension() == "ini"){
				FString OverrideFilename = GetWritableFilePath(*Filename);

				// Check for override and read it if found.
				if(GFileManager->FileSize(*OverrideFilename) >= 0)
					Result->Read(*OverrideFilename, false);
			}
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

		const TCHAR* Text = GetStr(Section, Key, Filename);

		if(Text){
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

		const TCHAR* Text = GetStr(Section, Key, Filename);

		if(Text){
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

		const TCHAR* Text = GetStr(Section, Key, Filename);

		if(Text){
			Value = appAtof(Text);

			return 1;
		}

		return 0;

		unguard;
	}

	UBOOL GetFString(const TCHAR* Section, const TCHAR* Key, FString& Str, const TCHAR* Filename){
		guard(FConfigCacheIni::GetString);

		const TCHAR* Value = GetStr(Section, Key, Filename);

		if(!Value){
			Str = "";

			return 0;
		}

		Str = Value;

		return 1;

		unguard;
	}

	UBOOL GetString(const TCHAR* Section, const TCHAR* Key, TCHAR* Value, INT Size, const TCHAR* Filename){
		guard(FConfigCacheIni::GetString);

		const TCHAR* Str = GetStr(Section, Key, Filename);

		if(!Str){
			*Value = 0;

			return 0;
		}

		appStrncpy(Value, Str, Size);

		return 1;

		unguard;
	}

	const TCHAR* GetStr(const TCHAR* Section, const TCHAR* Key, const TCHAR* Filename){
		guard(FConfigCacheIni::GetStr);

		FConfigFile* File = Find(Filename, false);

		if(!File)
			return NULL;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec)
			return NULL;

		FConfigString* Str = Sec->Find(Key);

		if(!Str)
			return NULL;

		return **Str;

		unguard;
	}

	UBOOL GetSection(const TCHAR* Section, TCHAR* Result, INT Size, const TCHAR* Filename){
		guard(FConfigCacheIni::GetSection);

		*Result = 0;
		FConfigFile* File = Find(Filename, false);

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

		FConfigFile* File = Find(Filename, Force != 0);

		if(!File)
			return NULL;

		FConfigSection* Sec = File->Find(Section);

		if(!Sec && Force)
			Sec = &File->Set(Section, FConfigSection());

		if(Sec && (Force || !Const))
			File->Dirty = true;

		return Sec;

		unguard;
	}

	void EmptySection(const TCHAR* Section, const TCHAR* Filename){
		guard(FConfigCacheIni::EmptySection);

		FConfigFile* File = Find(Filename, false);

		if(File){
			FConfigSection* Sec = File->Find(Section);

			if(Sec && FConfigSection::TIterator(*Sec)){
				Sec->Empty();
				File->Dirty = true;
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
		appSprintf(Text, "%g", Value);
		SetString(Section, Key, Text, Filename);

		unguard;
	}

	void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value, const TCHAR* Filename){
		guard(FConfigCacheIni::SetString);

		FConfigFile* File = Find(Filename, true);
		FConfigSection* Sec = File->Find(Section);

		if(!Sec)
			Sec = &File->Set(Section, FConfigSection());

		File->SetString(Sec, Key, Value);

		unguard;
	}

	void Flush(UBOOL Read, const TCHAR* Filename = NULL, const TCHAR* Section = NULL){
		guard(FConfigCacheIni::Flush);

		for(TIterator It(*this); It; ++It){
			if(!Filename || It.Key() == Filename)
				It.Value().Write(*GetWritableFilePath(*It.Key()), Section);
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

		FConfigFile* File = Find(Filename, true);

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

		FConfigFile* File = Find(Filename, true);

		if(File)
			File->NoSave = true;

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
