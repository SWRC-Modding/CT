	//Constructor
	AActor(){}

	//Overrides
	virtual void DebugOutput(class FOutputDevice&);
	virtual void DebugOutputSelf(class FOutputDevice&);
	virtual void Destroy();
	virtual FString GetDescription() const;
	virtual void InitExecution();
	virtual int IsPendingKill();
	virtual bool IsRuntimeStatic();
	virtual void NetDirty(class UProperty* property);
	virtual void PostEditChange();
	virtual void PostEditUndo();
	virtual void PostLoad();
	virtual void PreEditUndo();
	virtual void ProcessEvent(class UFunction* Function, void* Parms, void* UnusedResult = NULL);
	virtual int ProcessRemoteFunction(class UFunction* Function, void* Parms, struct FFrame* Stack);
	virtual void ProcessState(FLOAT DeltaSeconds);
	virtual void Serialize(class FArchive&);

	//Virtual functions
	virtual int Tick(FLOAT DeltaTime, enum ELevelTick TickType);
	virtual int* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, class UActorChannel* Channel);
	virtual void PostEditPaste();
	virtual FVector LOSTestLocation() const;
	virtual class ANavigationPoint* GetAnchor();
	virtual void SnapToNavPt(class ANavigationPoint*);
	virtual class APawn* GetPlayerPawn() const;
	virtual bool IsPlayer() const;
	virtual void ReceiveStimulus(class UStimulus*);
	virtual void FixupSubObjectPointers();
	virtual void DebugAnim(FOutputDevice&);
	virtual float GetNetPriority(FVector&, FVector&, AActor*, float, float);
	virtual float WorldLightRadius() const;
	virtual void PostEditMove();
	virtual void PostEditLoad();
	virtual void PreRaytrace();
	virtual void PostRaytrace();
	virtual void Spawned();
	virtual void PreNetReceive();
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();
	virtual class UMaterial* GetSkin(int);
	virtual int ShouldTickInEntry();
	virtual void PostNetSend();
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);
	virtual FCoords ToLocal() const;
	virtual FCoords ToWorld() const;
	virtual FMatrix LocalToWorld() const;
	virtual FMatrix WorldToLocal() const;
	virtual void PostScriptDestroyed();
	virtual int ShouldTrace(AActor*, unsigned long);
	virtual UPrimitive* GetPrimitive();
	virtual void NotifyBumpEx(AActor*);
	virtual void SetBase(AActor* NewBase, const FVector& NewFloor = FVector(0.0f, 0.0f, 1.0f), int bNotifyActor = 1);
	virtual void NotifyAnimEnd(int Channel);
	virtual void UpdateAnimation(FLOAT DeltaSeconds);
	virtual void StartAnimPoll();
	virtual int CheckAnimFinished(int Channel);
	virtual void TickAuthoritative(FLOAT DeltaSeconds);
	virtual void TickSimulated(FLOAT DeltaSeconds);
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual bool PlayerControlled() const;
	virtual int IsNetRelevantFor(class APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation);
	virtual int DelayScriptReplication(float);
	virtual void RenderEditorInfo(class FLevelSceneNode*, class FRenderInterface*, class FDynamicActor*);
	virtual void RenderEditorSelected(class FLevelSceneNode*, class FRenderInterface*, class FDynamicActor*);
	virtual float GetAmbientVolume(float);
	virtual void PlayOwnedSound(class USound*, int);
	virtual void PlayFootstepSound(unsigned char, unsigned char, class USound*);
	virtual void PlayOwnedFootstepSound(unsigned char, unsigned char, class USound*);
	virtual void SetZone(UBOOL bTest, UBOOL bForceRefresh);
	virtual void SetVolumes();
	virtual void PostBeginPlayEx();
	virtual void PostLoadBeginPlayEx();
	virtual void setPhysics(BYTE NewPhysics, AActor* NewFloor = NULL, const FVector& NewFloorV = FVector(0.0f, 0.0f, 1.0f));
	virtual void performPhysics(FLOAT DeltaSeconds);
	virtual void BoundProjectileVelocity();
	virtual void processHitWall(const FVector& HitNormal, AActor* HitActor, unsigned char);
	virtual void processLanded(const FVector& HitNormal, AActor* HitActor, FLOAT RemainingTime, INT Iterations);
	virtual void physFalling(FLOAT deltaTime, INT Iterations);
	virtual int physRootMotion(FLOAT DeltaTime);
	virtual FRotator FindSlopeRotation(const FVector& FloorNormal, const FRotator& NewRotation);
	virtual void SmoothHitWall(const FVector& HitNormal, AActor* HitActor, unsigned char);
	virtual void stepUp(const FVector&, const FVector&, const FVector&, struct FCheckResult&);
	virtual UBOOL ShrinkCollision(AActor* HitActor);
	virtual struct _McdModel* getKModel() const;
	virtual void physKarma(float);
	virtual void preKarmaStep(float);
	virtual void postKarmaStep();
	virtual void preContactUpdate();
