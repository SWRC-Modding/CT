#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

static remixapi_Interface GRemixInterface;

void URtxInterface::Init()
{
	if(!GRemixInterface.Startup)
	{
		debugf("Initializing remix bridge API");

		wchar_t DllPathBuffer[MAX_PATH];
		FString DllPath = FStringTemp(appBaseDir()) * "d3d9.dll";
		HMODULE DllHandle;
		MultiByteToWideChar(CP_ACP, 0, *DllPath, -1, DllPathBuffer, ARRAY_COUNT(DllPathBuffer));

		remixapi_ErrorCode Error = remixapi_lib_loadRemixDllAndInitialize(DllPathBuffer, &GRemixInterface, &DllHandle) ;
		if(Error != REMIXAPI_ERROR_CODE_SUCCESS)
			appErrorf("Failed to initialize remix API: %i", Error);
	}

	Lights.SetNoShrink(true);
	DestroyedLights.SetNoShrink(true);
}

void URtxInterface::Exit()
{
	appMemzero(&GRemixInterface, sizeof(GRemixInterface));
}

URtxLight* URtxInterface::CreateLight(bool ForceDefaultConstructed)
{
	URtxLight* Light;

	if(!ForceDefaultConstructed && DestroyedLights.Num() > 0)
		Light = DestroyedLights.Pop();
	else
		Light = ConstructObject<URtxLight>(URtxLight::StaticClass(), this);

	Light->bShouldBeDestroyed = 0;
	Light->bEnabled = 1;
	Lights.AddItem(Light);
	return Light;
}

void URtxInterface::DestroyLight(URtxLight* Light)
{
	if(Light)
		Light->bShouldBeDestroyed = 1;
}

void URtxInterface::DestroyAllLights()
{
	DestroyedLights += Lights;
	Lights.Empty();
}

void URtxInterface::RenderLights()
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
			Light = ConstructObject<URtxLight>(URtxLight::StaticClass(), this);
			Lights[i] = Light;
		}

		if(Light->bEnabled)
		{
			if(!Light->Handle)
				Light->Update();

			if(Light->Handle)
				GRemixInterface.DrawLightInstance(Light->Handle);
		}
	}
}

void URtxInterface::execCreateLight(FFrame& Stack, void* Result)
{
	P_FINISH;
	*static_cast<URtxLight**>(Result) = CreateLight();
}

void URtxInterface::execDestroyLight(FFrame& Stack, void* Result)
{
	P_GET_OBJECT(URtxLight, Light);
	P_FINISH;
	DestroyLight(Light);
}

void URtxInterface::execDestroyAllLights(FFrame& Stack, void* Result)
{
	P_FINISH;
	DestroyAllLights();
}

void URtxInterface::execGetInstance(FFrame& Stack, void* Result)
{
	P_FINISH;

	URtxRenderDevice* RenDev = Cast<URtxRenderDevice>(GEngine->GRenDev);

	if(RenDev)
		*static_cast<URtxInterface**>(Result) = RenDev->GetRtxInterface();
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
	Dest.coneAngleDegrees = Src.ConeAngleDegrees;
	Dest.coneSoftness     = Src.ConeSoftness;
	Dest.focusExponent    = Src.FocusExponent;
}

void URtxLight::Update()
{
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
			SphereInfo.radius = Sphere.Radius;
			SphereInfo.shaping_hasvalue = bUseShaping;

			if(SphereInfo.shaping_hasvalue)
				InitShaping(SphereInfo.shaping_value, Shaping);

			LightInfo.pNext = &SphereInfo;
			GRemixInterface.CreateLight(&LightInfo, &Handle);
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

			if(RectInfo.shaping_hasvalue)
				InitShaping(RectInfo.shaping_value, Shaping);

			LightInfo.pNext = &RectInfo;
			GRemixInterface.CreateLight(&LightInfo, &Handle);
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

			if(DiskInfo.shaping_hasvalue)
				InitShaping(DiskInfo.shaping_value, Shaping);

			LightInfo.pNext = &DiskInfo;
			GRemixInterface.CreateLight(&LightInfo, &Handle);
			break;
		}
	case RTXLIGHT_Cylinder:
		{
			remixapi_LightInfoCylinderEXT CylinderInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_CYLINDER_EXT};
			InitFloat3D(CylinderInfo.position, Position);
			InitFloat3D(CylinderInfo.axis, Cylinder.Axis.GetNormalized());
			CylinderInfo.radius = Cylinder.Radius;
			CylinderInfo.axisLength = Cylinder.Length;
			LightInfo.pNext = &CylinderInfo;
			GRemixInterface.CreateLight(&LightInfo, &Handle);
			break;
		}
	case RTXLIGHT_Distant:
		{
			remixapi_LightInfoDistantEXT DistantInfo = {REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISTANT_EXT};
			InitFloat3D(DistantInfo.direction, Distant.Direction.GetNormalized());
			DistantInfo.angularDiameterDegrees = Distant.AngularDiameterDegrees;
			LightInfo.pNext = &DistantInfo;
			GRemixInterface.CreateLight(&LightInfo, &Handle);
			break;
		}
	}
}

void URtxLight::DestroyHandle()
{
	if(Handle)
	{
		if(GRemixInterface.DestroyLight)
			GRemixInterface.DestroyLight(Handle);

		Handle = NULL;
	}
}
