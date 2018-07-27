// ====================================================================
//  Class:  ActorSpawner
//  Parent: Engine.Actor
//
//  ActorSpawners generate a series of actors 
// ====================================================================

class ActorSpawner extends Actor
	abstract
	hidecategories(Collision,Force,Karma,LightColor,Lighting,Sound);


enum ActorSpawnerType
{
	AST_GenericActor,
	AST_Projector

/* These to follow:

	AST_Rumble,
	AST_CameraShake */
};


struct SpawnInfo 
{
	var() ActorSpawnerType  TypeOfSpawn;
	var() class<Actor>      ActorToSpawn;
	var() class<Projector>	ProjectorToSpawn;
};

var() Array<SpawnInfo>   StuffToSpawn;

function PostNetBeginPlay()
{
	local int i;
	// Iterate through our array of guys and spawn stuff
	for (i = 0; i < StuffToSpawn.Length; i++)
	{
		if (StuffToSpawn[i].TypeOfSpawn == AST_GenericActor && StuffToSpawn[i].ActorToSpawn != None)
		{
			Spawn(StuffToSpawn[i].ActorToSpawn,,,Location,Rotation);
		}
		else if (StuffToSpawn[i].TypeOfSpawn == AST_Projector && StuffToSpawn[i].ProjectorToSpawn != None)
		{
			Spawn(StuffToSpawn[i].ProjectorToSpawn,,,Location,rotator(-vector(Rotation)));
		}
	}
	
	// Note, at some later date, we 
	Destroy();
}
	

defaultproperties
{
     bHidden=True
     bBlockZeroExtentTraces=False
     bBlockNonZeroExtentTraces=False
}

