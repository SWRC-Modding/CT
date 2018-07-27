// ====================================================================
//  Class:  FXFactory
//  Parent: Engine.Actor
//
//  Spawnner for Emitters
// ====================================================================

class FXFactory extends Factory
	placeable;

#exec Texture Import File=Textures\S_FXFactory.pcx  Name=S_FXFactory Mips=Off MASKED=1


var(Factory) class<Emitter>	Prototype;
var(Factory) range			EmitterDrawScale;
var(Factory) RangeVector	EmitterDrawScale3D;
var(Factory) RangeVector	SpawnOffset;

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
		NewDrawScale = RandRange(EmitterDrawScale.Min, EmitterDrawScale.Max);
		NewActor.SetDrawScale( NewDrawScale );		

		NewDrawScale3D.X = RandRange(EmitterDrawScale3D.X.Min, EmitterDrawScale3D.X.Max);
		NewDrawScale3D.Y = RandRange(EmitterDrawScale3D.Y.Min, EmitterDrawScale3D.Y.Max);
		NewDrawScale3D.Z = RandRange(EmitterDrawScale3D.Z.Min, EmitterDrawScale3D.Z.Max);
		NewActor.SetDrawScale3D( NewDrawScale3D );		

		NewOffset.X = RandRange(SpawnOffset.X.Min, SpawnOffset.X.Max);
		NewOffset.Y = RandRange(SpawnOffset.Y.Min, SpawnOffset.Y.Max);
		NewOffset.Z = RandRange(SpawnOffset.Z.Min, SpawnOffset.Z.Max);
		
		NewActor.SetLocation( NewActor.Location + NewOffset );		
		return Super.SetupNewActor(NewActor, SpawnPt);
	}

	return false;
}


defaultproperties
{
     EmitterDrawScale=(Min=1,Max=1)
     EmitterDrawScale3D=(X=(Min=1,Max=1),Y=(Min=1,Max=1),Z=(Min=1,Max=1))
     Texture=Texture'CTGame.S_FXFactory'
}

