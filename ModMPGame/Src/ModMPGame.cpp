#include "../Inc/ModMPGame.h"

static ABotSupport* GBotSupport = NULL;

/*
 * FBotSupportExecHook
 */
static struct FBotSupportExecHook : FExec{
	FExec* OldGExec;

	FBotSupportExecHook() : OldGExec(GExec){ GExec = this; }
	~FBotSupportExecHook(){ GExec = OldGExec; }

	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar){
		guard(FBotSupportExecHook::Exec);

		if(GBotSupport){
			if(ParseCommand(&Cmd, "IMPORTPATHS")){
				GBotSupport->ImportPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "EXPORTPATHS")){
				GBotSupport->ExportPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "BUILDPATHS")){
				GBotSupport->BuildPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "CLEARPATHS")){
				GBotSupport->ClearPaths();

				return 1;
			}else if(ParseCommand(&Cmd, "ADDBOT")){
				GBotSupport->AddBot();

				return 1;
			}else if(ParseCommand(&Cmd, "REMOVEBOT")){
				GBotSupport->RemoveBot();

				return 1;
			}

			if(GIsClient){ // Commands only available ingame
				UClass* PutNavPtClass = NULL;

				if(ParseCommand(&Cmd, "SHOWPATHS")){
					GBotSupport->bHidden = !GBotSupport->bHidden;

					return 1;
				}else if(ParseCommand(&Cmd, "REMOVENAVIGATIONPOINT")){
					APlayerController* Player = GetLocalPlayerController();
					check(Player);

					for(TActorIterator<ANavigationPoint> It(GBotSupport->XLevel); It; ++It){
						if(!It->IsA(APlayerStart::StaticClass()) &&
						   ((Player->Pawn ? Player->Pawn->Location : Player->Location) - It->Location).SizeSquared() <= 40 * 40){
							It->bStatic = 0;
							It->bNoDelete = 0;
							GBotSupport->XLevel->DestroyActor(*It);
							GBotSupport->BuildPaths();

							break;
						}
					}

					return 1;
				}else if(ParseCommand(&Cmd, "PUTPATHNODE")){
					PutNavPtClass = APathNode::StaticClass();
				}else if(ParseCommand(&Cmd, "PUTCOVERPOINT")){
					PutNavPtClass = ACoverPoint::StaticClass();
				}else if(ParseCommand(&Cmd, "PUTPATROLPOINT")){
					PutNavPtClass = APatrolPoint::StaticClass();
				}

				if(PutNavPtClass){
					APlayerController* Player = GetLocalPlayerController();
					check(Player);
					FVector Loc;
					FRotator Rot(0, 0, 0);

					if(Player->Pawn){
						Loc = Player->Pawn->Location;
						Rot.Yaw = Player->Pawn->Rotation.Yaw;
					}else{
						Loc = Player->Location;
						Rot.Yaw = Player->Rotation.Yaw;
					}

					GBotSupport->SpawnNavigationPoint(PutNavPtClass, Loc, Rot);

					return 1;
				}
			}
		}

		return OldGExec ? OldGExec->Exec(Cmd, Ar) : 0;

		unguard;
	}
} BotSupportExecHook;

APlayerController* GetLocalPlayerController(){
	TObjectIterator<UViewport> It;

	return It ? It->Actor : NULL;
}

static FFilename GetPathFileName(const FString MapName){
	return GFileManager->GetDefaultDirectory() * ".." * "Maps" * "Paths" * FFilename(MapName).GetBaseFilename() + ".ctp";
}

enum ENavPtType{
	NAVPT_PathNode,
	NAVPT_CoverPoint,
	NAVPT_PatrolPoint
};

/*
 * FNavPtInfo
 * Information about a navigation point that is written to a file
 */
struct FNavPtInfo{
	ENavPtType Type;
	FVector Location;
	FRotator Rotation;

	friend FArchive& operator<<(FArchive& Ar, FNavPtInfo& N){
		int Type = N.Type;

		Ar << Type << N.Location << N.Rotation;
		N.Type = static_cast<ENavPtType>(Type);

		return Ar;
	}
};

/*
 * ABotSupport::SpawnNavigationPoint
 * Spawns a navigation point at the specified position. Can be called during gameplay
 */
