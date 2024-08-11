#include "RtxDrvPrivate.h"
#include "RtxRenderDevice.h"

bridgeapi_Interface URtxInterface::BridgeInterface;

void URtxInterface::Init()
{
	if(!BridgeInterface.initialized)
	{
		debugf("Initializing remix bridge API");

		if(bridgeapi_initialize(&BridgeInterface) != BRIDGEAPI_ERROR_CODE_SUCCESS || !BridgeInterface.initialized)
			appErrorf("Failed to initialize remix bridge API");

		BridgeInterface.RegisterDevice();
	}
}

void URtxInterface::Exit()
{
	BridgeInterface.initialized = false;
}

URtxLight* URtxInterface::CreateLight(bool ForceDefaultConstructed)
{
	URtxLight* Light;

	if(!ForceDefaultConstructed && DestroyedLights.Num() > 0)
		Light = DestroyedLights.Pop();
	else
		Light = ConstructObject<URtxLight>(URtxLight::StaticClass(), this);

	Light->bEnabled = 1;
	Lights.AddItem(Light);
	return Light;
}

void URtxInterface::DestroyLight(URtxLight* Light)
{
	if(Light)
	{
		Lights.RemoveItem(Light);
		DestroyedLights.AddItem(Light);
	}
}

void URtxInterface::DestroyAllLights()
{
	DestroyedLights += Lights;
	Lights.Empty();
}

void URtxInterface::RenderLights()
{
	for(INT i = 0; i < Lights.Num(); ++i)
	{
		URtxLight* Light = Lights[i];

		if(!Light) // NULL entry can happen if a light was added via the property window UI. In that case just create it
		{
			Light = ConstructObject<URtxLight>(URtxLight::StaticClass(), this);
			Lights[i] = Light;

			TObjectIterator<UViewport> It;
			if(It && It->Actor)
				Lights[i]->Position = It->Actor->Location;
		}

		if(Light->bEnabled)
		{
			if(!Light->Handle)
				Light->Update();

			if(Light->Handle)
				BridgeInterface.DrawLightInstance(Light->Handle);
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
	if(Handle)
	{
		if(URtxInterface::BridgeInterface.initialized)
			URtxInterface::BridgeInterface.DestroyLight(Handle);

		Handle = 0;
	}

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
	const bridgeapi_Interface& Bridge = URtxInterface::BridgeInterface;

	if(Handle)
	{
		Bridge.DestroyLight(Handle);
		Handle = 0;
	}

	x86::remixapi_LightInfo LightInfo;
	LightInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
	LightInfo.hash = reinterpret_cast<DWORD>(this);
	InitFloat3D(LightInfo.radiance, Radiance);

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

			Handle = Bridge.CreateSphereLight(&LightInfo, &SphereInfo);
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

			Handle = Bridge.CreateRectLight(&LightInfo, &RectInfo);
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

			Handle = Bridge.CreateDiskLight(&LightInfo, &DiskInfo);
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
			Handle = Bridge.CreateCylinderLight(&LightInfo, &CylinderInfo);
			break;
		}
	case RTXLIGHT_Distant:
		{
			x86::remixapi_LightInfoDistantEXT DistantInfo;
			DistantInfo.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_DISTANT_EXT;
			InitFloat3D(DistantInfo.direction, Distant.Direction.GetNormalized());
			DistantInfo.angularDiameterDegrees = Distant.AngularDiameterDegrees;
			Handle = Bridge.CreateDistantLight(&LightInfo, &DistantInfo);
			break;
		}
	}
}
