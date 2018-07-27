//=============================================================================
// ShadowProjector.
//=============================================================================

class ShadowProjector extends Projector native;

var() Actor					ShadowActor;
var() vector				LightDirection;
var() float					LightDistance;
var   float					ShadowIntensity;
var() bool					RootMotion;
var() bool					bBlobShadow;
var() bool					bShadowActive;
var() bool                  bUseLightAverage;
var transient ShadowBitmapMaterial	ShadowTexture;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

//
//	PostBeginPlay
//

simulated event PostBeginPlay()
{
	Super(Actor).PostBeginPlay();
}

//
//	Destroyed
//

simulated event Destroyed()
{
	if(ShadowTexture != None)
	{
		ShadowTexture.ShadowActor = None;
		
		//if(!ShadowTexture.Invalid)
		//	Level.ObjectPool.FreeObject(ShadowTexture);

		ShadowTexture = None;
	}

	Super.Destroyed();
}

//
//	InitShadow
//

simulated function InitShadow()
{
	if(ShadowActor != None)
	{
		// The object pool does not save.
		//ShadowTexture = ShadowBitmapMaterial(Level.ObjectPool.AllocateObject(class'ShadowBitmapMaterial'));
		ShadowTexture = new class'ShadowBitmapMaterial';
		if(ShadowTexture != None)
		{
			UpdateShadow();
			Enable('Tick');			
		}
		else
		{
			Disable('Tick');
			Log(Name$".InitShadow: Failed to allocate texture");
		}
	}
	else
	{
		Disable('Tick');
		Log(Name$".InitShadow: No actor");
	}
}

//
//	UpdateShadow
//

simulated native event UpdateShadow();

//
//	Tick
//

simulated function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);
	if( Level.ShadowsEnabled )
		UpdateShadow();
}

//
//	Default properties
//



cpptext
{
	virtual UMaterial* GetProjTexture();

}

defaultproperties
{
     ShadowIntensity=1
     bShadowActive=True
     bUseLightAverage=True
     bProjectActor=False
     bClipBSP=True
     bGradient=True
     bProjectOnAlpha=True
     bProjectOnParallelBSP=True
     bDynamicAttach=True
     bShadow=True
     bStatic=False
     bOwnerNoSee=True
     CullDistance=1200
}