void ABotSupport::SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation){
	guard(ABotSupport::SpawnNavigationPoint);
	check(NavPtClass->IsChildOf(ANavigationPoint::StaticClass()));

	NavPtClass->GetDefaultActor()->bStatic = 0;
	NavPtClass->GetDefaultActor()->bNoDelete = 0;

	// For some reason the game crashes when two navigation points have the same name
	// SpawnActor should take care of this, but whatever...
	// This guarantees a unique name
	static int NavPtNum = 0;
	ANavigationPoint* NavPt = Cast<ANavigationPoint>(XLevel->SpawnActor(
		NavPtClass,
		FName(FString::Printf("CustomNavPt%i", NavPtNum++)),
		Location,
		Rotation
	));

	if(NavPt){
		NavPt->bStatic = 1;
		NavPt->bNoDelete = 1;
	}else{
		GLog->Logf(NAME_Error, "Failed to spawn %s", *NavPtClass->FriendlyName);
		NavPtFailLocations.AddItem(Location);
	}

	NavPtClass->GetDefaultActor()->bStatic = 1;
	NavPtClass->GetDefaultActor()->bNoDelete = 1;

	unguard;
}

void ABotSupport::ImportPaths(){
	guard(ABotSupport::ImportPaths);

	if(bPathsImported){
		GLog->Log(NAME_Error, "Paths have already been imported");

		return;
	}

	FString Filename = GetPathFileName(Level->Title);
	FArchive* Ar = GFileManager->CreateFileReader(*Filename);

	if(Ar){
		TArray<FNavPtInfo> NavPtInfo;

		*Ar << NavPtInfo;

		for(int i = 0; i < NavPtInfo.Num(); ++i){
			UClass* NavPtClass;

			if(NavPtInfo[i].Type == NAVPT_CoverPoint)
				NavPtClass = ACoverPoint::StaticClass();
			else if(NavPtInfo[i].Type == NAVPT_PatrolPoint)
				NavPtClass = APatrolPoint::StaticClass();
			else
				NavPtClass = APathNode::StaticClass();

			SpawnNavigationPoint(
				NavPtClass,
				NavPtInfo[i].Location,
				NavPtInfo[i].Rotation
			);
		}

		bPathsImported = 1;

		delete Ar;
	}else{
		GLog->Logf(NAME_Error, "Cannot import paths from file '%s'", *Filename);
	}

	unguard;
}


/*
 * ABotSupport::ExportPaths
 * Exports all navigation points from the current map to a file
 */
void ABotSupport::ExportPaths(){
	guard(ABotSupport::ExportPaths);

	TArray<FNavPtInfo> NavPts;
	ANavigationPoint* NavPt = Level->NavigationPointList;

	while(NavPt){
		FNavPtInfo NavPtInfo;

		if(!NavPt->IsA(APlayerStart::StaticClass())){ // Playerstarts are always part of the map and thus not exported
			ENavPtType NavPtType;

			if(NavPt->IsA(ACoverPoint::StaticClass()))
				NavPtType = NAVPT_CoverPoint;
			else if(NavPt->IsA(APatrolPoint::StaticClass()))
				NavPtType = NAVPT_PatrolPoint;
			else
				NavPtType = NAVPT_PathNode;

			NavPtInfo.Type = NavPtType;
			NavPtInfo.Location = NavPt->Location;
			NavPtInfo.Rotation = NavPt->Rotation;
			NavPts.AddItem(NavPtInfo);
		}

		NavPt = NavPt->nextNavigationPoint;
	}

	if(NavPts.Num() > 0){
		if(Level->Title.Len() > 0){
			FFilename Filename = GetPathFileName(Level->Title);
			GFileManager->MakeDirectory(*Filename.GetPath(), 1);
			FArchive* Ar = GFileManager->CreateFileWriter(*Filename);

			if(Ar){
				*Ar << NavPts;

				delete Ar;
			}else{
				GLog->Logf(NAME_Error, "Failed to open file '%s' for writing", *Filename);
			}
		}else{
			GLog->Log(NAME_Error, "LevelInfo.Title must not be empty");
		}
	}else{
		GLog->Log("Map does not contain any path nodes");
	}

	unguard;
}

