class CameraTextureClient extends Info
	placeable;

var() ScriptedTexture	DestTexture;
var() name				CameraTag;
var() float				RefreshRate;
var() float				FOV;

var Actor				CameraActor;

//
//	PostBeginPlay
//

simulated function PostBeginPlay()
{
	local Actor	CameraActorIt;

	ForEach AllActors(class'Actor',CameraActorIt,CameraTag)
	{
		CameraActor = CameraActorIt;
		break;
	}

	if(DestTexture != None)
	{
		DestTexture.Client = Self;
		SetTimer(1.0 / RefreshRate,true);
		Enable('Timer');
	}
}

//
//	Timer
//

simulated function Timer()
{
	DestTexture.Revision++;
}

//
//	RenderTexture
//

simulated event RenderTexture(ScriptedTexture Tex)
{
	if(CameraActor != None)
		Tex.DrawPortal(0,0,Tex.USize,Tex.VSize,CameraActor,CameraActor.Location,CameraActor.Rotation,FOV);
}

//
//	Default properties
//


defaultproperties
{
     RefreshRate=60
     FOV=60
     bNoDelete=True
     bAlwaysRelevant=True
}