private:
	virtual class UStimulus* GenerateStimulus(enum EStimulusType);
	virtual class UStimulus* GenerateStimulus(enum EStimulusType, FVector&);
public:
	virtual int AddMyMarker(AActor*);
	virtual void PostNavListBuild();
	virtual void ClearMarker();
	virtual void ClearPathReferences();
	virtual AActor* AssociatedLevelGeometry();
	virtual int HasAssociatedLevelGeometry(AActor*);
	virtual void PlayAnim(const struct FPlayAnim&);
	virtual void CheckForErrors();
	virtual void PrePath();
	virtual void PostPath();
	virtual AActor* GetProjectorBase();
	virtual int IsABrush();
	virtual int IsAMover();
	virtual int IsAVolume();
	virtual int IsAPlayerController();
	virtual int IsAPawn();
	virtual int IsAProjectile();
	virtual class APlayerController* GetTopPlayerController();

	//Events
	void AddToSquadInterest();
	void RemoveFromSquadInterest();
	void AddToTargetList();
	void RemoveFromTargetList();
	void ResetCollisionSize();
	UBOOL Move(const FVector& Delta);

	void AnimLoopEnd(FLOAT LoopStart){
		DECLARE_NAME(AnimLoopEnd);
		struct{
			FLOAT LoopStart;
		} Parms;
		Parms.LoopStart=LoopStart;
		UObject::ProcessEvent(NAnimLoopEnd, &Parms);
	}

	void AnimEnd(INT Channel){
		FName NAnimEnd(NAME_AnimEnd);
		if(!IsProbing(NAnimEnd))
			return;
		struct{
			INT Channel;
		} Parms;
		Parms.Channel=Channel;
		UObject::ProcessEvent(NAnimEnd, &Parms);
	}

	AActor* SpawnAttachment(FName BoneName, class UClass* ActorClass, class UStaticMesh* Mesh){
		DECLARE_NAME(SpawnAttachment);
		struct{
			FName BoneName;
			class UClass* ActorClass;
			class UStaticMesh* Mesh;
			AActor* ReturnValue;
		} Parms;
		Parms.BoneName=BoneName;
		Parms.ActorClass=ActorClass;
		Parms.Mesh=Mesh;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NSpawnAttachment, &Parms);
		return Parms.ReturnValue;
	}

	void DestroyAttachment(FName BoneName){
		DECLARE_NAME(DestroyAttachment);
		struct{
			FName BoneName;
		} Parms;
		Parms.BoneName=BoneName;
		UObject::ProcessEvent(NDestroyAttachment, &Parms);
	}

	void KImpact(AActor* Other, const FVector& pos, const FVector& impactVel, const FVector& impactNorm, BYTE MaterialHit){
		DECLARE_NAME(KImpact);
		struct{
			AActor* Other;
			FVector pos;
			FVector impactVel;
			FVector impactNorm;
			BYTE MaterialHit;
		} Parms;
		Parms.Other=Other;
		Parms.pos=pos;
		Parms.impactVel=impactVel;
		Parms.impactNorm=impactNorm;
		Parms.MaterialHit=MaterialHit;
		UObject::ProcessEvent(NKImpact, &Parms);
	}

	void KVelDropBelow(){
		DECLARE_NAME(KVelDropBelow);
		UObject::ProcessEvent(NKVelDropBelow, NULL);
	}

	void KAtRest(){
		DECLARE_NAME(KAtRest);
		UObject::ProcessEvent(NKAtRest, NULL);
	}

	void KSkelConvulse(){
		DECLARE_NAME(KSkelConvulse);
		UObject::ProcessEvent(NKSkelConvulse, NULL);
	}

	void KApplyForce(FVector& Force, FVector& Torque){
		DECLARE_NAME(KApplyForce);
		struct{
			FVector Force;
			FVector Torque;
		} Parms;
		Parms.Force=Force;
		Parms.Torque=Torque;
		UObject::ProcessEvent(NKApplyForce, &Parms);
		Force=Parms.Force;
		Torque=Parms.Torque;
	}

	UBOOL KUpdateState(FKRigidBodyState& newState){
		DECLARE_NAME(KUpdateState);
		struct{
			FKRigidBodyState newState;
			UBOOL ReturnValue;
		} Parms;
		Parms.newState=newState;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NKUpdateState, &Parms);
		newState=Parms.newState;
		return Parms.ReturnValue;
	}

	void Destroyed(){
		FName NDestroyed(NAME_Destroyed);
		if(!IsProbing(NDestroyed))
			return;
		UObject::ProcessEvent(NDestroyed, NULL);
	}

	void GainedChild(AActor* Other){
		FName NGainedChild(NAME_GainedChild);
		if(!IsProbing(NGainedChild))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NGainedChild, &Parms);
	}

	void LostChild(AActor* Other){
		FName NLostChild(NAME_LostChild);
		if(!IsProbing(NLostChild))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NLostChild, &Parms);
	}

	void Tick(FLOAT DeltaTime){
		FName NTick(NAME_Tick);
		if(!IsProbing(NTick))
			return;
		struct{
			FLOAT DeltaTime;
		} Parms;
		Parms.DeltaTime=DeltaTime;
		UObject::ProcessEvent(NTick, &Parms);
	}

	void PostNetworkReceive(){
		DECLARE_NAME(PostNetworkReceive);
		UObject::ProcessEvent(NPostNetworkReceive, NULL);
	}

	void Trigger(AActor* Other, class APawn* EventInstigator){
		FName NTrigger(NAME_Trigger);
		if(!IsProbing(NTrigger))
			return;
		struct{
			AActor* Other;
			class APawn* EventInstigator;
		} Parms;
		Parms.Other=Other;
		Parms.EventInstigator=EventInstigator;
		UObject::ProcessEvent(NTrigger, &Parms);
	}

	void UnTrigger(AActor* Other, class APawn* EventInstigator){
		FName NUnTrigger(NAME_UnTrigger);
		if(!IsProbing(NUnTrigger))
			return;
		struct{
			AActor* Other;
			class APawn* EventInstigator;
		} Parms;
		Parms.Other=Other;
		Parms.EventInstigator=EventInstigator;
		UObject::ProcessEvent(NUnTrigger, &Parms);
	}

	void BeginEvent(){
		DECLARE_NAME(BeginEvent);
		UObject::ProcessEvent(NBeginEvent, NULL);
	}

	void EndEvent(){
		DECLARE_NAME(EndEvent);
		UObject::ProcessEvent(NEndEvent, NULL);
	}

	void Timer(){
		FName NTimer(NAME_Timer);
		if(!IsProbing(NTimer))
			return;
		UObject::ProcessEvent(NTimer, NULL);
	}

	void HitWall(const FVector& HitNormal, AActor* HitWall, BYTE KindOfMaterial){
		FName NHitWall(NAME_HitWall);
		if(!IsProbing(NHitWall))
			return;
		struct{
			FVector HitNormal;
			AActor* HitWall;
			BYTE KindOfMaterial;
		} Parms;
		Parms.HitNormal=HitNormal;
		Parms.HitWall=HitWall;
		Parms.KindOfMaterial=KindOfMaterial;
		UObject::ProcessEvent(NHitWall, &Parms);
	}

	void Falling(){
		FName NFalling(NAME_Falling);
		if(!IsProbing(NFalling))
			return;
		UObject::ProcessEvent(NFalling, NULL);
	}

	void Landed(const FVector& HitNormal){
		FName NLanded(NAME_Landed);
		if(!IsProbing(NLanded))
			return;
		struct{
			FVector HitNormal;
		} Parms;
		Parms.HitNormal=HitNormal;
		UObject::ProcessEvent(NLanded, &Parms);
	}

	void ZoneChange(AZoneInfo* NewZone){
		FName NZoneChange(NAME_ZoneChange);
		if(!IsProbing(NZoneChange))
			return;
		struct{
			AZoneInfo* NewZone;
		} Parms;
		Parms.NewZone=NewZone;
		UObject::ProcessEvent(NZoneChange, &Parms);
	}

	void PhysicsVolumeChange(class APhysicsVolume* NewVolume){
		FName NPhysicsVolumeChange(NAME_PhysicsVolumeChange);
		if(!IsProbing(NPhysicsVolumeChange))
			return;
		struct{
			class APhysicsVolume* NewVolume;
		} Parms;
		Parms.NewVolume=NewVolume;
		UObject::ProcessEvent(NPhysicsVolumeChange, &Parms);
	}

	void Touch(AActor* Other){
		FName NTouch(NAME_Touch);
		if(!IsProbing(NTouch))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NTouch, &Parms);
	}

	void PostTouch(AActor* Other){
		FName NPostTouch(NAME_PostTouch);
		if(!IsProbing(NPostTouch))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NPostTouch, &Parms);
	}

	void UnTouch(AActor* Other){
		FName NUnTouch(NAME_UnTouch);
		if(!IsProbing(NUnTouch))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NUnTouch, &Parms);
	}

	void Bump(AActor* Other){
		FName NBump(NAME_Bump);
		if(!IsProbing(NBump))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NBump, &Parms);
	}

	void BaseChange(){
		FName NBaseChange(NAME_BaseChange);
		if(!IsProbing(NBaseChange))
			return;
		UObject::ProcessEvent(NBaseChange, NULL);
	}

	void Attach(AActor* Other){
		FName NAttach(NAME_Attach);
		if(!IsProbing(NAttach))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NAttach, &Parms);
	}

	void Detach(AActor* Other){
		FName NDetach(NAME_Detach);
		if(!IsProbing(NDetach))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NDetach, &Parms);
	}

	UBOOL EncroachingOn(AActor* Other){
		FName NEncroachingOn(NAME_EncroachingOn);
		if(!IsProbing(NEncroachingOn))
			return 0;
		struct{
			AActor* Other;
			UBOOL ReturnValue;
		} Parms;
		Parms.Other=Other;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NEncroachingOn, &Parms);
		return Parms.ReturnValue;
	}

	void EncroachedBy(AActor* Other){
		FName NEncroachedBy(NAME_EncroachedBy);
		if(!IsProbing(NEncroachedBy))
			return;
		struct{
			AActor* Other;
		} Parms;
		Parms.Other=Other;
		UObject::ProcessEvent(NEncroachedBy, &Parms);
	}

	void FinishedInterpolation(){
		DECLARE_NAME(FinishedInterpolation);
		UObject::ProcessEvent(NFinishedInterpolation, NULL);
	}

	void EndedRotation(){
		FName NEndedRotation(NAME_EndedRotation);
		if(!IsProbing(NEndedRotation))
			return;
		UObject::ProcessEvent(NEndedRotation, NULL);
	}

	void UsedBy(class APawn* User){
		DECLARE_NAME(UsedBy);
		struct{
			class APawn* User;
		} Parms;
		Parms.User=User;
		UObject::ProcessEvent(NUsedBy, &Parms);
	}

	void FellOutOfWorld(BYTE KillType){
		DECLARE_NAME(FellOutOfWorld);
		struct{
			BYTE KillType;
		} Parms;
		Parms.KillType=KillType;
		UObject::ProcessEvent(NFellOutOfWorld, &Parms);
	}

	void KilledBy(class APawn* EventInstigator, class UClass* DamageType){
		DECLARE_NAME(KilledBy);
		struct{
			class APawn* EventInstigator;
			class UClass* DamageType;
		} Parms;
		Parms.EventInstigator=EventInstigator;
		Parms.DamageType=DamageType;
		UObject::ProcessEvent(NKilledBy, &Parms);
	}

	FLOAT TakeDamage(FLOAT Damage, class APawn* EventInstigator, const FVector& HitLocation, const FVector& Momentum, class UClass* DamageType, FName BoneName){
		DECLARE_NAME(TakeDamage);
		struct{
			FLOAT Damage;
			class APawn* EventInstigator;
			FVector HitLocation;
			FVector Momentum;
			class UClass* DamageType;
			FName BoneName;
			FLOAT ReturnValue;
		} Parms;
		Parms.Damage=Damage;
		Parms.EventInstigator=EventInstigator;
		Parms.HitLocation=HitLocation;
		Parms.Momentum=Momentum;
		Parms.DamageType=DamageType;
		Parms.BoneName=BoneName;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NTakeDamage, &Parms);
		return Parms.ReturnValue;
	}

	void TornOff(){
		DECLARE_NAME(TornOff);
		UObject::ProcessEvent(NTornOff, NULL);
	}

	FVector CalcTrajectoryVelocity(const FVector& InitialPosition, const FVector& TargetPosition, FLOAT HorizontalVelocity, UBOOL AllowFlatArcs);

	UBOOL PreTeleport(class ATeleporter* InTeleporter){
		DECLARE_NAME(PreTeleport);
		struct{
			class ATeleporter* InTeleporter;
			UBOOL ReturnValue;
		} Parms;
		Parms.InTeleporter=InTeleporter;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NPreTeleport, &Parms);
		return Parms.ReturnValue;
	}

	void PostTeleport(class ATeleporter* OutTeleporter){
		DECLARE_NAME(PostTeleport);
		struct{
			class ATeleporter* OutTeleporter;
		} Parms;
		Parms.OutTeleporter=OutTeleporter;
		UObject::ProcessEvent(NPostTeleport, &Parms);
	}

	void BeginPlay(){
		FName NBeginPlay(NAME_BeginPlay);
		if(!IsProbing(NBeginPlay))
			return;
		UObject::ProcessEvent(NBeginPlay, NULL);
	}

	void RenderTexture(class UScriptedTexture* Tex){
		DECLARE_NAME(RenderTexture);
		struct{
			class UScriptedTexture* Tex;
		} Parms;
		Parms.Tex=Tex;
		UObject::ProcessEvent(NRenderTexture, &Parms);
	}

	void PreBeginPlay(){
		FName NPreBeginPlay(NAME_PreBeginPlay);
		if(!IsProbing(NPreBeginPlay))
			return;
		UObject::ProcessEvent(NPreBeginPlay, NULL);
	}

	void BroadcastLocalizedMessage(class UClass* MessageClass, INT Switch, class APlayerReplicationInfo* RelatedPRI_1, class APlayerReplicationInfo* RelatedPRI_2, class UObject* OptionalObject){
		DECLARE_NAME(BroadcastLocalizedMessage);
		struct{
			class UClass* MessageClass;
			INT Switch;
			class APlayerReplicationInfo* RelatedPRI_1;
			class APlayerReplicationInfo* RelatedPRI_2;
			class UObject* OptionalObject;
		} Parms;
		Parms.MessageClass=MessageClass;
		Parms.Switch=Switch;
		Parms.RelatedPRI_1=RelatedPRI_1;
		Parms.RelatedPRI_2=RelatedPRI_2;
		Parms.OptionalObject=OptionalObject;
		UObject::ProcessEvent(NBroadcastLocalizedMessage, &Parms);
	}

	void PostBeginPlay(){
		FName NPostBeginPlay(NAME_PostBeginPlay);
		if(!IsProbing(NPostBeginPlay))
			return;
		UObject::ProcessEvent(NPostBeginPlay, NULL);
	}

	void SetInitialState(){
		DECLARE_NAME(SetInitialState);
		UObject::ProcessEvent(NSetInitialState, NULL);
	}

	void PostLoadBeginPlay(){
		FName NPostLoadBeginPlay(NAME_PostLoadBeginPlay);
		if(!IsProbing(NPostLoadBeginPlay))
			return;
		UObject::ProcessEvent(NPostLoadBeginPlay, NULL);
	}

	void PostNetBeginPlay(){
		DECLARE_NAME(PostNetBeginPlay);
		UObject::ProcessEvent(NPostNetBeginPlay, NULL);
	}

	void HurtRadius(FLOAT DamageAmount, FLOAT DamageRadius, class UClass* DamageType, FLOAT Momentum, const FVector& HitLocation, AActor* ExcludedActor){
		DECLARE_NAME(HurtRadius);
		struct{
			FLOAT DamageAmount;
			FLOAT DamageRadius;
			class UClass* DamageType;
			FLOAT Momentum;
			FVector HitLocation;
			AActor* ExcludedActor;
		} Parms;
		Parms.DamageAmount=DamageAmount;
		Parms.DamageRadius=DamageRadius;
		Parms.DamageType=DamageType;
		Parms.Momentum=Momentum;
		Parms.HitLocation=HitLocation;
		Parms.ExcludedActor=ExcludedActor;
		UObject::ProcessEvent(NHurtRadius, &Parms);
	}

	void TravelPreAccept(){
		DECLARE_NAME(TravelPreAccept);
		UObject::ProcessEvent(NTravelPreAccept, NULL);
	}

	void TravelPostAccept(){
		DECLARE_NAME(TravelPostAccept);
		UObject::ProcessEvent(NTravelPostAccept, NULL);
	}

	FString GetItemName(const FString& FullName){
		DECLARE_NAME(GetItemName);
		struct{
			FString FullName;
			FString ReturnValue;
		} Parms;
		Parms.FullName=FullName;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetItemName, &Parms);
		return Parms.ReturnValue;
	}

	void TriggerEvent(FName EventName, AActor* Other, class APawn* EventInstigator){
		DECLARE_NAME(TriggerEvent);
		struct{
			FName EventName;
			AActor* Other;
			class APawn* EventInstigator;
		} Parms;
		Parms.EventName=EventName;
		Parms.Other=Other;
		Parms.EventInstigator=EventInstigator;
		UObject::ProcessEvent(NTriggerEvent, &Parms);
	}

	UBOOL IsInVolume(class AVolume* aVolume){
		DECLARE_NAME(IsInVolume);
		struct{
			class AVolume* aVolume;
			UBOOL ReturnValue;
		} Parms;
		Parms.aVolume=aVolume;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NIsInVolume, &Parms);
		return Parms.ReturnValue;
	}

	UBOOL ContainsPartialEvent(const FString& StartOfEventName){
		DECLARE_NAME(ContainsPartialEvent);
		struct{
			FString StartOfEventName;
			UBOOL ReturnValue;
		} Parms;
		Parms.StartOfEventName=StartOfEventName;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NContainsPartialEvent, &Parms);
		return Parms.ReturnValue;
	}

	UBOOL ContainsPartialTag(const FString& StartOfTagName){
		DECLARE_NAME(ContainsPartialTag);
		struct{
			FString StartOfTagName;
			UBOOL ReturnValue;
		} Parms;
		Parms.StartOfTagName=StartOfTagName;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NContainsPartialTag, &Parms);
		return Parms.ReturnValue;
	}

	//Functions
	FVector AimLocation() const;
	void AttachProjector(class AProjector* Projector);
	UBOOL AttachToBone(AActor* Attachment, FName BoneName);
	void BeginTouch(AActor* Other);
	void ClearOctreeData();
	void ClearRenderData();
	UBOOL DetachFromBone(AActor* Attachment);
	void DetachProjector(class AProjector* Projector);
	int EditorContainsPartialEvent(const FString&);
	int EditorContainsPartialTag(const FString&);
	void EndTouch(AActor* Other, UBOOL NoNotifySelf);
	void FindBase();
	class FDynamicActor* GetActorRenderData();
	AActor* GetAmbientLightingActor();
	FVector GetAnimRootTranslation(const FName&);
	FVector GetBoneLocation(FName, enum ESpace);
	FRotator GetBoneRotation(FName, enum ESpace);
	FVector GetCylinderExtent() const{ return FVector(CollisionRadius, CollisionRadius, CollisionHeight); }
	AActor* GetHitActor(){ return this; }
	unsigned char GetLastTouchedMaterialType() const;
	FORCEINLINE class ULevel* GetLevel() const{ return XLevel; }
	class FDynamicLight* GetLightRenderData();
	FVector GetMarkerSpotLocation(int);
	void GetNetBuoyancy(FLOAT& NetBuoyancy, FLOAT& NetFluidFriction);
	AActor* GetTopOwner();
	FRotator GetViewRotation();
	UBOOL IsAnimating(int Channel = 0) const;
	UBOOL IsBasedOn(const AActor* Other) const;
	UBOOL IsBlockedBy(const AActor* Other) const;
	UBOOL IsBrush() const;
	UBOOL IsEncroacher() const;
	bool IsHiddenEd();
	UBOOL IsInOctree();
	UBOOL IsInZone(const AZoneInfo* Other) const;
	UBOOL IsJoinedTo(const AActor* Other) const;
	UBOOL IsMovingBrush() const;
	UBOOL IsOverlapping(AActor* Other, FCheckResult* Hit = NULL);
	UBOOL IsOwnedBy(const AActor* TestOwner) const;
	bool IsPlayingAnim(const FName&, const FName&);
	UBOOL IsStaticBrush() const;
	UBOOL IsValidEnemy() const;
	UBOOL IsVolumeBrush() const;
	void KAddForces(const FVector&, const FVector&);
	void KDestroyJointChain(int);
	void KDrawRigidBodyState(struct FKRigidBodyState*, int);
	void KFreezeRagdoll();
	void KGetRigidBodyState(struct FKRigidBodyState*);
	int KIsAwake();
	int KIsRagdollAvailable();
	void KMakeRagdollAvailable();
	void KWake();
	FORCEINLINE FLOAT LifeFraction(){ return Clamp(1.0f - LifeSpan / GetClass()->GetDefaultActor()->LifeSpan, 0.0f, 1.0f); }
	void PlayReplicatedAnim(int);
	class UStimulus* PostStimulusToIndividual(EStimulusType, AActor*);
	class UStimulus* PostStimulusToWorld(EStimulusType);
	class UStimulus* PostStimulusToWorld(EStimulusType, FVector&);
	void ProcessDemoRecFunction(UFunction* Function, void* Parms, FFrame* Stack);
	void ReplicateAnim(int, int, const struct FAnimChannel&, bool);
	void SetCollision(UBOOL NewCollideActors, UBOOL NewBlockActors, UBOOL NewBlockPlayers);
	void SetCollisionSize(FLOAT NewRadius, FLOAT NewHeight);
	void SetDrawScale3D(const FVector& NewScale3D);
	void SetDrawScale(FLOAT NewScale);
	void SetDrawType(EDrawType NewDrawType);
	int SetLocation(const FVector&, int);
	void SetOwner(AActor* Owner);
	int SetRotation(const FRotator&);
	void SetStaticMesh(class UStaticMesh* NewStaticMesh);
	void StopAnimating(int);
	FVector SuggestFallVelocity(const FVector& Dest, const FVector& Start, FLOAT XYSpeed, FLOAT BaseZ, FLOAT JumpZ, FLOAT MaxXYSpeed);
	int TestCanSeeMe(class APlayerController* Viewer);
	void TwoWallAdjust(const FVector& DesiredDir, FVector& Delta, FVector& HitNormal, FVector& OldHitNormal, FLOAT HitTime);
	void UpdateRelativeRotation();
	void UpdateRenderData();
	void UpdateTimers(FLOAT DeltaSeconds);
	int fixedTurn(int current, int desired, int deltaRate);
	int moveSmooth(const FVector& Delta);
	void physKarmaRagDoll(FLOAT DeltaTime);
	void physProjectile(FLOAT deltaTime, INT Iterations);
	void physTrailer(FLOAT DeltaTime);
	void physicsRotation(FLOAT DeltaTime);
	void postKarmaStep_skeletal();
	void preKarmaStep_skeletal(FLOAT DeltaTime);
