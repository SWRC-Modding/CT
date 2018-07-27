// ====================================================================
//  Class:  ProjectorFactory
//  Parent: Engine.Actor
//
//  Spawnner for Projectors
// ====================================================================

class ProjectorFactory extends Factory
	placeable;

var(Factory) class<Projector>	Prototype;
var(Factory) range				ActorDrawScale;
var(Factory) RangeVector		ActorDrawScale3D;
var(Factory) RangeVector		SpawnOffset;
var(Factory) float				LifeTime;

function PostBeginPlay()
{	
	InternalPrototype = Prototype;
	Super.PostBeginPlay();	
}

function bool SetupNewActor(Actor NewActor, SpawnPoint SpawnPt)
{
	local vector NewDrawScale3D, NewOffset;
	local float NewDrawScale;

	if( NewActor != None )
	{
		NewDrawScale = RandRange(ActorDrawScale.Min,ActorDrawScale.Max);
		NewActor.SetDrawScale( NewDrawScale );		

		NewDrawScale3D.X = RandRange(ActorDrawScale3D.X.Min, ActorDrawScale3D.X.Max);
		NewDrawScale3D.Y = RandRange(ActorDrawScale3D.Y.Min, ActorDrawScale3D.Y.Max);
		NewDrawScale3D.Z = RandRange(ActorDrawScale3D.Z.Min, ActorDrawScale3D.Z.Max);
		NewActor.SetDrawScale3D( NewDrawScale3D );		

		NewOffset.X = RandRange(SpawnOffset.X.Min, SpawnOffset.X.Max);
		NewOffset.Y = RandRange(SpawnOffset.Y.Min, SpawnOffset.Y.Max);
		NewOffset.Z = RandRange(SpawnOffset.Z.Min, SpawnOffset.Z.Max);
		
		NewActor.SetLocation( NewActor.Location + NewOffset );	
		NewActor.LifeSpan = LifeTime;
		return Super.SetupNewActor(NewActor, SpawnPt);
	}

	return false;
}


defaultproperties
{
     ActorDrawScale=(Min=1,Max=1)
     ActorDrawScale3D=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
}

