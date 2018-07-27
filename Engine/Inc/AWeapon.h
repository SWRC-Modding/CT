	//Events
	void TurretFire(){
		DECLARE_NAME(TurretFire);
		UObject::ProcessEvent(NTurretFire, NULL);
	}
	void LoadAttachment(){
		DECLARE_NAME(LoadAttachment);
		UObject::ProcessEvent(NLoadAttachment, NULL);
	}
	void HolsterAttachment(){
		DECLARE_NAME(HolsterAttachment);
		UObject::ProcessEvent(NHolsterAttachment, NULL);
	}
	UBOOL IsTargeting(){
		DECLARE_NAME(IsTargeting);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NIsTargeting, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL IsTargetLocked(){
		DECLARE_NAME(IsTargetLocked);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NIsTargetLocked, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetTargetLockTime(){
		DECLARE_NAME(GetTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetElapsedTargetLockTime(){
		DECLARE_NAME(GetElapsedTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetElapsedTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	class UTexture* GetLockReticle(){
		DECLARE_NAME(GetLockReticle);
		struct{
			class UTexture* ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetLockReticle, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL IsSecondaryTargetLocked(){
		DECLARE_NAME(IsSecondaryTargetLocked);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NIsSecondaryTargetLocked, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetSecondaryTargetLockTime(){
		DECLARE_NAME(GetSecondaryTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetSecondaryTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetElapsedSecondaryTargetLockTime(){
		DECLARE_NAME(GetElapsedSecondaryTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NGetElapsedSecondaryTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	void ServerFire(){
		DECLARE_NAME(ServerFire);
		UObject::ProcessEvent(NServerFire, NULL);
	}
	void StopFire(FLOAT Value){
		DECLARE_NAME(StopFire);
		struct{
			FLOAT Value;
		} Parms;
		Parms.Value=Value;
		UObject::ProcessEvent(NStopFire, &Parms);
	}
	void ServerStopFire(){
		DECLARE_NAME(ServerStopFire);
		UObject::ProcessEvent(NServerStopFire, NULL);
	}
	FVector GetFireStart(const FVector& X, const FVector& Y, const FVector& Z){
		DECLARE_NAME(GetFireStart);
		struct{
			FVector X;
			FVector Y;
			FVector Z;
			FVector ReturnValue;
		} Parms;
		Parms.X=X;
		Parms.Y=Y;
		Parms.Z=Z;
		Parms.ReturnValue=FVector(0);
		UObject::ProcessEvent(NGetFireStart, &Parms);
		return Parms.ReturnValue;
	}
	void ForceReload(){
		DECLARE_NAME(ForceReload);
		UObject::ProcessEvent(NForceReload, NULL);
	}
	void ServerForceReload(){
		DECLARE_NAME(ServerForceReload);
		UObject::ProcessEvent(NServerForceReload, NULL);
	}
	void ServerThrowGrenade(){
		DECLARE_NAME(ServerThrowGrenade);
		UObject::ProcessEvent(NServerThrowGrenade, NULL);
	}
	void ReleaseGrenade(){
		DECLARE_NAME(ReleaseGrenade);
		UObject::ProcessEvent(NReleaseGrenade, NULL);
	}
	void DoMeleeAttack(){
		DECLARE_NAME(DoMeleeAttack);
		UObject::ProcessEvent(NDoMeleeAttack, NULL);
	}
	UBOOL CanSwitchWeapon(){
		DECLARE_NAME(CanSwitchWeapon);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NCanSwitchWeapon, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT SwitchPriority(){
		DECLARE_NAME(SwitchPriority);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		UObject::ProcessEvent(NSwitchPriority, &Parms);
		return Parms.ReturnValue;
	}