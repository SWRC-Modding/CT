//=============================================================================
// Clone Troooper Game Info for Single-Player campaign
//
// Created: 2003 Jan 21, John Hancock
//=============================================================================
class CTGameInfo extends GameInfo
	native;

/* AcceptInventory()
Examine the passed player's inventory, and accept or discard each item
* AcceptInventory needs to gracefully handle the case of some inventory
being accepted but other inventory not being accepted (such as the default
weapon).  There are several things that can go wrong: A weapon's
AmmoType not being accepted but the weapon being accepted -- the weapon
should be killed off. Or the player's selected inventory item, active
weapon, etc. not being accepted, leaving the player weaponless or leaving
the HUD inventory rendering messed up (AcceptInventory should pick another
applicable weapon/item as current).
*/
function AcceptInventory(pawn PlayerPawn)
{
    while ( PlayerPawn.Inventory != None )
	      PlayerPawn.Inventory.Destroy();

    PlayerPawn.Weapon = None;
    PlayerPawn.SelectedItem = None;
    PlayerPawn.OldWeapon = None;
	PlayerPawn.CurrentGrenade = None;
    AddDefaultInventory( PlayerPawn );
}

// Default Add Inventory for CTGame should be to add the pawn's
// default equipment. Sub classes should override this function
// as neccesary
function AddDefaultInventory( pawn PlayerPawn )
{
	if ( PlayerPawn != none )
		PlayerPawn.AddDefaultInventory();
}


defaultproperties
{
     HUDType="CTGame.CTHUD"
     PlayerControllerClassName="CTGame.CTPlayer"
     GameReplicationInfoClass=Class'CTGame.CTGRI'
}

