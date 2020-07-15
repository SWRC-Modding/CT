	//Overrides
	virtual int CheckAnimFinished(int);
	virtual int* GetOptimizedRepList(unsigned char*, struct FPropertyRetirement*, int*, class UPackageMap*, class UActorChannel*);
	virtual void StartAnimPoll();
	virtual int Tick(float, enum ELevelTick);

	//Virtual Functions
	virtual bool AppearsDead() const;
	virtual bool SetEnemy(class AActor*);
	virtual int InCombat() const;
	virtual void CheckHearSound(class AActor*, class USound*, int);
	virtual int SafeToShoot(class AActor*, const FVector&);
	virtual class AActor* GetViewTarget();
	virtual void WeaponFired();
	virtual class FVector GetFireStart();
	virtual void SetAnchor(class ANavigationPoint*);
	virtual bool CrossesMyFire(const FVector&, const FVector&);
	virtual int AcceptNearbyPath(class AActor*);
	virtual void AdjustFromWall(const FVector&, class AActor*);
	virtual void SetAdjustLocation(const FVector&);
	virtual int LocalPlayerController();
	virtual int WantsLedgeCheck();
	virtual int StopAtLedge();
	virtual void DebugDraw(class FLineBatcher&);
	virtual int ScriptedGoalsFinished() const;
	virtual int IsAvailable() const;

	//Events
	void MayFall(){
		FName NMayFall(NAME_MayFall);
		if(!IsProbing(NMayFall))
			return;
		ProcessEvent(NMayFall, NULL);
	}
	FColor GetDebugColor(){
		DECLARE_NAME(GetDebugColor);
		struct{
			FColor ReturnValue;
		} Parms;
		Parms.ReturnValue=FColor(0);
		ProcessEvent(NGetDebugColor, &Parms);
		return Parms.ReturnValue;
	}
	void SoakStop(const FString& problem){
		DECLARE_NAME(SoakStop);
		struct{
			FString problem;
		} Parms;
		Parms.problem=problem;
		ProcessEvent(NSoakStop, &Parms);
	}
	void LongFall(){
		DECLARE_NAME(LongFall);
		ProcessEvent(NLongFall, NULL);
	}
	UBOOL NotifyPhysicsVolumeChange(class APhysicsVolume* NewVolume){
		FName NNotifyPhysicsVolumeChange(NAME_NotifyPhysicsVolumeChange);
		if(!IsProbing(NNotifyPhysicsVolumeChange))
			return 0;
		struct{
			class APhysicsVolume* NewVolume;
			UBOOL ReturnValue;
		} Parms;
		Parms.NewVolume=NewVolume;
		Parms.ReturnValue=0;
		ProcessEvent(NNotifyPhysicsVolumeChange, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL NotifyHeadVolumeChange(class APhysicsVolume* NewVolume){
		FName NNotifyHeadVolumeChange(NAME_NotifyHeadVolumeChange);
		if(!IsProbing(NNotifyHeadVolumeChange))
			return 0;
		struct{
			class APhysicsVolume* NewVolume;
			UBOOL ReturnValue;
		} Parms;
		Parms.NewVolume=NewVolume;
		Parms.ReturnValue=0;
		ProcessEvent(NNotifyHeadVolumeChange, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL NotifyLanded(const FVector& HitNormal){
		FName NNotifyLanded(NAME_NotifyLanded);
		if(!IsProbing(NNotifyLanded))
			return 0;
		struct{
			FVector HitNormal;
			UBOOL ReturnValue;
		} Parms;
		Parms.HitNormal=HitNormal;
		Parms.ReturnValue=0;
		ProcessEvent(NNotifyLanded, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL NotifyHitWall(const FVector& HitNormal, class AActor* Wall){
		FName NNotifyHitWall(NAME_NotifyHitWall);
		if(!IsProbing(NNotifyHitWall))
			return 0;
		struct{
			FVector HitNormal;
			class AActor* Wall;
			UBOOL ReturnValue;
		} Parms;
		Parms.HitNormal=HitNormal;
		Parms.Wall=Wall;
		Parms.ReturnValue=0;
		ProcessEvent(NNotifyHitWall, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL NotifyBump(class AActor* Other){
		FName NNotifyBump(NAME_NotifyBump);
		if(!IsProbing(NNotifyBump))
			return 0;
		struct{
			class AActor* Other;
			UBOOL ReturnValue;
		} Parms;
		Parms.Other=Other;
		Parms.ReturnValue=0;
		ProcessEvent(NNotifyBump, &Parms);
		return Parms.ReturnValue;
	}
	void NotifyHitMover(const FVector& HitNormal, class AMover* Wall){
		DECLARE_NAME(NotifyHitMover);
		struct{
			FVector HitNormal;
			class AMover* Wall;
		} Parms;
		Parms.HitNormal=HitNormal;
		Parms.Wall=Wall;
		ProcessEvent(NNotifyHitMover, &Parms);
	}
	void NotifyJumpApex(){
		DECLARE_NAME(NotifyJumpApex);
		ProcessEvent(NNotifyJumpApex, NULL);
	}
	void NotifyMissedJump(){
		DECLARE_NAME(NotifyMissedJump);
		ProcessEvent(NNotifyMissedJump, NULL);
	}
	void PrepareForMove(class ANavigationPoint* Goal, class UReachSpec* Path){
		FName NPrepareForMove(NAME_PrepareForMove);
		if(!IsProbing(NPrepareForMove))
			return;
		struct{
			class ANavigationPoint* Goal;
			class UReachSpec* Path;
		} Parms;
		Parms.Goal=Goal;
		Parms.Path=Path;
		ProcessEvent(NPrepareForMove, &Parms);
	}
	void HearNoise(FLOAT Loudness, class AActor* NoiseMaker){
		FName NHearNoise(NAME_HearNoise);
		if(!IsProbing(NHearNoise))
			return;
		struct{
			FLOAT Loudness;
			class AActor* NoiseMaker;
		} Parms;
		Parms.Loudness=Loudness;
		Parms.NoiseMaker=NoiseMaker;
		ProcessEvent(NHearNoise, &Parms);
	}
	void SeePlayer(class APawn* Seen){
		FName NSeePlayer(NAME_SeePlayer);
		if(!IsProbing(NSeePlayer))
			return;
		struct{
			class APawn* Seen;
		} Parms;
		Parms.Seen=Seen;
		ProcessEvent(NSeePlayer, &Parms);
	}
	void StopShake(){
		DECLARE_NAME(StopShake);
		ProcessEvent(NStopShake, NULL);
	}
	void ShakeView(FLOAT InTime, FLOAT SustainTime, FLOAT OutTime, FLOAT XMag, FLOAT YMag, FLOAT ZMag, FLOAT YawMag, FLOAT PitchMag, FLOAT Frequency){
		DECLARE_NAME(ShakeView);
		struct{
			FLOAT InTime;
			FLOAT SustainTime;
			FLOAT OutTime;
			FLOAT XMag;
			FLOAT YMag;
			FLOAT ZMag;
			FLOAT YawMag;
			FLOAT PitchMag;
			FLOAT Frequency;
		} Parms;
		Parms.InTime=InTime;
		Parms.SustainTime=SustainTime;
		Parms.OutTime=OutTime;
		Parms.XMag=XMag;
		Parms.YMag=YMag;
		Parms.ZMag=ZMag;
		Parms.YawMag=YawMag;
		Parms.PitchMag=PitchMag;
		Parms.Frequency=Frequency;
		ProcessEvent(NShakeView, &Parms);
	}
	void ThrowScav(){
		DECLARE_NAME(ThrowScav);
		ProcessEvent(NThrowScav, NULL);
	}
	void SwitchGrenade(FLOAT F){
		DECLARE_NAME(SwitchGrenade);
		struct{
			FLOAT F;
		} Parms;
		Parms.F=F;
		ProcessEvent(NSwitchGrenade, &Parms);
	}
	void ServerReStartPlayer(){
		DECLARE_NAME(ServerReStartPlayer);
		ProcessEvent(NServerReStartPlayer, NULL);
	}
	void MonitoredPawnAlert(){
		DECLARE_NAME(MonitoredPawnAlert);
		ProcessEvent(NMonitoredPawnAlert, NULL);
	}

	//Functions
	int CanHear(const FVector&, float, float, float, class AActor*);
	int CanHear(class UStimulus*);
	int CanHearSound(const FVector&, class AActor*, float);
	void CheckFears();
	void EnableTurning(bool, bool, bool);
	FVector GetFocusLocation();
	FRotator GetFocusRotation();
	unsigned long LineOfSightTo(class AActor*, int);
	int SafeToShoot(class AActor*);
	int SeeActor(class AActor*, int);
	void SetFocus(const FRotator&);
	void SetFocus(const FVector&);
	void SetFocus(class AActor*);
	void SetupMoveToward(class AActor*, float, int);
