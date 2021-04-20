/*=============================================================================
	UnEngine.h: Unreal engine definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unreal engine.
-----------------------------------------------------------------------------*/

class ENGINE_API UEngine : public USubsystem{
	DECLARE_ABSTRACT_CLASS(UEngine,USubsystem,CLASS_Config|CLASS_Transient,Engine)

	//Subsystems
	class UClass* AudioDevice;
	class UClass* Console;
	class UClass* DefaultMenu;
	class UClass* DefaultPlayerMenu;
	class UClass* NetworkDevice;
	class UClass* Language;
	class UPrimitive* Cylinder;
	class UClient* Client;
	class UAudioSubsystem* Audio;
	class URenderDevice* GRenDev;

	//Variables
	INT CacheSizeMegs;
	BITFIELD UseSound:1;
	BITFIELD UpdateDynamicLightmaps:1;
	BITFIELD UseStaticMeshBatching:1;
	FLOAT CurrentTickRate;
	class UMaterial* HBumpShaderMacros;
	INT ActiveControllerId;
	FColor C_WorldBox;
	FColor C_GroundPlane;
	FColor C_GroundHighlight;
	FColor C_BrushWire;
	FColor C_Pivot;
	FColor C_Select;
	FColor C_Current;
	FColor C_AddWire;
	FColor C_SubtractWire;
	FColor C_GreyWire;
	FColor C_BrushVertex;
	FColor C_BrushSnap;
	FColor C_Invalid;
	FColor C_ActorWire;
	FColor C_ActorHiWire;
	FColor C_Black;
	FColor C_White;
	FColor C_Mask;
	FColor C_SemiSolidWire;
	FColor C_NonSolidWire;
	FColor C_WireBackground;
	FColor C_WireGridAxis;
	FColor C_ActorArrow;
	FColor C_ScaleBox;
	FColor C_ScaleBoxHi;
	FColor C_ZoneWire;
	FColor C_Mover;
	FColor C_OrthoBackground;
	FColor C_StaticMesh;
	FColor C_VolumeBrush;
	FColor C_ConstraintLine;
	FColor C_AnimMesh;
	FColor C_TerrainWire;
	FColor C_KActor;
	FColor C_Ragdoll;
	FColor C_Pawn;
	FColor C_Prop;
	FColor C_AnimProp;
	FColor C_KarmaProp;
	FColor C_Pickup;

	//Constructors
	UEngine();
	void StaticConstructor();

	//Overrides
	virtual void Destroy();
	virtual void Serialize(FArchive&);

	//Virtual Functions
	virtual void Init();
	virtual void SetDemoParameters(float, float, int);
	virtual void Flush(int);
	virtual void UpdateGamma();
	virtual void RestoreGamma();
	virtual int Key(UViewport*, EInputKey, unsigned short);
	virtual int InputEvent(UViewport*, EInputKey, EInputAction, float);
	virtual void Draw(UViewport*, int, unsigned char*, int*);
	virtual void MouseDelta(UViewport*, unsigned long, float, float);
	virtual void MousePosition(UViewport*, unsigned long, float, float);
	virtual void MouseWheel(UViewport*, unsigned long, int);
	virtual void Click(UViewport*, unsigned long, float, float);
	virtual void UnClick(UViewport*, unsigned long, int, int);
	virtual void SetClientTravel(UPlayer*, char const*, int, ETravelType);
	virtual int ChallengeResponse(int);
	virtual float GetMaxTickRate();
	virtual void SetProgress(char const*, char const*, float);
	virtual void DisplayError(char const*);
	virtual void PlayMovie(char const*, bool, bool, bool, bool, bool);
	virtual void FullscreenToggled(bool);
	virtual void InitAudio();
	virtual void edSetClickLocation(FVector&);
	virtual void edDrawAxisIndicator(class FSceneNode*);
	virtual int edcamMode(UViewport*);
	virtual int edcamTerrainBrush();
	virtual int edcamMouseControl(UViewport *);
	virtual void EdCallback(unsigned long, int, unsigned long);
	virtual bool CanDisplayError() const;
};

/*-----------------------------------------------------------------------------
	UGlobalTempObjects.
-----------------------------------------------------------------------------*/

class UGlobalTempObjects : public UObject{
	DECLARE_CLASS(UGlobalTempObjects,UObject,CLASS_Transient,Engine);
	TArray<UObject**> GlobalObjectPtrs;

	// Constructor
	UGlobalTempObjects(){ AddToRoot(); }

	// UGlobalTempObjects interface
	void AddGlobalObject(UObject** InObjectPtr){ GlobalObjectPtrs.AddItem(InObjectPtr); }

	// UObject interface
	void Serialize(FArchive& Ar){
		guardFunc;

		Super::Serialize(Ar);

		if(!(Ar.IsLoading() || Ar.IsSaving())){
			for(INT i = 0; i < GlobalObjectPtrs.Num(); ++i)
				*(GlobalObjectPtrs[i]) = NULL;

			GlobalObjectPtrs.Empty();
		}

		unguard;
	}

	void Destroy(){
		Super::Destroy();

		for(INT i = 0; i < GlobalObjectPtrs.Num(); ++i)
			*(GlobalObjectPtrs[i]) = NULL;

		GlobalObjectPtrs.Empty();
	}
};

/*-----------------------------------------------------------------------------
	UCubemapManager.
-----------------------------------------------------------------------------*/

class ENGINE_API UCubemapManager : public UObject{
	DECLARE_CLASS(UCubemapManager, UObject, 0, Engine);

	BITFIELD  bEnabled:1;
	AActor*   Actor;
	FLOAT     BumpSize;
	FVector   CameraLocation;
	FRotator  CameraRotation;
	UTexture* StaticDiffuse;
	UTexture* StaticSpecularMatte;
	UTexture* StaticSpecularDull;
	UTexture* StaticSpecularPolished;
	UTexture* StaticSpecularShiny;

	UCubemapManager();

	// UObject interface
	virtual void Destroy();

	// UCubemapManager interface
	void Init();
	void Enable();
	bool IsEnabled() const;
	void Disable();
	void CleanupObjects();
	FLOAT GetBumpSize() const;
	void SetBumpSize(FLOAT BumpSize);
	const FVector& GetCameraLocation();
	const FRotator& GetCameraRotation();
	void SetCamera(const FVector& Location, const FRotator& Rotation);
	AActor* GetCurrentActor();
	void SetCurrentActor(AActor* Actor, TList<FDynamicLight*>*);
	UTexture* GetEnvLightmap(EBumpMapType BumpmapType, UShader* Shader, FMatrix**, bool&);
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