/*
 * ABotSupport::BuildPaths
 * Does the same as the build paths option in the editor
 */
void ABotSupport::BuildPaths(){
	guard(ABotSupport::BuildPaths);

	UBOOL IsEd = GIsEditor;
	UBOOL BegunPlay = Level->bBegunPlay;

	Level->bBegunPlay = 0;
	GIsEditor = 1;

	GPathBuilder.definePaths(XLevel);

	GIsEditor = IsEd;
	Level->bBegunPlay = 1; // Actor script events are only called if Level->bBegunPlay == true which is not the case when paths are loaded at startup

	SetupPatrolRoute();

	Level->bBegunPlay = BegunPlay;

	unguard;
}

/*
 * ABotSupport::ClearPaths
 * Removes all existing paths but keeps navigation points intact
 */
void ABotSupport::ClearPaths(){
	guard(ABotSupport::ClearPaths);
	GPathBuilder.undefinePaths(XLevel);
	unguard;
}

void ABotSupport::Spawned(){
	GBotSupport = this;

	if(bAutoImportPaths){
		ImportPaths();

		if(bPathsImported) // We don't want the paths to be rebuilt if import failed
			BuildPaths();
	}
}

void ABotSupport::Destroy(){
	if(GBotSupport == this)
		GBotSupport = NULL;

	Super::Destroy();
}

UBOOL ABotSupport::Tick(FLOAT DeltaTime, ELevelTick TickType){
	guard(ABotSupport::Tick);

	/*
	 * Keeping the BotSupport Actor in the players view at all times so that it is always rendered
	 * which is needed when the ShowPaths command was used
	 */
	if(!bHidden){
		APlayerController* Player = GetLocalPlayerController();

		if(Player){
			FVector Loc;

			if(Player->Pawn){
				Loc = Player->Pawn->Location;
				Loc.Z += Player->Pawn->EyeHeight;
			}else{
				Loc = Player->Location;
			}

			XLevel->FarMoveActor(this, Loc + Player->Rotation.Vector());
		}
	}

	return Super::Tick(DeltaTime, TickType);

	unguard;
}

/*
 * ABotSupport::PostRender
 */
void ABotSupport::PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI){
	guard(ABotSupport::PostRender);
	Super::PostRender(SceneNode, RI);

	FLineBatcher LineBatcher(RI);
	APlayerController* Player = GetLocalPlayerController();
	check(Player);

	// Drawing the collision cylinder for each bot
	// Not terribly useful but can make it easier to see them when testing
	for(int i = 0; i < Bots.Num(); ++i){
		if(Bots[i]->Pawn){
			LineBatcher.DrawCylinder(
				Bots[i]->Pawn->Location,
				FVector(0, 0, 1),
				FColor(255, 0, 255),
				Bots[i]->Pawn->CollisionRadius,
				Bots[i]->Pawn->CollisionHeight,
				16
			);

			Bots[i]->DebugDraw(LineBatcher);
		}
	}

	FVector BoxSize(8, 8, 8);

	// Navigation points that failed to spawn are drawn as a red box for debugging purposes
	for(int i = 0; i < NavPtFailLocations.Num(); ++i)
		LineBatcher.DrawBox(FBox(NavPtFailLocations[i] - BoxSize, NavPtFailLocations[i] + BoxSize), FColor(255, 0, 0));

	// All navigation points in the level are drawn as a colored box
	for(TObjectIterator<ANavigationPoint> It; It; ++It){
		if(It->XLevel != XLevel || It->bDeleteMe)
			continue;

		FColor Color;

		if(It->IsA(APlayerStart::StaticClass()))
			Color = FColor(100, 100, 100);
		else if(It->IsA(ACoverPoint::StaticClass()))
			Color = FColor(0, 0, 255);
		else if(It->IsA(APatrolPoint::StaticClass()))
			Color = FColor(0, 255, 255);
		else
			Color = FColor(150, 100, 150);

		LineBatcher.DrawBox(FBox(It->Location - BoxSize, It->Location + BoxSize), Color);

		if(It->bDirectional)
			LineBatcher.DrawDirectionalArrow(It->Location, It->Rotation, FColor(255, 0, 0));
	}

	// Drawing connections between path nodes like in UnrealEd
	for(ANavigationPoint *Nav=Level->NavigationPointList; Nav; Nav=Nav->nextNavigationPoint){
		for(int i = 0; i < Nav->PathList.Num(); ++i){
			UReachSpec* ReachSpec = Nav->PathList[i];

			if(ReachSpec->Start && ReachSpec->End){
				LineBatcher.DrawLine(
					ReachSpec->Start->Location + FVector(0, 0, 8),
					ReachSpec->End->Location,
					ReachSpec->PathColor()
				);

				// make arrowhead to show L.D direction of path
				FVector Dir = ReachSpec->End->Location - ReachSpec->Start->Location - FVector(0, 0, 8);
				Dir.Normalize();

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir + FVector(0, 0, 3),
					ReachSpec->End->Location - 6 * Dir,
					ReachSpec->PathColor()
				);

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir - FVector(0, 0, 3),
					ReachSpec->End->Location - 6 * Dir,
					ReachSpec->PathColor()
				);

				LineBatcher.DrawLine(
					ReachSpec->End->Location - 12 * Dir + FVector(0, 0, 3),
					ReachSpec->End->Location - 12 * Dir - FVector(0, 0, 3),
					ReachSpec->PathColor()
				);
			}
		}
	}

	unguard;
}

