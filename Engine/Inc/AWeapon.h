	//Events
	void TurretFire(){
		DECLARE_NAME(TurretFire);
		ProcessEvent(NTurretFire, NULL);
	}
	void LoadAttachment(){
		DECLARE_NAME(LoadAttachment);
		ProcessEvent(NLoadAttachment, NULL);
	}
	void HolsterAttachment(){
		DECLARE_NAME(HolsterAttachment);
		ProcessEvent(NHolsterAttachment, NULL);
	}
	UBOOL IsTargeting(){
		DECLARE_NAME(IsTargeting);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NIsTargeting, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL IsTargetLocked(){
		DECLARE_NAME(IsTargetLocked);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NIsTargetLocked, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetTargetLockTime(){
		DECLARE_NAME(GetTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NGetTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetElapsedTargetLockTime(){
		DECLARE_NAME(GetElapsedTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NGetElapsedTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	class UTexture* GetLockReticle(){
		DECLARE_NAME(GetLockReticle);
		struct{
			class UTexture* ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NGetLockReticle, &Parms);
		return Parms.ReturnValue;
	}
	UBOOL IsSecondaryTargetLocked(){
		DECLARE_NAME(IsSecondaryTargetLocked);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NIsSecondaryTargetLocked, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetSecondaryTargetLockTime(){
		DECLARE_NAME(GetSecondaryTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NGetSecondaryTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT GetElapsedSecondaryTargetLockTime(){
		DECLARE_NAME(GetElapsedSecondaryTargetLockTime);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NGetElapsedSecondaryTargetLockTime, &Parms);
		return Parms.ReturnValue;
	}
	void ServerFire(){
		DECLARE_NAME(ServerFire);
		ProcessEvent(NServerFire, NULL);
	}
	void StopFire(FLOAT Value){
		DECLARE_NAME(StopFire);
		struct{
			FLOAT Value;
		} Parms;
		Parms.Value=Value;
		ProcessEvent(NStopFire, &Parms);
	}
	void ServerStopFire(){
		DECLARE_NAME(ServerStopFire);
		ProcessEvent(NServerStopFire, NULL);
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
		ProcessEvent(NGetFireStart, &Parms);
		return Parms.ReturnValue;
	}
	void ForceReload(){
		DECLARE_NAME(ForceReload);
		ProcessEvent(NForceReload, NULL);
	}
	void ServerForceReload(){
		DECLARE_NAME(ServerForceReload);
		ProcessEvent(NServerForceReload, NULL);
	}
	void ServerThrowGrenade(){
		DECLARE_NAME(ServerThrowGrenade);
		ProcessEvent(NServerThrowGrenade, NULL);
	}
	void ReleaseGrenade(){
		DECLARE_NAME(ReleaseGrenade);
		ProcessEvent(NReleaseGrenade, NULL);
	}
	void DoMeleeAttack(){
		DECLARE_NAME(DoMeleeAttack);
		ProcessEvent(NDoMeleeAttack, NULL);
	}
	UBOOL CanSwitchWeapon(){
		DECLARE_NAME(CanSwitchWeapon);
		struct{
			UBOOL ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NCanSwitchWeapon, &Parms);
		return Parms.ReturnValue;
	}
	FLOAT SwitchPriority(){
		DECLARE_NAME(SwitchPriority);
		struct{
			FLOAT ReturnValue;
		} Parms;
		Parms.ReturnValue=0;
		ProcessEvent(NSwitchPriority, &Parms);
		return Parms.ReturnValue;
	}
