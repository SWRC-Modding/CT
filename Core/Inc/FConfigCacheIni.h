/*=============================================================================
	FConfigCacheIni.h: Unreal config file reading/writing.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Config cache.
-----------------------------------------------------------------------------*/

// Single config file.
class FConfigFile : public TMap<FString, FConfigSection>{
public:
	UBOOL Dirty, NoSave;

	FConfigFile() : Dirty(0),
					NoSave(0){}

	void Read(const TCHAR* Filename, FConfigFile* DefaultsOverride = NULL){
		guard(FConfigFile::Read);

		Empty();

		FString Text;

		if(appLoadFileToString(Text, Filename)){
			TCHAR* Ptr = const_cast<TCHAR*>(*Text);
			FConfigSection* CurrentSection = NULL;
			const FConfigSection* CurrentOverrideSection = NULL;
			bool Done = false;
			bool EmptySection = false;

			while(!Done){
				while(*Ptr == '\r' || *Ptr == '\n')
					Ptr++;

				TCHAR* Start = Ptr;

				while(*Ptr && *Ptr != '\r' && *Ptr != '\n')
					Ptr++;

				if(*Ptr == 0)
					Done = true;

				*Ptr++ = 0;

				// Skip comments
				if(*Start == ';')
					continue;

				if(Start[0] == '[' && Start[appStrlen(Start) - 1] == ']'){
					if(CurrentOverrideSection && EmptySection)
						*CurrentSection = *CurrentOverrideSection;

					Start++;
					Start[appStrlen(Start) - 1] = 0;
					CurrentSection = Find(Start);

					if(!CurrentSection)
						CurrentSection = &Set(Start, FConfigSection());

					if(DefaultsOverride)
						CurrentOverrideSection = DefaultsOverride->Find(Start);

					EmptySection = true;
				}else if(CurrentSection && *Start){
					TCHAR* Value = appStrstr(Start, "=");

					if(Value){
						bool IsArrayValue = *(Value - 1) == '+'; // RC uses '+=' for arrays

						if(IsArrayValue)
							*(Value - 1) = 0;

						*Value++ = 0;

						if(*Value == '\"' && Value[appStrlen(Value) - 1] == '\"'){
							Value++;
							Value[appStrlen(Value) - 1] = 0;
						}

						if(CurrentOverrideSection && !CurrentSection->Find(Start)){
							TArray<FString> OverrideValues;
							CurrentOverrideSection->MultiFind(Start, OverrideValues);

							if(OverrideValues.Num() > 1 || IsArrayValue){
								if(OverrideValues.FindItemIndex(Value) == INDEX_NONE) // Only add Value if it is not already contained in OverrideValues
									CurrentSection->Add(Start, Value);

								for(TArray<FString>::TIterator It(OverrideValues); It; ++It)
									CurrentSection->Add(Start, **It);
							}else{
								CurrentSection->Add(Start, OverrideValues.Num() == 1 ? *OverrideValues[0] : Value);
							}
						}else{
							CurrentSection->Add(Start, Value);
						}

						EmptySection = false;
					}
				}
			}

			// Inserting remaining sections from defaults override which do not exist in the file read from disk
			if(DefaultsOverride){
				for(TIterator It(*DefaultsOverride); It; ++It){
					if(Find(It.Key()))
						continue;

					Set(*It.Key(), It.Value());
				}
			}
		}else if(DefaultsOverride){
			*this = *DefaultsOverride;
		}

		unguard;
	}

	UBOOL Write(const TCHAR* Filename, const FConfigFile* Defaults = NULL){
		guard(FConfigFile::Write);

		if(!Dirty || NoSave)
			return 1;

		Dirty = 0;

		FString Text;
		char Temp[1024];

		for(TIterator SectionIt(*this); SectionIt; ++SectionIt){
			appSprintf(Temp, "[%s]\r\n", *SectionIt.Key());
			Text += Temp;

			// Array values are are all processed at once and added to this array.
			// When encountering a key that is contained in this array we can simply skip it.
			TArray<FName> AlreadyProcessed;
			const FConfigSection* DefaultSection = Defaults ? Defaults->Find(SectionIt.Key()) : NULL;

			for(FConfigSection::TIterator ValueIt(SectionIt.Value()); ValueIt; ++ValueIt){
				if(AlreadyProcessed.FindItemIndex(ValueIt.Key()) != INDEX_NONE)
					continue;

				TArray<FString> Values;
				TArray<FString> DefaultValues;

				if(DefaultSection)
					DefaultSection->MultiFind(ValueIt.Key(), DefaultValues);

				SectionIt.Value().MultiFind(ValueIt.Key(), Values);

				// Values that are the same as in the default config are commented out using ';'
				if(Values.Num() == 1 && DefaultValues.Num() <= 1){ // If there is only one single value '=' is used instead of "+="
					if(DefaultSection && DefaultValues.Num() == 1 && Values[0] == DefaultValues[0])
						appSprintf(Temp, ";  %s=%s\r\n", *ValueIt.Key(), *Values[0]);
					else
						appSprintf(Temp, "%s=%s\r\n", *ValueIt.Key(), *Values[0]);

					Text += Temp;
				}else{
					for(TArray<FString>::TIterator It(Values); It; ++It){
						int Index = DefaultValues.FindItemIndex(*It);

						if(Index == INDEX_NONE){
							appSprintf(Temp, "%s+=%s\r\n", *ValueIt.Key(), *It);
						}else{
							appSprintf(Temp, ";  %s+=%s\r\n", *ValueIt.Key(), *It);
							DefaultValues.Remove(Index);
						}

						Text += Temp;
					}

					// Writing remaining default values that have not yet been written
					for(TArray<FString>::TIterator It(DefaultValues); It; ++It){
						appSprintf(Temp, ";  %s+=%s\r\n", *ValueIt.Key(), *It);
						Text += Temp;
					}
				}

				AlreadyProcessed.AddUniqueItem(ValueIt.Key());
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
	FString SystemIni, UserIni;

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

			DefaultsOverride.Read(*((Filename == UserIni ? GCurrProfilePath : GGlobalSettingsPath) * Filename.GetCleanFilename()));

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

	UBOOL GetString(const TCHAR* Section, const TCHAR* Key, FString& Str, const TCHAR* Filename){
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

	TMultiMap<FName,FString>* GetSectionPrivate(const TCHAR* Section, UBOOL Force, UBOOL Const, const TCHAR* Filename){
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

		FString* Str = Sec->Find(Key);

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
				FString OutFilename = (It.Key() == UserIni ? GCurrProfilePath : GGlobalSettingsPath) * It.Key();
				FConfigFile DefaultConfig;

				DefaultConfig.Read(*It.Key());

				if(Section){ // Only the specified section is updated in the file on disk
					FConfigSection* Sec = GetSectionPrivate(Section, 0, 1, *It.Key());

					if(Sec){
						FConfigFile OverrideConfig;

						OverrideConfig.Read(*OutFilename);
						OverrideConfig.Set(Section, *Sec);
						OverrideConfig.Write(*OutFilename, &DefaultConfig);
					}
				}else{
					It.Value().Write(*OutFilename, &DefaultConfig);
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

		operator delete(this); // This is ok since the Object will not be referenced anymore after the call to exit

		if(GConfig == this)
			GConfig = NULL;

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