void ABotSupport::execSpawnNavigationPoint(FFrame& Stack, void* Result){
	P_GET_OBJECT(UClass, NavPtClass);
	P_GET_VECTOR(Loc);
	P_GET_ROTATOR_OPTX(Rot, FRotator(0, 0, 0));
	P_FINISH;
	SpawnNavigationPoint(NavPtClass, Loc, Rot);
}

void ABotSupport::execBuildPaths(FFrame& Stack, void* Result){
	P_FINISH;
	BuildPaths();
}

void ABotSupport::execClearPaths(FFrame& Stack, void* Result){
	P_FINISH;
	ClearPaths();
}

/*
 * AMPBot::Tick
 */
int AMPBot::Tick(FLOAT DeltaTime, ELevelTick TickType){
	/*
	 * This is really stupid but for some reason the movement code
	 * only updates the Pawn's rotation in single player.
	 * The only solution is to pretend we're in SP while calling UpdateMovementAnimation.
	 */
	if(Level->NetMode != NM_Standalone &&
	   Pawn &&
	   !Pawn->bInterpolating &&
	   Pawn->bPhysicsAnimUpdate &&
	   Pawn->Mesh){
		BYTE Nm = Level->NetMode;

		Level->NetMode = NM_Standalone;
		Pawn->UpdateMovementAnimation(DeltaTime);
		Level->NetMode = Nm;
	}

	return Super::Tick(DeltaTime, TickType);
}

/*
 * UExportPathsCommandlet
 */
class UExportPathsCommandlet : public UCommandlet{
	DECLARE_CLASS(UExportPathsCommandlet, UCommandlet, 0, ModMPGame);

	void StaticConstructor(){
		LogToStdout = 0;
		IsServer = 1;
		IsClient = 1;
		IsEditor = 1;
		LazyLoad = 1;
		ShowErrorCount = 0;
		ShowBanner = 0;
	}

	virtual INT Main(const TCHAR* Parms){
		FString MapName;

		if(Parse(Parms, "map=", MapName)){
			UPackage* Package = UObject::LoadPackage(NULL, *MapName, LOAD_NoFail);
			ULevel* Level = NULL;

			for(TObjectIterator<ULevel> It; It; ++It){
				if(It->IsIn(Package)){
					Level = *It;

					break;
				}
			}

			if(Level){
				ABotSupport* BotSupport = Cast<ABotSupport>(Level->SpawnActor(ABotSupport::StaticClass()));

				if(BotSupport)
					BotSupport->ExportPaths();
				else
					GWarn->Log(NAME_Error, "Unable to export paths");
			}else{
				GWarn->Logf(NAME_Error, "Package '%s' is not a map", *MapName);
			}
		}else{
			GWarn->Log(NAME_Error, "Map to export paths from must be specified with 'map=<MapName>'");
		}

		return 0;
	}
};

IMPLEMENT_CLASS(UExportPathsCommandlet);
