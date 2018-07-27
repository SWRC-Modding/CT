class Flashlight extends Actor
	native;

var bool					bIsOn;
var class<DynamicProjector>	ProjectorClass;
var class<Emitter>			BeamClass;
var DynamicProjector		Projector;
var Emitter					BeamEmitter;
var Material				ProjectorTexture;

function PostBeginPlay()
{
	if( ProjectorClass != None )
	{
		Projector = spawn(ProjectorClass,self);
		Projector.ProjTexture = ProjectorTexture;
		Projector.FOV = 5;
		Projector.MaxTraceDistance=10000;
		Projector.FramebufferBlendingOp = PB_Add;
		Projector.ProjTexture = None;
		Projector.SetBase( self );
		Projector.SetRelativeLocation(vect(0,0,0));
		Projector.SetRelativeRotation(rot(0,0,0));
		Projector.ExcludeActor(Owner);
	}
	if( BeamClass != None )
		BeamEmitter = spawn(BeamClass,self);
}

event TurnOn()
{
	bIsOn=true;
	bHidden=false;
	if( Projector != None )
	{		
		Projector.ProjTexture = ProjectorTexture;
		GotoState('NeedlessUpdate');
	}
}

event TurnOff()
{
	bIsOn=false;	
	bHidden=true;
	if( Projector != None )
	{
		Projector.ProjTexture = None;
		GotoState('');
	}
}

function Destroyed()
{
	if( Projector != None )
		Projector.Destroy();

	if( BeamEmitter != None )
		BeamEmitter.Destroy();

	Super.Destroyed();
}

// REVISIT: THIS IS REALLY AWFUL AND HACKY
// Unfortunately, some dynamic projectors will NOT update their rotation regardless it's base
State NeedlessUpdate
{
	function Tick( float DeltaTime )
	{	
		if( bIsOn )
			Projector.SetRotation( Rotation );
	}
}


defaultproperties
{
     DrawType=DT_StaticMesh
     bHidden=True
     bAcceptsProjectors=False
     bUnlit=True
     bBlockZeroExtentTraces=False
     bBlockNonZeroExtentTraces=False
}

