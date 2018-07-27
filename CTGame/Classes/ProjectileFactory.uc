// ====================================================================
//  Class:  ProjectileFactory
//
//  Spawnner for Projectile
// ====================================================================

class ProjectileFactory extends Factory
	placeable;

var(Factory) class<Projectile>	Prototype;

function PostBeginPlay()
{
	InternalPrototype = Prototype;
	Super.PostBeginPlay();	
}

defaultproperties
{
}

