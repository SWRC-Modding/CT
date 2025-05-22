#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

static bool               GRemixApiInitialized = false;
static remixapi_Interface GRemixApi            = {0};
static HMODULE            GRemixDllHandle      = NULL;

void URtx::Init()
{
	if(!GRemixApiInitialized)
	{
		debugf("Initializing remix bridge API");

		wchar_t DllPathBuffer[MAX_PATH];
		FString DllPath = FStringTemp(appBaseDir()) * "d3d9.dll";
		MultiByteToWideChar(CP_ACP, 0, *DllPath, -1, DllPathBuffer, ARRAY_COUNT(DllPathBuffer));

		remixapi_ErrorCode Error = remixapi_lib_loadRemixDllAndInitialize(DllPathBuffer, &GRemixApi, &GRemixDllHandle) ;
		GRemixApiInitialized     = Error == REMIXAPI_ERROR_CODE_SUCCESS;

		if(!GRemixApiInitialized)
			debugf(NAME_Error, "Failed to initialize remix API: %i", Error);
	}

	Lights.SetNoShrink(true);
	DestroyedLights.SetNoShrink(true);
}

void URtx::Exit()
{
	if(GRemixApiInitialized)
	{
		remixapi_lib_shutdownAndUnloadRemixDll(&GRemixApi, GRemixDllHandle);
		GRemixApiInitialized = false;
		GRemixDllHandle = NULL;
	}
}

void URtx::SetConfigVariable(const TCHAR* Key, const TCHAR* Value)
{
	if(GRemixApiInitialized)
		GRemixApi.SetConfigVariable(Key, Value);
}

URtxLight* URtx::CreateLight(bool ForceDefaultConstructed)
{
	URtxLight* Light;

	if(!ForceDefaultConstructed && DestroyedLights.Num() > 0)
		Light = DestroyedLights.Pop();
	else
		Light = new(this) URtxLight;

	Light->bShouldBeDestroyed = 0;
	Light->bEnabled = 1;
	Lights.AddItem(Light);
	return Light;
}

void URtx::DestroyLight(URtxLight* Light)
{
	if(Light)
		Light->bShouldBeDestroyed = 1;
}

void URtx::LevelChanged(ULevel* Level)
{
	DestroyedLights += Lights;
	Lights.Empty();
	Components.Empty(); // Component actors aren't valid anymore now that the level has changed so just clear the old ones

	for(INT i = 0; i < ComponentClasses.Num(); ++i)
	{
		UClass* ComponentClass = ComponentClasses[i];
		if(ComponentClass)
		{
			ARtxComponent* Actor = static_cast<ARtxComponent*>(Level->SpawnActor(ComponentClass));
			if(Actor)
				Components.AddItem(Actor);
			else
				debugf("Failed to spawn Rtx component of class %s", ComponentClass->GetName());
		}
	}
}

void URtx::RenderLights()
{
	if(!bEnableLights)
		return;

	for(INT i = 0; i < Lights.Num(); ++i)
	{
		URtxLight* Light = Lights[i];

		if(Light && Light->bShouldBeDestroyed)
		{
			Light->DestroyHandle();
			URtxLight* Last = Lights.Pop();
			DestroyedLights.AddItem(Light);

			if(Light == Last)
				break;

			Lights[i] = Last;
			Light     = Last;
		}

		if(!Light) // NULL entry can happen if a light was added via the property window UI. In that case just create it
		{
			Light = new(this) URtxLight;
			Lights[i] = Light;
		}

		if(Light->bEnabled)
		{
			if(!Light->Handle)
				Light->Update();

			if(Light->Handle)
				GRemixApi.DrawLightInstance(Light->Handle);
		}
	}
}

void URtx::execCreateLight(FFrame& Stack, void* Result)
{
	P_FINISH;
	*static_cast<URtxLight**>(Result) = CreateLight();
}

void URtx::execDestroyLight(FFrame& Stack, void* Result)
{
	P_GET_OBJECT(URtxLight, Light);
	P_FINISH;
	DestroyLight(Light);
}

void URtx::execGetInstance(FFrame& Stack, void* Result)
{
	P_FINISH;

	URtxRenderDevice* RenDev = Cast<URtxRenderDevice>(GEngine->GRenDev);

	if(RenDev)
		*static_cast<URtx**>(Result) = RenDev->GetRtxInterface();
}

void URtx::execSetConfigVariable(FFrame& Stack, void* Result)
{
	P_GET_STR(Key);
	P_GET_STR(Value);
	P_FINISH;
	SetConfigVariable(*Key, *Value);
}

