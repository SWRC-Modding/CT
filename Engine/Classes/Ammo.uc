//=============================================================================
// Ammo.
//=============================================================================
class Ammo extends Pickup
	abstract
	native;

#exec Texture Import File=Textures\Ammo.pcx Name=S_Ammo Mips=Off MASKED=1

var() int AmmoAmount;

function inventory SpawnCopy( Pawn Other )
{
	local Inventory Copy;

	Copy = Super.SpawnCopy(Other);
	Ammunition(Copy).AmmoAmount = AmmoAmount;
	return Copy;
}


defaultproperties
{
     RespawnTime=30
     PickupMessage="Ammo"
     PickupType=PUT_Ammo
     Texture=Texture'Engine.S_Ammo'
     CollisionRadius=84
     CollisionHeight=8
}

