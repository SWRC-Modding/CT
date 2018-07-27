/*=============================================================================
	UnGame.h: Unreal game class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unreal game engine.
-----------------------------------------------------------------------------*/

struct FLevelLoadingInfoEntry{
	FStringNoInit MapName;
	FStringNoInit LoadingMenuClass;
	FStringNoInit LoadingPic;
	FStringNoInit LoadingTitle;
	FStringNoInit LoadingText;
	BITFIELD LoadingShowHints:1;
};

struct FLoadingHint{
	FStringNoInit Title;
	FStringNoInit Text;
	INT PicIndex;
};

#define UCONST_NUM_PROGRESS_LEVELS 56

//
// The Unreal game engine.
//
class ENGINE_API UGameEngine : public UEngine{
	DECLARE_CLASS(UGameEngine,UEngine,CLASS_Config|CLASS_Transient,Engine)

	//Variables
	TArrayNoInit<FString> ServerActors;
	TArrayNoInit<FString> ServerPackages;
	class ULevel* GLevel;
	class ULevel* GEntry;
	class UPendingLevel* GPendingLevel;
	class APlayerController* LevelLoadingController;
	FLOAT LoadProgressFraction;
	FURL LastURL;
	TArrayNoInit<class UPackageCheckInfo*> PackageValidation;
	class UObject* MD5Package;
	BITFIELD bUseXInterface:1;
	FStringNoInit MainMenuClass;
	FStringNoInit InitialMenuClass;
	FStringNoInit ControllerLayoutMenuClass;
	FStringNoInit ConnectingMenuClass;
	FStringNoInit DisconnectMenuClass;
	FStringNoInit DisconnectMenuArgs;
	FStringNoInit LoadingClass;
	FStringNoInit PauseMenuClass;
	FStringNoInit LoadingMenuClass;
	FStringNoInit LoadingInfoMenuClass;
	FStringNoInit TeamMenuClass;
	FStringNoInit ProfileMenuClass;
	FStringNoInit MultiplayerPauseMenuClass;
	INT FPSAlarm;
	BITFIELD bMemoryAlarm:1;
	BITFIELD bAutoSaveStats:1;
	BITFIELD bCheatProtection:1;
	BITFIELD FramePresentPending:1;
	FLOAT TimeUntilExit;
	BITFIELD ExitDuringMovie:1;
	FLOAT ElapsedExitTime;
	FLOAT TimeUntilAutoPause;
	FLOAT ElapsedAutoPauseTime;
	FLOAT ElapsedInactiveTime;
	FLOAT InitialAttractTime;
	FLOAT TimeUntilAttractMode;
	FLOAT ElapsedInactiveAttractTime;
	FStringNoInit AttractModeMovie;
	BITFIELD bAttractDuringLevel:1;
	BITFIELD bAttractExitLevel:1;
	BITFIELD bLoopAttractModeMovie:1;
	FStringNoInit AttractModeMenuClass;
	BITFIELD bMovieInterruptable:1;
	BITFIELD bMoviePausedLevelMusic:1;
	BITFIELD bMoviePausedLevelSound:1;
	BITFIELD bMovieDisassociatePads:1;
	FStringNoInit ProfilePrefix;
	FStringNoInit DeathMatchStr;
	FStringNoInit TeamDeathMatchStr;
	FStringNoInit CaptureTheFlagStr;
	FStringNoInit AssaultStr;
	TArrayNoInit<FLevelLoadingInfoEntry> LevelLoadingInfo;
	TArrayNoInit<FLoadingHint> LoadingHints;
	FStringNoInit LocalizedKeyNames[255];
	FStringNoInit LocalizedXboxButtonNames[16];
	FStringNoInit ProgressLevels[56];
	FStringNoInit LevelMissionStartLevels[56];
	FStringNoInit DiscReadError;
	INT NumFriendRequests;
	INT NumGameInvites;
	INT LastNumFriendRequests;
	INT LastNumGameInvites;
	FLOAT FriendRequestTimeout;
	FLOAT GameInviteTimeout;
	FLOAT NextMatchmakingQueryTime;