void URtxLight::execUpdate(FFrame& Stack, void* Result)
{
	P_FINISH;
	Update();
}

void URtxLight::Destroy()
{
	DestroyHandle();
	Super::Destroy();
}

static void InitFloat3D(remixapi_Float3D& Dest, const FVector& Src)
{
	Dest.x = Src.X;
	Dest.y = Src.Y;
	Dest.z = Src.Z;
}

static void InitShaping(remixapi_LightInfoLightShaping& Dest, const FRtxLightShaping& Src)
{
	InitFloat3D(Dest.direction, Src.Direction.GetNormalized());
	Dest.coneAngleDegrees = Src.ConeAngleDegrees;
	Dest.coneSoftness     = Src.ConeSoftness;
	Dest.focusExponent    = Src.FocusExponent;
}

void URtxLight::Update()
{
	if(!GRemixApiInitialized)
		return;

	DestroyHandle();

	remixapi_LightInfo LightInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO};
	LightInfo.hash = reinterpret_cast<DWORD>(this);
	InitFloat3D(LightInfo.radiance, Color.Plane() * Radiance);

	switch(Type)
	{
	case RTXLIGHT_Sphere:
		{
			remixapi_LightInfoSphereEXT SphereInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT};
			InitFloat3D(SphereInfo.position, Position);
			SphereInfo.radius           = Sphere.Radius;
			SphereInfo.shaping_hasvalue = bUseShaping;

			if(SphereInfo.shaping_hasvalue)
				InitShaping(SphereInfo.shaping_value, Shaping);

			LightInfo.pNext = &SphereInfo;
			break;
		}
	case RTXLIGHT_Rect:
		{
			remixapi_LightInfoRectEXT RectInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_RECT_EXT};
			InitFloat3D(RectInfo.position, Position);
			InitFloat3D(RectInfo.xAxis, Rect.XAxis.GetNormalized());
			RectInfo.xSize = Rect.XSize;
			InitFloat3D(RectInfo.yAxis, Rect.YAxis.GetNormalized());
			RectInfo.ySize = Rect.YSize;
			InitFloat3D(RectInfo.direction, Rect.Direction.GetNormalized());
			RectInfo.shaping_hasvalue = bUseShaping;

			if(RectInfo.shaping_hasvalue)
				InitShaping(RectInfo.shaping_value, Shaping);

			LightInfo.pNext = &RectInfo;
			break;
		}
	case RTXLIGHT_Disk:
		{
			remixapi_LightInfoDiskEXT DiskInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISK_EXT};
			InitFloat3D(DiskInfo.position, Position);
			InitFloat3D(DiskInfo.xAxis, Disk.XAxis.GetNormalized());
			DiskInfo.xRadius = Disk.XRadius;
			InitFloat3D(DiskInfo.yAxis, Disk.YAxis.GetNormalized());
			DiskInfo.yRadius = Disk.YRadius;
			InitFloat3D(DiskInfo.direction, Disk.Direction.GetNormalized());
			DiskInfo.shaping_hasvalue = bUseShaping;

			if(DiskInfo.shaping_hasvalue)
				InitShaping(DiskInfo.shaping_value, Shaping);

			LightInfo.pNext = &DiskInfo;
			break;
		}
	case RTXLIGHT_Cylinder:
		{
			remixapi_LightInfoCylinderEXT CylinderInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_CYLINDER_EXT};
			InitFloat3D(CylinderInfo.position, Position);
			InitFloat3D(CylinderInfo.axis, Cylinder.Axis.GetNormalized());
			CylinderInfo.radius     = Cylinder.Radius;
			CylinderInfo.axisLength = Cylinder.Length;
			LightInfo.pNext         = &CylinderInfo;
			break;
		}
	case RTXLIGHT_Distant:
		{
			remixapi_LightInfoDistantEXT DistantInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISTANT_EXT};
			InitFloat3D(DistantInfo.direction, Distant.Direction.GetNormalized());
			DistantInfo.angularDiameterDegrees = Distant.AngularDiameterDegrees;
			LightInfo.pNext = &DistantInfo;
			break;
		}
	}

	GRemixApi.CreateLight(&LightInfo, &Handle);
}

void URtxLight::DestroyHandle()
{
	if(GRemixApiInitialized && Handle)
	{
		GRemixApi.DestroyLight(Handle);
		Handle = NULL;
	}
}
