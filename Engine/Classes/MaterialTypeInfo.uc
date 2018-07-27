//=============================================================================
// Material type info: A class which contains a collection of mappings from 
// material type to resultant effects (actors which are spawned when a particular
// material type is dealt with)
//=============================================================================
class MaterialTypeInfo extends Object
	dependsOn(Actor)
	hidecategories(Object)
	abstract;


struct MaterialMapInfo
{
	var() Actor.EMaterialType		TypeOfMaterial;
	var() class<Emitter>			Emitter;
	var() pconly class<HitDecal>	HitDecal;
	var() class<ActorSpawner>		ActorSpawner;	
};

var() Array<MaterialMapInfo> MaterialMap;
var() MaterialMapInfo		 DefaultMaterialEffect;
var globalconfig bool        bSpawnHitDecals;


static function SpawnEffectsFor( Actor Instigator, Actor.EMaterialType type, vector Location, vector Direction )
{
	local int i;
	local MaterialMapInfo HitMaterial;

	if (type == MT_NoEffectSpawned)
	{
		return;
	}

	HitMaterial = default.DefaultMaterialEffect;
	
	for( i = 0; i < default.MaterialMap.Length; i++ )
	{
		if (default.MaterialMap[i].TypeOfMaterial == type)
		{
			HitMaterial = default.MaterialMap[i];
			break;
		}
	}

	if( HitMaterial.Emitter != None )
		Instigator.Spawn(HitMaterial.Emitter,,,Location,rotator(Direction));
	if( default.bSpawnHitDecals && HitMaterial.HitDecal != None )
		Instigator.Spawn(HitMaterial.HitDecal,,,Location,rotator(-Direction));
	if( HitMaterial.ActorSpawner != None )
		Instigator.Spawn(HitMaterial.ActorSpawner,,,Location,rotator(-Direction));
}

static function class<Emitter> DetermineEffectClass( Actor Instigator, Actor.EMaterialType type, vector Location, vector Direction )
{
	local int i;
	local MaterialMapInfo HitMaterial;

	HitMaterial = default.DefaultMaterialEffect;
	
	for( i = 0; i < default.MaterialMap.Length; i++ )
	{
		if (default.MaterialMap[i].TypeOfMaterial == type)
		{
			HitMaterial = default.MaterialMap[i];
			break;
		}
	}

	if( HitMaterial.Emitter != None )
		return HitMaterial.Emitter;
}


defaultproperties
{
     bSpawnHitDecals=True
}

