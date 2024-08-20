#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

static bridgeapi_Interface GBridgeInterface;

void URtxInterface::Init()
{
	if(!GBridgeInterface.initialized)
	{
		debugf("Initializing remix bridge API");

		if(bridgeapi_initialize(&GBridgeInterface) != BRIDGEAPI_ERROR_CODE_SUCCESS || !GBridgeInterface.initialized)
			appErrorf("Failed to initialize remix bridge API");

		GBridgeInterface.RegisterDevice();
	}

	Lights.SetNoShrink(true);
	DestroyedLights.SetNoShrink(true);
}

void URtxInterface::Exit()
{
	GBridgeInterface.initialized = false;
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
				GBridgeInterface.DrawLightInstance(Light->Handle);
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

static void InitFloat3D(x86::remixapi_Float3D& Dest, const FVector& Src)
{
	Dest.x = Src.X;
	Dest.y = Src.Y;
	Dest.z = Src.Z;
}

static void InitShaping(x86::remixapi_LightInfoLightShaping& Dest, const FRtxLightShaping& Src)
{
	Dest.coneAngleDegrees = Src.ConeAngleDegrees;
	Dest.coneSoftness     = Src.ConeSoftness;
	Dest.focusExponent    = Src.FocusExponent;
}

void URtxLight::Update()
{
	DestroyHandle();

	x86::remixapi_LightInfo LightInfo;
	LightInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
	LightInfo.hash = reinterpret_cast<DWORD>(this);
	InitFloat3D(LightInfo.radiance, Color.Plane() * Radiance);

	switch(Type)
	{
	case RTXLIGHT_Sphere:
		{
			x86::remixapi_LightInfoSphereEXT SphereInfo;
			SphereInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			InitFloat3D(SphereInfo.position, Position);
			SphereInfo.radius = Sphere.Radius;
			SphereInfo.shaping_hasvalue = bUseShaping;

			if(SphereInfo.shaping_hasvalue)
				InitShaping(SphereInfo.shaping_value, Shaping);

			Handle = GBridgeInterface.CreateSphereLight(&LightInfo, &SphereInfo);
			break;
		}
	case RTXLIGHT_Rect:
		{
			x86::remixapi_LightInfoRectEXT RectInfo;
			RectInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_RECT_EXT;
			InitFloat3D(RectInfo.position, Position);
			InitFloat3D(RectInfo.xAxis, Rect.XAxis.GetNormalized());
			RectInfo.xSize = Rect.XSize;
			InitFloat3D(RectInfo.yAxis, Rect.YAxis.GetNormalized());
			RectInfo.ySize = Rect.YSize;
			InitFloat3D(RectInfo.direction, Rect.Direction.GetNormalized());

			if(RectInfo.shaping_hasvalue)
				InitShaping(RectInfo.shaping_value, Shaping);

			Handle = GBridgeInterface.CreateRectLight(&LightInfo, &RectInfo);
			break;
		}
	case RTXLIGHT_Disk:
		{
			x86::remixapi_LightInfoDiskEXT DiskInfo;
			DiskInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISK_EXT;
			InitFloat3D(DiskInfo.position, Position);
			InitFloat3D(DiskInfo.xAxis, Disk.XAxis.GetNormalized());
			DiskInfo.xRadius = Disk.XRadius;
			InitFloat3D(DiskInfo.yAxis, Disk.YAxis.GetNormalized());
			DiskInfo.yRadius = Disk.YRadius;
			InitFloat3D(DiskInfo.direction, Disk.Direction.GetNormalized());

			if(DiskInfo.shaping_hasvalue)
				InitShaping(DiskInfo.shaping_value, Shaping);

			Handle = GBridgeInterface.CreateDiskLight(&LightInfo, &DiskInfo);
			break;
		}
	case RTXLIGHT_Cylinder:
		{
			x86::remixapi_LightInfoCylinderEXT CylinderInfo;
			CylinderInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_CYLINDER_EXT;
			InitFloat3D(CylinderInfo.position, Position);
			InitFloat3D(CylinderInfo.axis, Cylinder.Axis.GetNormalized());
			CylinderInfo.radius = Cylinder.Radius;
			CylinderInfo.axisLength = Cylinder.Length;
			Handle = GBridgeInterface.CreateCylinderLight(&LightInfo, &CylinderInfo);
			break;
		}
	case RTXLIGHT_Distant:
		{
			x86::remixapi_LightInfoDistantEXT DistantInfo;
			DistantInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISTANT_EXT;
			InitFloat3D(DistantInfo.direction, Distant.Direction.GetNormalized());
			DistantInfo.angularDiameterDegrees = Distant.AngularDiameterDegrees;
			Handle = GBridgeInterface.CreateDistantLight(&LightInfo, &DistantInfo);
			break;
		}
	}
}

void URtxLight::DestroyHandle()
{
	if(Handle)
	{
		if(GBridgeInterface.initialized)
			GBridgeInterface.DestroyLight(Handle);

		Handle = 0;
	}
}
