	//TODO: See if this can be removed
	//Also defined in UnPath.h
	enum EReachSpecFlags{
		R_WALK = 1,	//walking required
		R_FLY = 2,   //flying required
		R_SWIM = 4,  //swimming required
		R_JUMP = 8,   // jumping required
		R_DOOR = 16,
		R_SPECIAL = 32,
		R_PLAYERONLY = 64
	};

	// Constructors.
	APawn(){}

	//Overrides
private:
	virtual class UStimulus* GenerateStimulus(enum EStimulusType);
	virtual class UStimulus* GenerateStimulus(enum EStimulusType, class FVector&);
public:
	virtual void CheckForErrors();
	virtual void DebugOutput(class FOutputDevice&);
	virtual void DebugOutputSelf(class FOutputDevice&);
	virtual void DebugWindowSizeGet(int&, int&)const;
	virtual int DelayScriptReplication(float);
	virtual class FRotator FindSlopeRotation(const FVector&, const FRotator&);
	virtual class ANavigationPoint* GetAnchor();
	virtual float GetNetPriority(class FVector&, class FVector&, class AActor*, float, float);
	virtual int* GetOptimizedRepList(unsigned char*, struct FPropertyRetirement*, int*, class UPackageMap*, class UActorChannel*);
	virtual class APawn* GetPlayerPawn()const;
	virtual int IsAPawn();
	virtual int IsNetRelevantFor(class APlayerController*, class AActor*, const FVector&);
	virtual bool IsPlayer()const;
	virtual class FVector LOSTestLocation()const;
	virtual void NotifyAnimEnd(int);
	virtual void NotifyBumpEx(class AActor*);
	virtual bool PlayerControlled()const;
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();
	virtual void PostNetSend();
	virtual void PostRender(class FLevelSceneNode*, class FRenderInterface*);
	virtual void PostScriptDestroyed();
	virtual void PreNetReceive();
	virtual void ReceiveStimulus(class UStimulus*);
	virtual void RenderEditorSelected(class FLevelSceneNode*, class FRenderInterface*, class FDynamicActor*);
	virtual void SetBase(class AActor*, const FVector&, int);
	virtual void SetZone(int, int);
	virtual int ShouldTrace(class AActor*, unsigned long);
	virtual void SmoothHitWall(const FVector&, class AActor*, unsigned char);
	virtual void SnapToNavPt(class ANavigationPoint*);
	virtual int Tick(float, enum ELevelTick);
	virtual void TickSimulated(float);
	virtual void TickSpecial(float);
	virtual void performPhysics(float);
	virtual void physFalling(float, int);
	virtual int physRootMotion(float);
	virtual void processHitWall(const FVector&, class AActor*, unsigned char);
	virtual void processLanded(const FVector&, class AActor*, float, int);
	virtual void stepUp(const FVector&, const FVector&, const FVector&, struct FCheckResult&);

	//Virtual Functions
	virtual bool IsDead()const;
	virtual bool AppearsDead()const;
	virtual int moveToward(const FVector&,class AActor*);
	virtual int pointReachable(const FVector&,int);
	virtual int actorReachable(class AActor*,int,int);
	virtual int HurtByVolume(class AActor*);
	virtual void physicsRotation(float,const FVector&);

	//Events
	void SetAnchor(class ANavigationPoint* Point);
	void StopTalking();
	void SaveSquadTravelInfo(){
		DECLARE_NAME(SaveSquadTravelInfo);
		UObject::ProcessEvent(NSaveSquadTravelInfo, NULL);
	}
	void SetWalking(UBOOL bNewIsWalking){
		DECLARE_NAME(SetWalking);
		struct{
			UBOOL bNewIsWalking;
		} Parms;
		Parms.bNewIsWalking=bNewIsWalking;
		UObject::ProcessEvent(NSetWalking, &Parms);
	}
	void ClientMessage(const FString& S, FName Type){
		DECLARE_NAME(ClientMessage);
		struct{
			FString S;
			FName Type;
		} Parms;
		Parms.S=S;
		Parms.Type=Type;
		UObject::ProcessEvent(NClientMessage, &Parms);
	}
	void ModifyVelocity(FLOAT DeltaTime, const FVector& OldVelocity){
		FName NModifyVelocity(NAME_ModifyVelocity);
		if(!IsProbing(NModifyVelocity))
			return;
		struct{
			FLOAT DeltaTime;
			FVector OldVelocity;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		Parms.OldVelocity=OldVelocity;
		UObject::ProcessEvent(NModifyVelocity, &Parms);
	}
	void EndCrouch(FLOAT HeightAdjust){
		DECLARE_NAME(EndCrouch);
		struct{
			FLOAT HeightAdjust;
		} Parms;
		Parms.HeightAdjust=HeightAdjust;
		UObject::ProcessEvent(NEndCrouch, &Parms);
	}
	void StartCrouch(FLOAT HeightAdjust){
		DECLARE_NAME(StartCrouch);
		struct{
			FLOAT HeightAdjust;
		} Parms;
		Parms.HeightAdjust=HeightAdjust;
		UObject::ProcessEvent(NStartCrouch, &Parms);
	}
	void TossWeapon(class AWeapon* WeaponToThrow, const FVector& TossVel){
		DECLARE_NAME(TossWeapon);
		struct{
			class AWeapon* WeaponToThrow;
			FVector TossVel;
		} Parms;
		Parms.WeaponToThrow=WeaponToThrow;
		Parms.TossVel=TossVel;
		UObject::ProcessEvent(NTossWeapon, &Parms);
	}
	class AInventory* FindInventoryType(class UClass* DesiredClass){
		DECLARE_NAME(FindInventoryType);
		struct{
			class UClass* DesiredClass;
			class AInventory* ReturnValue;
		} Parms;
		Parms.DesiredClass=DesiredClass;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NFindInventoryType, &Parms);
		return Parms.ReturnValue;
	}
	void ChangedWeapon(){
		DECLARE_NAME(ChangedWeapon);
		UObject::ProcessEvent(NChangedWeapon, NULL);
	}
	void UpdateEyeHeight(FLOAT DeltaTime){
		FName NUpdateEyeHeight(NAME_UpdateEyeHeight);
		if(!IsProbing(NUpdateEyeHeight))
			return;
		struct{
			FLOAT DeltaTime;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		UObject::ProcessEvent(NUpdateEyeHeight, &Parms);
	}
	FVector EyePosition(){
		DECLARE_NAME(EyePosition);
		struct{
			FVector ReturnValue;
		} Parms;
		Parms.ReturnValue=FVector(0);
		UObject::ProcessEvent(NEyePosition, &Parms);
		return Parms.ReturnValue;
	}
	void DieAgain(){
		DECLARE_NAME(DieAgain);
		UObject::ProcessEvent(NDieAgain, NULL);
	}
	void BleedOut(){
		FName NBleedOut(NAME_BleedOut);
		UObject::ProcessEvent(NBleedOut, NULL);
	}
	void HeadVolumeChange(class APhysicsVolume* newHeadVolume){
		FName NHeadVolumeChange(NAME_HeadVolumeChange);
		if(!IsProbing(NHeadVolumeChange))
			return;
		struct{
			class APhysicsVolume* newHeadVolume;
		} Parms;
		Parms.newHeadVolume=newHeadVolume;
		UObject::ProcessEvent(NHeadVolumeChange, &Parms);
	}
	void SetAnimAction(FName NewAction){
		DECLARE_NAME(SetAnimAction);
		struct{
			FName NewAction;
		} Parms;
		Parms.NewAction=NewAction;
		UObject::ProcessEvent(NSetAnimAction, &Parms);
	}
	void PawnStopFiring(){
		DECLARE_NAME(PawnStopFiring);
		UObject::ProcessEvent(NPawnStopFiring, NULL);
	}
	void ChangeAnimation(){
		DECLARE_NAME(ChangeAnimation);
		UObject::ProcessEvent(NChangeAnimation, NULL);
	}
	void EndJump(){
		DECLARE_NAME(EndJump);
		UObject::ProcessEvent(NEndJump, NULL);
	}
	void PlayJump(){
		DECLARE_NAME(PlayJump);
		UObject::ProcessEvent(NPlayJump, NULL);
	}
	void PlayFalling(){
		DECLARE_NAME(PlayFalling);
		UObject::ProcessEvent(NPlayFalling, NULL);
	}
	void PlayDodging(UBOOL bRight){
		DECLARE_NAME(PlayDodging);
		struct{
			UBOOL bRight;
		} Parms;
		Parms.bRight=bRight;
		UObject::ProcessEvent(NPlayDodging, &Parms);
	}
	void PlayDying(class UClass* DamageType, const FVector& HitLoc, FName BoneName){
		DECLARE_NAME(PlayDying);
		struct{
			class UClass* DamageType;
			FVector HitLoc;
			FName BoneName;
		} Parms;
		Parms.DamageType=DamageType;
		Parms.HitLoc=HitLoc;
		Parms.BoneName=BoneName;
		UObject::ProcessEvent(NPlayDying, &Parms);
	}
	void Revive(){
		DECLARE_NAME(Revive);
		UObject::ProcessEvent(NRevive, NULL);
	}
	UBOOL GoRagDoll(class UClass* DamageType, const FVector& HitLocation){
		DECLARE_NAME(GoRagDoll);
		struct{
			class UClass* DamageType;
			FVector HitLocation;
			UBOOL ReturnValue;
		} Parms;
		Parms.DamageType=DamageType;
		Parms.HitLocation=HitLocation;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGoRagDoll, &Parms);
		return Parms.ReturnValue;
	}
	void PlayIncapacitated(class UClass* DamageType, const FVector& HitLocation){
		DECLARE_NAME(PlayIncapacitated);
		struct{
			class UClass* DamageType;
			FVector HitLocation;
		} Parms;
		Parms.DamageType=DamageType;
		Parms.HitLocation=HitLocation;
		UObject::ProcessEvent(NPlayIncapacitated, &Parms);
	}
	UBOOL IsDeadOrIncapacitated(){
		DECLARE_NAME(IsDeadOrIncapacitated);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NIsDeadOrIncapacitated, &Parms);
		return Parms.ReturnValue;
	}
	void SetShields(FLOAT NewShields){
		DECLARE_NAME(SetShields);
		struct{
			FLOAT NewShields;
		} Parms;
		Parms.NewShields=NewShields;
		UObject::ProcessEvent(NSetShields, &Parms);
	}
	void SetHealthLevel(){
		DECLARE_NAME(SetHealthLevel);
		UObject::ProcessEvent(NSetHealthLevel, NULL);
	}
	void DoMeleeAttack(FName AttackBone, const FVector& BoneOffset){
		DECLARE_NAME(DoMeleeAttack);
		struct{
			FName AttackBone;
			FVector BoneOffset;
		} Parms;
		Parms.AttackBone=AttackBone;
		Parms.BoneOffset=BoneOffset;
		UObject::ProcessEvent(NDoMeleeAttack, &Parms);
	}
	FRotator GetWeaponRotation(){
		DECLARE_NAME(GetWeaponRotation);
		struct{
			FRotator ReturnValue;
		} Parms;
		Parms.ReturnValue=FRotator(0);
		UObject::ProcessEvent(NGetWeaponRotation, &Parms);
		return Parms.ReturnValue;
	}
	void PlayDeathCue(class UClass* DamageType){
		DECLARE_NAME(PlayDeathCue);
		struct{
			class UClass* DamageType;
		} Parms;
		Parms.DamageType=DamageType;
		UObject::ProcessEvent(NPlayDeathCue, &Parms);
	}
	void PlaySpottedCue(class AActor* Enemy){
		DECLARE_NAME(PlaySpottedCue);
		struct{
			class AActor* Enemy;
		} Parms;
		Parms.Enemy=Enemy;
		UObject::ProcessEvent(NPlaySpottedCue, &Parms);
	}
	FString GetHudDescription(){
		DECLARE_NAME(GetHudDescription);
		struct{
			FString ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetHudDescription, &Parms);
		return Parms.ReturnValue;
	}
	FString GetHudNickname(){
		DECLARE_NAME(GetHudNickname);
		struct{
			FString ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetHudNickname, &Parms);
		return Parms.ReturnValue;
	}

	//Functions
	int CacheNetRelevancy(int,class APlayerController*,class AActor*);
	enum EHealthLevel CalcHealthLevel(float);
	int CanCrouchWalk(const FVector&,const FVector&);
	FVector CheckForLedges(const FVector&,const FVector&,const FVector&,int&,int&);
	void Crouch(int);
	enum ETestMoveResult FindBestJump(const FVector&);
	enum ETestMoveResult FindJumpUp(const FVector&);
	float FindPath(class ANavigationPoint*,class ANavigationPoint*,class TArray<class UReachSpec*>&,bool);
	float GetMaxSpeed();
	enum ETeamRelationship GetRelationTowards(int) const;
	enum ETeamRelationship GetRelationTowards(class APawn const*)const;
	class USound* GetSoundFromCue(unsigned char,float,bool);
	int GetTeamIndex() const;
	float GetWeaponSpeedModifier() const;
	bool HasCueBeenPlayedRecently(unsigned char,float);
	bool HasSoundForCue(unsigned char);
	enum ETestMoveResult HitGoal(class AActor*);
	int IsAvailable() const;
	int IsFlashlightOn() const;
	bool IsHumanControlled() const;
	bool IsIncapacitated() const;
	bool IsLocallyControlled() const;
	int IsUnderAttack(float) const;
	int IsWounded() const;
	FVector NewFallVelocity(const FVector&,const FVector&,float);
	int PickWallAdjust(const FVector&);
	class USound* PlayOwnedCue(unsigned char,float);
	int Reachable(const FVector&,class AActor*);
	int ReachedDestination(const FVector&,class AActor*);
	void SetDodgeInfo();
	void SetTeamIndex(int);
	float SpeedFactor(const FVector&)const;
	FVector SuggestJumpVelocity(const FVector&,float,float);
	void UnCrouch(int);
	void UpdateMovementAnimation(float);
	bool UpdateSkeletalAiming(float);
	int UseWoundedAnims() const;
	int ValidAnchor();
	int calcMoveFlags();
	enum ETestMoveResult flyMove(const FVector&,class AActor*,float);
	int flyReachable(const FVector&,int,class AActor*);
	enum ETestMoveResult jumpLanding(const FVector&,int);
	int jumpReachable(const FVector&,int,class AActor*);
	void physFlying(float,int);
	void physSpider(float,int);
	void physSwimming(float,int);
	void physWalking(float,int);
	void setMoveTimer(float);
	void startNewPhysics(float,int);
	void startSwimming(const FVector&,const FVector&,float,float,int);
	enum ETestMoveResult swimMove(const FVector&,class AActor*,float);
	int swimReachable(const FVector&,int,class AActor*);
	enum ETestMoveResult walkMove(const FVector&,struct FCheckResult&,class AActor*,float);
	int walkReachable(const FVector&,int,class AActor*);

private:
	int Pick3DWallAdjust(const FVector&);
	void SpiderstepUp(const FVector&,const FVector&,struct FCheckResult&);
	float Swim(const FVector&,struct FCheckResult&);
	void UpdateSphyll();
	void calcVelocity(const FVector&,float,float,float,int,int,int);
	int checkFloor(const FVector&,struct FCheckResult&);
	int findNewFloor(const FVector&,float,float,int);
	FVector findWaterLine(const FVector&,const FVector&);