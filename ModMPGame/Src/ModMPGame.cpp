#include "ModMPGamePrivate.h"

#include "../../Core/Inc/UnLinker.h"
#include "../../Gameplay/Inc/Gameplay.h"

struct FNavPtInfo{
	FVector Location;
	FRotator Rotation; // Shouldn't be needed but just in case...
	FString Name, Tag;

	friend FArchive& operator<<(FArchive& Ar, FNavPtInfo& N){
		return Ar << N.Location << N.Rotation << N.Name << N.Tag;
	}
};

struct FLevelNavPts{
	TMultiMap<FString, FNavPtInfo> NavPtsByClass;
	TArray<FString> NavPtClasses;

	friend FArchive& operator<<(FArchive& Ar, FLevelNavPts& N){
		INT NumClasses;

		if(Ar.IsLoading()){
			Ar << NumClasses;

			for(INT i = 0; i < NumClasses; ++i){
				FString ClassName;

				Ar << ClassName;

				N.NavPtClasses.AddUniqueItem(ClassName);
				INT NumNavPts;

				Ar << NumNavPts;

				for(INT j = 0; j < NumNavPts; ++j){
					FNavPtInfo NavPtInfo;

					Ar << NavPtInfo;

					N.NavPtsByClass.Add(*ClassName, NavPtInfo);
				}
			}
		}else{
			NumClasses = N.NavPtClasses.Num();

			Ar << NumClasses;

			for(INT i = 0; i < NumClasses; ++i){
				FString Class = N.NavPtClasses[i];
				TArray<FNavPtInfo> NavPts;

				N.NavPtsByClass.MultiFind(Class, NavPts);

				INT NumNavPts = NavPts.Num();

				Ar << Class << NumNavPts;

				for(INT j = 0; j < NumNavPts; ++j)
					Ar << NavPts[j];
			}
		}

		return Ar;
	}
};

static FString MakeNvPtFilePath(const FString& FileName){
	FString Path = FString(appBaseDir()) + "/ModMPGameData";

	GFileManager->MakeDirectory(*Path);

	return Path + "/" + FileName + ".nvp";
}

class UExtractPathsCommandlet : public UCommandlet{
	DECLARE_CLASS(UExtractPathsCommandlet,UCommandlet,0,ModMPGame);

	void StaticConstructor(){
		LogToStdout = 1;
		IsServer = 1;
		IsClient = 1;
		IsEditor = 1;
		LazyLoad = 1;
		ShowErrorCount = 0;
		ShowBanner = 0;
	}

	virtual INT Main(const TCHAR* Parms){
		FString MapName;

		if(!Parse(Parms, "Map=", MapName)){
			GLog->Log(NAME_Error, "Map not specified");

			return 1;
		}

		// Need to use LOAD_Verify to make sure LoadPackage returns NULL if not found (It will create a new empty package otherwise).
		// LOAD_Throw does absolutely nothing for whatever reason...
		UPackage* Map = UObject::LoadPackage(NULL, *MapName, LOAD_Verify);

		if(!Map)
			return 1;

		// Need to load again without LOAD_Verify
		Map = UObject::LoadPackage(NULL, *MapName, LOAD_NoFail);

		FLevelNavPts NavPts;

		for(TObjectIterator<ANavigationPoint> It; It; ++It){
			if(It->IsIn(Map) && !It->IsA(APlayerStart::StaticClass())){
				FString ClassName = It->GetClass()->GetPathName();
				FNavPtInfo NavPtInfo = {It->Location, It->Rotation, It->GetName(), *It->Tag};

				NavPts.NavPtsByClass.Add(*ClassName, NavPtInfo);
				NavPts.NavPtClasses.AddUniqueItem(ClassName);

				GLog->Logf("Exporting %s", It->GetName());
			}
		}

		if(NavPts.NavPtClasses.Num() > 0){
			FString FilePath = MakeNvPtFilePath(MapName);
			FArchive* Out = GFileManager->CreateFileWriter(*FilePath);

			if(!Out){
				GLog->Logf(NAME_Error, "Unable to open file '%s' for writing", *FilePath);

				return 1;
			}

			*Out << NavPts;

			Out->Close();

			delete Out;
		}

		return 0;
	}
};

IMPLEMENT_CLASS(UExtractPathsCommandlet);

void ABotSupport::execImportPaths(FFrame& Stack, void* Result){
	guard(ABotSupport::execImportPaths);

	P_GET_STR(MapName);
	P_FINISH;

	UBOOL ReturnValue = 0;
	UBOOL IsEditor = GIsEditor;
	UBOOL BegunPlay = Level->bBegunPlay;
	FString FilePath = MakeNvPtFilePath(MapName);
	FArchive* In = GFileManager->CreateFileReader(*FilePath);

	if(In){
		GIsEditor = 1;
		Level->bBegunPlay = 0;

		FLevelNavPts LevelNavPts;

		*In << LevelNavPts;

		for(int i = 0; i < LevelNavPts.NavPtClasses.Num(); ++i){
			FString ClassName = LevelNavPts.NavPtClasses[i];
			UClass* Class = LoadClass<ANavigationPoint>(NULL, *ClassName, NULL, LOAD_NoFail, NULL);
			TArray<FNavPtInfo> NavPts;

			LevelNavPts.NavPtsByClass.MultiFind(ClassName, NavPts);

			for(int j = 0; j < NavPts.Num(); ++j){
				GLog->Logf("Importing %s", *NavPts[j].Name);

				AActor* NavPt = XLevel->SpawnActor(Class, FName(NavPts[j].Name), NavPts[j].Location, NavPts[j].Rotation);

				if(NavPt)
					{NavPt->Tag = FName(NavPts[j].Tag); NavPt->bHidden = 0;}
				else
					GLog->Logf(NAME_Warning, "Failed to spawn %s", *NavPts[j].Name);
			}
		}

		XLevel->Modify();
		GPathBuilder.definePaths(XLevel);

		Level->bBegunPlay = BegunPlay;
		GIsEditor = IsEditor;
	}else{
		GLog->Logf(NAME_Error, "Unable to open file '%s' for reading", *FilePath);
	}

	*static_cast<UBOOL*>(Result) = ReturnValue;

	unguard;
}