	//Events
	FString GetLocalizedKeyName(BYTE K);
	void InitLevelLoadingInfo(){
		DECLARE_NAME(InitLevelLoadingInfo);
		UObject::ProcessEvent(NInitLevelLoadingInfo, NULL);
	}
	void PotentialSubtitledSoundPlayed(const FString& SoundName, FLOAT Duration, class AActor* A, INT Priority){
		DECLARE_NAME(PotentialSubtitledSoundPlayed);
		struct{
			FString SoundName;
			FLOAT Duration;
			class AActor* A;
			INT Priority;
		} Parms;
		Parms.SoundName=SoundName;
		Parms.Duration=Duration;
		Parms.A=A;
		Parms.Priority=Priority;
		UObject::ProcessEvent(NPotentialSubtitledSoundPlayed, &Parms);
	}
	INT GetLevelProgressIdx(const FString& Level){
		DECLARE_NAME(GetLevelProgressIdx);
		struct{
			FString Level;
			INT ReturnValue;
		} Parms;
		Parms.Level=Level;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetLevelProgressIdx, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL HasReachedLevel(const FString& Level, INT CurrentProgress){
		DECLARE_NAME(HasReachedLevel);
		struct{
			FString Level;
			INT CurrentProgress;
			UBOOL ReturnValue;
		} Parms;
		Parms.Level=Level;
		Parms.CurrentProgress=CurrentProgress;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NHasReachedLevel, &Parms);
		return Parms.ReturnValue;
	}
	FString GetNextLevel(INT afterThisIdx){
		DECLARE_NAME(GetNextLevel);
		struct{
			INT afterThisIdx;
			FString ReturnValue;
		} Parms;
		Parms.afterThisIdx=afterThisIdx;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetNextLevel, &Parms);
		return Parms.ReturnValue;
	}
	FString GetLevelMissionStart(const FString& Level){
		DECLARE_NAME(GetLevelMissionStart);
		struct{
			FString Level;
			FString ReturnValue;
		} Parms;
		Parms.Level=Level;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetLevelMissionStart, &Parms);
		return Parms.ReturnValue;
	}

	//Constructors
	UGameEngine();

	//Overrides
	virtual int Exec(const char*, FOutputDevice&);
	virtual void Destroy();
	virtual void Init();
	virtual void Tick(float);
	virtual bool CanDisplayError() const;
	virtual int ChallengeResponse(int);
	virtual void Click(class UViewport*, unsigned long, float, float);
	virtual void DisplayError(const char*);
	virtual void Draw(class UViewport*, int, unsigned char*, int*);
	virtual void FullscreenToggled(bool);
	virtual float GetMaxTickRate();
	virtual int InputEvent(class UViewport*, enum EInputKey, enum EInputAction, float);
	virtual int Key(class UViewport*, enum EInputKey, unsigned short);
	virtual void MouseDelta(class UViewport*, unsigned long, float, float);
	virtual void MousePosition(class UViewport*, unsigned long, float, float);
	virtual void MouseWheel(class UViewport*, unsigned long, int);
	virtual void PlayMovie(const char*, bool, bool, bool, bool, bool);
	virtual void Serialize(FArchive&);
	virtual void SetClientTravel(class UPlayer*, const char*, int, enum ETravelType);
	virtual void SetDemoParameters(float, float, int);
	virtual void SetProgress(const char*, const char*, float);
	virtual void UnClick(class UViewport*, unsigned long, int, int);

	//Virtual Functions
	virtual int Browse(const FURL& URL, const TMap<FString, FString>* TravelInfo, FString& Error);
	virtual void UnloadCurrentMap();
	virtual ULevel* LoadMap(const FURL& URL, UPendingLevel*, const TMap<FString, FString>* TravelInfo, FString& Error, bool);
	virtual void SaveGame(const char*);
	virtual void CancelPending();
	virtual void UpdateConnectingMessage();
	virtual void BuildServerMasterMap(UNetDriver* NetDriver, ULevel* InLevel);
	virtual void NotifyLevelChange();
	virtual void UpdateProgress();

	//Functions
	void AuthorizeClient(class ULevel*);
	int CheckForRogues();
	void FixUpLevel();
	const char* GetLanguageHack();
	FString InitSpecial(const char*);
	void LoadHints();
	const char* LocalizeHack(const char*, const char*, const char*, const char*, int);
	int PackageRevisionLevel();
	void PickRandomLoadingHint(FString&, FString&, int&);
	void ServerUpdateMD5();
	void UnloadHints();
	int ValidatePackage(const char*, const char*);
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/