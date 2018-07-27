//=============================================================================
// Powerup items - activatable inventory.
//=============================================================================
class Powerups extends Inventory
	abstract
	native
	nativereplication;

var travel int NumCopies;
var() bool bCanHaveMultipleCopies;     // if player can possess more than one of this
var() bool bAutoActivate;			   // automatically activated when picked up
var() bool        bActivatable;       // Whether item can be activated/deactivated (if true, must auto activate)
var	travel bool   bActive;			  // Whether item is currently activated.
var() localized String ExpireMessage; // Messages shown when powerup charge runs out

var() sound ActivateSound, DeActivateSound;

replication
{
	// Things the server should send to the client.
	reliable if( bNetOwner && bNetDirty && (Role==ROLE_Authority) )
		NumCopies, bActivatable, bActive;
}

event TravelPreAccept()
{
	Super.TravelPreAccept();
	if( bActive )
		Activate();
}

function PickupFunction(Pawn Other)
{
	Super.PickupFunction(Other);

	if (bActivatable && Other.SelectedItem==None) 
		Other.SelectedItem=self;
	if (bActivatable && bAutoActivate && Other.bAutoActivate) 
		Activate();
}

//
// Select first activatable item.
//
function Powerups SelectNext()
{
	if ( bActivatable ) 
		return self;

	if ( Inventory != None )
		return Inventory.SelectNext();
	else
		return None;
}

//
// Toggle Activation of selected Item.
// 
function Activate()
{
	if( bActivatable )
	{
		GoToState('Activated');
	}
}

//
// Advanced function which lets existing items in a pawn's inventory
// prevent the pawn from picking something up. Return true to abort pickup
// or if item handles the pickup
function bool HandlePickupQuery( Pickup Item )
{
	if (item.InventoryType == class) 
	{
		if (bCanHaveMultipleCopies) 
			NumCopies++;
		else if ( bDisplayableInv ) 
		{		
			if ( Item.Inventory != None )
				Charge = Max(Charge, Item.Inventory.Charge); 
			else
				Charge = Max(Charge, Item.InventoryType.Default.Charge);
		}
		else 
			return false;

		Item.AnnouncePickup(Pawn(Owner));
		return true;				
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

function float UseCharge(float Amount);
function FireEffect();

//
// This is called when a usable inventory item has used up it's charge.
//
function UsedUp()
{
	if ( Pawn(Owner) != None )
	{
		bActivatable = false;
		Pawn(Owner).NextItem();
		if (Pawn(Owner).SelectedItem == Self) 
		{
			Pawn(Owner).NextItem();	
			if (Pawn(Owner).SelectedItem == Self) 
				Pawn(Owner).SelectedItem=None;
		}

		Instigator.ReceiveLocalizedMessage( MessageClass, 0, None, None, Self.Class );
	}
	Owner.PlaySound(DeactivateSound,SLOT_Interface);
	Destroy();
}

static function string GetLocalString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2
	)
{
	return Default.ExpireMessage;
}


//=============================================================================
// Active state: this inventory item is armed and ready to rock!

state Activated
{
	function BeginState()
	{
		bActive = true;
	}

	function EndState()
	{
		bActive = false;
	}

	function Activate()
	{
		if ( (Pawn(Owner) != None) && Pawn(Owner).bAutoActivate 
			&& bAutoActivate && (Charge>0) )
				return;

		Global.Activate();	
	}
}


defaultproperties
{
}

