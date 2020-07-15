	//Overrides
	virtual void CheckHearSound(class AActor*, class USound*, int);
	virtual int* GetOptimizedRepList(unsigned char*, struct FPropertyRetirement*, int*, class UPackageMap*, class UActorChannel*);
	virtual class AActor* GetViewTarget();
	virtual int IsAPlayerController();
	virtual int IsNetRelevantFor(class APlayerController*, class AActor*, const FVector&);
	virtual int LocalPlayerController();
	virtual void PostRender(class FSceneNode*);
	virtual void PostScriptDestroyed();
	virtual void SetAnchor(class ANavigationPoint*);
	virtual void Spawned();
	virtual int StopAtLedge();
	virtual int Tick(float, enum ELevelTick);
	virtual int WantsLedgeCheck();

	//Events
	void NoHelmetInit(){
		DECLARE_NAME(NoHelmetInit);
		ProcessEvent(NNoHelmetInit, NULL);
	}
	void X_ClientValidate(const FString& C);
	void X_ServerValidationResponse(const FString& R);
	void ClientValidate(const FString& C){
		DECLARE_NAME(ClientValidate);
		struct{
			FString C;
		} Parms;
		Parms.C=C;
		ProcessEvent(NClientValidate, &Parms);
	}
	void ServerValidationResponse(const FString& R){
		DECLARE_NAME(ServerValidationResponse);
		struct{
			FString R;
		} Parms;
		Parms.R=R;
		ProcessEvent(NServerValidationResponse, &Parms);
	}
	void X_ClientHearSound(class AActor* Actor, class USound* S, const FVector& SoundLocation, INT Flags);
	void ClientHearSound(class AActor* Actor, class USound* S, const FVector& SoundLocation, INT Flags){
		DECLARE_NAME(ClientHearSound);
		struct{
			class AActor* Actor;
			class USound* S;
			FVector SoundLocation;
			INT Flags;
		} Parms;
		Parms.Actor=Actor;
		Parms.S=S;
		Parms.SoundLocation=SoundLocation;
		Parms.Flags=Flags;
		ProcessEvent(NClientHearSound, &Parms);
	}
	void KickWarning(){
		DECLARE_NAME(KickWarning);
		ProcessEvent(NKickWarning, NULL);
	}
	void ClientSetViewTarget(class AActor* A){
		DECLARE_NAME(ClientSetViewTarget);
		struct{
			class AActor* A;
		} Parms;
		Parms.A=A;
		ProcessEvent(NClientSetViewTarget, &Parms);
	}
	void InitInputSystem(){
		DECLARE_NAME(InitInputSystem);
		ProcessEvent(NInitInputSystem, NULL);
	}
	void ViewFlash(FLOAT DeltaTime){
		DECLARE_NAME(ViewFlash);
		struct{
			FLOAT DeltaTime;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		ProcessEvent(NViewFlash, &Parms);
	}
	void ReceiveLocalizedMessage(class UClass* Message, INT Switch, class APlayerReplicationInfo* RelatedPRI_1, class APlayerReplicationInfo* RelatedPRI_2, class UObject* OptionalObject){
		DECLARE_NAME(ReceiveLocalizedMessage);
		struct{
			class UClass* Message;
			INT Switch;
			class APlayerReplicationInfo* RelatedPRI_1;
			class APlayerReplicationInfo* RelatedPRI_2;
			class UObject* OptionalObject;
		} Parms;
		Parms.Message=Message;
		Parms.Switch=Switch;
		Parms.RelatedPRI_1=RelatedPRI_1;
		Parms.RelatedPRI_2=RelatedPRI_2;
		Parms.OptionalObject=OptionalObject;
		ProcessEvent(NReceiveLocalizedMessage, &Parms);
	}
	void ClientMessage(const FString& S, FName Type){
		DECLARE_NAME(ClientMessage);
		struct{
			FString S;
			FName Type;
		} Parms;
		Parms.S=S;
		Parms.Type=Type;
		ProcessEvent(NClientMessage, &Parms);
	}
	void TeamMessage(class APlayerReplicationInfo* PRI, const FString& S, FName Type){
		DECLARE_NAME(TeamMessage);
		struct{
			class APlayerReplicationInfo* PRI;
			FString S;
			FName Type;
		} Parms;
		Parms.PRI=PRI;
		Parms.S=S;
		Parms.Type=Type;
		ProcessEvent(NTeamMessage, &Parms);
	}
	void PreClientTravel(){
		DECLARE_NAME(PreClientTravel);
		ProcessEvent(NPreClientTravel, NULL);
	}
	void SendClientAdjustment(){
		DECLARE_NAME(SendClientAdjustment);
		ProcessEvent(NSendClientAdjustment, NULL);
	}
	void ViewShake(FLOAT DeltaTime){
		DECLARE_NAME(ViewShake);
		struct{
			FLOAT DeltaTime;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		ProcessEvent(NViewShake, &Parms);
	}
	void LoadMostRecent(){
		DECLARE_NAME(LoadMostRecent);
		ProcessEvent(NLoadMostRecent, NULL);
	}
	void ShowMenu(const FString& MenuClass, const FString& Title, const FString& Text, const FString& PicName, const FString& NewLevel){
		DECLARE_NAME(ShowMenu);
		struct{
			FString MenuClass;
			FString Title;
			FString Text;
			FString PicName;
			FString NewLevel;
		} Parms;
		Parms.MenuClass=MenuClass;
		Parms.Title=Title;
		Parms.Text=Text;
		Parms.PicName=PicName;
		Parms.NewLevel=NewLevel;
		ProcessEvent(NShowMenu, &Parms);
	}
	void UpdateName(const FString& newName){
		DECLARE_NAME(UpdateName);
		struct{
			FString newName;
		} Parms;
		Parms.newName=newName;
		ProcessEvent(NUpdateName, &Parms);
	}
	void SetProgressMessage(INT Index, const FString& S, const FColor& C){
		DECLARE_NAME(SetProgressMessage);
		struct{
			INT Index;
			FString S;
			FColor C;
		} Parms;
		Parms.Index=Index;
		Parms.S=S;
		Parms.C=C;
		ProcessEvent(NSetProgressMessage, &Parms);
	}
	void SetProgressTime(FLOAT T){
		DECLARE_NAME(SetProgressTime);
		struct{
			FLOAT T;
		} Parms;
		Parms.T=T;
		ProcessEvent(NSetProgressTime, &Parms);
	}
	void PlayerTick(FLOAT DeltaTime){
		FName NPlayerTick(NAME_PlayerTick);
		if(!IsProbing(NPlayerTick))
			return;
		struct{
			FLOAT DeltaTime;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		ProcessEvent(NPlayerTick, &Parms);
	}
	void AddCameraEffect(class UCameraEffect* NewEffect, UBOOL RemoveExisting){
		DECLARE_NAME(AddCameraEffect);
		struct{
			class UCameraEffect* NewEffect;
			UBOOL RemoveExisting;
		} Parms;
		Parms.NewEffect=NewEffect;
		Parms.RemoveExisting=RemoveExisting;
		ProcessEvent(NAddCameraEffect, &Parms);
	}
	void RemoveCameraEffect(class UCameraEffect* ExEffect){
		DECLARE_NAME(RemoveCameraEffect);
		struct{
			class UCameraEffect* ExEffect;
		} Parms;
		Parms.ExEffect=ExEffect;
		ProcessEvent(NRemoveCameraEffect, &Parms);
	}
	void PlayerCalcView(class AActor*& ViewActor, FVector& CameraLocation, FRotator& CameraRotation){
		DECLARE_NAME(PlayerCalcView);
		struct{
			class AActor* ViewActor;
			FVector CameraLocation;
			FRotator CameraRotation;
		} Parms;
		Parms.ViewActor=ViewActor;
		Parms.CameraLocation=CameraLocation;
		Parms.CameraRotation=CameraRotation;
		ProcessEvent(NPlayerCalcView, &Parms);
		ViewActor=Parms.ViewActor;
		CameraLocation=Parms.CameraLocation;
		CameraRotation=Parms.CameraRotation;
	}
	void UpdateRotation(FLOAT DeltaTime, FLOAT MaxPitch);
	void ClientSetBadCDKey(UBOOL bSet){
		DECLARE_NAME(ClientSetBadCDKey);
		struct{
			UBOOL bSet;
		} Parms;
		Parms.bSet=bSet;
		ProcessEvent(NClientSetBadCDKey, &Parms);
	}
	void ClientSetMissingContent(UBOOL bSet, const FString& ContentName){
		DECLARE_NAME(ClientSetMissingContent);
		struct{
			UBOOL bSet;
			FString ContentName;
		} Parms;
		Parms.bSet=bSet;
		Parms.ContentName=ContentName;
		ProcessEvent(NClientSetMissingContent, &Parms);
	}
	void ClientOpenMenu(const FString& Menu, UBOOL bDisconnect, const FString& Msg1, const FString& Msg2){
		DECLARE_NAME(ClientOpenMenu);
		struct{
			FString Menu;
			UBOOL bDisconnect;
			FString Msg1;
			FString Msg2;
		} Parms;
		Parms.Menu=Menu;
		Parms.bDisconnect=bDisconnect;
		Parms.Msg1=Msg1;
		Parms.Msg2=Msg2;
		ProcessEvent(NClientOpenMenu, &Parms);
	}
	void ClientOpenXMenu(const FString& Menu, UBOOL bDisconnect, const FString& Msg1, const FString& Msg2){
		DECLARE_NAME(ClientOpenXMenu);
		struct{
			FString Menu;
			UBOOL bDisconnect;
			FString Msg1;
			FString Msg2;
		} Parms;
		Parms.Menu=Menu;
		Parms.bDisconnect=bDisconnect;
		Parms.Msg1=Msg1;
		Parms.Msg2=Msg2;
		ProcessEvent(NClientOpenXMenu, &Parms);
	}
	void ClientShowSingularMenu(const FString& Menu, const FString& Args){
		DECLARE_NAME(ClientShowSingularMenu);
		struct{
			FString Menu;
			FString Args;
		} Parms;
		Parms.Menu=Menu;
		Parms.Args=Args;
		ProcessEvent(NClientShowSingularMenu, &Parms);
	}
	void ClientCloseMenu(UBOOL bCloseAll, UBOOL bCancel){
		DECLARE_NAME(ClientCloseMenu);
		struct{
			UBOOL bCloseAll;
			UBOOL bCancel;
		} Parms;
		Parms.bCloseAll=bCloseAll;
		Parms.bCancel=bCancel;
		ProcessEvent(NClientCloseMenu, &Parms);
	}
	void ClientCloseXMenu(UBOOL bCloseAll, UBOOL bCancel){
		DECLARE_NAME(ClientCloseXMenu);
		struct{
			UBOOL bCloseAll;
			UBOOL bCancel;
		} Parms;
		Parms.bCloseAll=bCloseAll;
		Parms.bCancel=bCancel;
		ProcessEvent(NClientCloseXMenu, &Parms);
	}
	void ServerChangeVoiceChatter(class APlayerController* Player, FXboxAddr const& xbAddr, INT Handle, INT VoiceChannels, UBOOL Add){
		DECLARE_NAME(ServerChangeVoiceChatter);
		struct{
			class APlayerController* Player;
			FXboxAddr xbAddr;
			INT Handle;
			INT VoiceChannels;
			UBOOL Add;
		} Parms;
		Parms.Player=Player;
		Parms.xbAddr=xbAddr;
		Parms.Handle=Handle;
		Parms.VoiceChannels=VoiceChannels;
		Parms.Add=Add;
		ProcessEvent(NServerChangeVoiceChatter, &Parms);
	}
	void ServerChangeHasVoice(UBOOL bVoiceOn){
		DECLARE_NAME(ServerChangeHasVoice);
		struct{
			UBOOL bVoiceOn;
		} Parms;
		Parms.bVoiceOn=bVoiceOn;
		ProcessEvent(NServerChangeHasVoice, &Parms);
	}
	void ServerGetVoiceChatters(class APlayerController* Player){
		DECLARE_NAME(ServerGetVoiceChatters);
		struct{
			class APlayerController* Player;
		} Parms;
		Parms.Player=Player;
		ProcessEvent(NServerGetVoiceChatters, &Parms);
	}
	void StartClientDemoRec(){
		DECLARE_NAME(StartClientDemoRec);
		ProcessEvent(NStartClientDemoRec, NULL);
	}
	void MenuOpen(class UClass* MenuClass, const FString& Args){
		DECLARE_NAME(MenuOpen);
		struct{
			class UClass* MenuClass;
			FString Args;
		} Parms;
		Parms.MenuClass=MenuClass;
		Parms.Args=Args;
		ProcessEvent(NMenuOpen, &Parms);
	}
	void MenuClose(){
		DECLARE_NAME(MenuClose);
		ProcessEvent(NMenuClose, NULL);
	}
	void PotentialSubtitledSoundPlayed(const FString& SoundName, FLOAT Duration, INT Priority){
		DECLARE_NAME(PotentialSubtitledSoundPlayed);
		struct{
			FString SoundName;
			FLOAT Duration;
			INT Priority;
		} Parms;
		Parms.SoundName=SoundName;
		Parms.Duration=Duration;
		Parms.Priority=Priority;
		ProcessEvent(NPotentialSubtitledSoundPlayed, &Parms);
	}
	UBOOL InTacticalVisionMode(){
		DECLARE_NAME(InTacticalVisionMode);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NInTacticalVisionMode, &Parms);
		return Parms.ReturnValue;
	}
	void CheckpointSaveStarted(){
		DECLARE_NAME(CheckpointSaveStarted);
		ProcessEvent(NCheckpointSaveStarted, NULL);
	}

	//Functions
	INT GetNumFriendRequests();
	void SetNumFriendRequests(INT Num);
	INT GetNumGameInvites();
	void SetNumGameInvites(INT Num);
	INT GetLastNumFriendRequests();
	void SetLastNumFriendRequests(INT Num);
	INT GetLastNumGameInvites();
	void SetLastNumGameInvites(INT Num);
	FLOAT GetFriendRequestTimeout();
	void SetFriendRequestTimeout(FLOAT Timeout);
	FLOAT GetGameInviteTimeout();
	void SetGameInviteTimeout(FLOAT Timeout);
	FLOAT GetNextMatchmakingQueryTime();
	void SetNextMatchmakingQueryTime(FLOAT Time);
	void SetLevelProgress(const FString& Level);
	UBOOL HasReachedLevel(const FString& Level);
	INT GetLevelIndex(const FString& Level);
	int GetDesireType(class AActor*);
	void SetNetSpeed(int);
	void SetPlayer(class UPlayer*);
