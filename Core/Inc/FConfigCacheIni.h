/*=============================================================================
	FConfigCacheIni.h: Unreal config file reading/writing.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Config cache.
-----------------------------------------------------------------------------*/

//! @brief Single section in a config file.
typedef TMultiMap<FName, FString> FConfigSection;

//! @brief Single config file.
class FConfigFile : public TMap<FString, FConfigSection>{
public:
	UBOOL Dirty, NoSave;

	FConfigFile() : Dirty(0),
					NoSave(0){}

	void Read(const TCHAR* Filename){
		guard(FConfigFile::Read);

		Empty();

		FString Text;

		if(appLoadFileToString(Text, Filename)){
			TCHAR* Ptr = const_cast<TCHAR*>(*Text);
			FConfigSection* CurrentSection = NULL;

			bool Done = false;

			while(!Done){
				while(*Ptr == '\r' || *Ptr == '\n')
					Ptr++;

				TCHAR* Start = Ptr;

				while(*Ptr && *Ptr != '\r' && *Ptr != '\n')
					Ptr++;

				if(*Ptr == 0)
					Done = true;

				*Ptr++ = 0;

				// Ignore comments
				if(Start[0] == ';')
					continue;

				if(Start[0] == '[' && Start[appStrlen(Start) - 1] == ']'){
					Start++;
					Start[appStrlen(Start) - 1] = 0;
					CurrentSection = Find(Start);

					if(!CurrentSection)
						CurrentSection = &Set(Start, FConfigSection());
				}else if(CurrentSection && *Start){
					TCHAR* Value = appStrstr(Start, "=");

					if(Value){
						if(*(Value - 1) == '+')	// RC uses '+=' for arrays
							*(Value - 1) = 0;

						*Value++ = 0;

						if(*Value == '\"' && Value[appStrlen(Value) - 1] == '\"'){
							Value++;
							Value[appStrlen(Value) - 1] = 0;
						}

						CurrentSection->Add(Start, Value);
					}
				}
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

		for(TIterator It(*this); It; ++It){
			appSprintf(Temp, "[%s]\r\n", *It.Key());
			Text += Temp;

			TArray<FName> AlreadyProcessed;

			for(FConfigSection::TIterator It2(It.Value()); It2; ++It2){
				if(AlreadyProcessed.FindItemIndex(It2.Key()) != INDEX_NONE)
					continue;

				TArray<FString> ArrayValues;

				It.Value().MultiFind(It2.Key(), ArrayValues);

				if(ArrayValues.Num() == 1){
					appSprintf(Temp, "%s=%s\r\n", *It2.Key(), ArrayValues[0]);
					Text += Temp;
				}else{
					for(TArray<FString>::TIterator It3(ArrayValues); It3; ++It3){
						appSprintf(Temp, "%s+=%s\r\n", *It2.Key(), *It3);
						Text += Temp;
					}
				}

				AlreadyProcessed.AddUniqueItem(It2.Key());
			}

			Text += "\r\n";
		}

		return appSaveStringToFile(Text, Filename);

		unguard;
	}
};

//! @brief Set of all cached config files.
class FConfigCacheIni : public FConfigCache, public TMap<FString, FConfigFile>{
protected:
	FConfigCacheIni(){} //!< Private constructor. Use FConfigCacheIni::Factory instead

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
			Result = &Set(*Filename.GetCleanFilename(), FConfigFile());
			Result->Read(*Filename);
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

		FConfigFile* File = Find(Filename, 0);

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
		FConfigFile* File = Find(Filename, 0);

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

		TCHAR* Result = appStaticString1024();

		if(GetString(Section, Key, Result, 1024, Filename)){}
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

	void Flush(UBOOL Read, const TCHAR* Filename = NULL){
		guard(FConfigCacheIni::Flush);

		for(TIterator It(*this); It; ++It){
			if(!Filename || It.Key()==Filename)
				It.Value().Write(*It.Key());
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

	//! Static allocator.
	static FConfigCache* Factory(){
		return new FConfigCacheIni();
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
