//=============================================================================
// Inventory
//
// Inventory is the parent class of all actors that can be carried by other actors.  
// Inventory items are placed in the holding actor's inventory chain, a linked list 
// of inventory actors.  Each inventory class knows what pickup can spawn it (its 
// PickupClass).  When tossed out (using the DropFrom() function), inventory items 
// replace themselves with an actor of their Pickup class.
//
//=============================================================================
class Inventory extends Actor
	abstract
	native
	nativereplication;

#exec Texture Import File=Textures\Inventry.pcx Name=S_Inventory Mips=Off MASKED=1

//-----------------------------------------------------------------------------

var	 byte			InventoryGroup;     // The weapon/inventory set, 0-9.
var	 byte			GroupOffset;		// position within inventory group. (used by prevweapon and nextweapon) 				
var	 bool	 		bDisplayableInv;	// Item displayed in HUD.
var	 bool			bTossedOut;			// true if weapon/inventory was tossed out (so players can't cheat w/ weaponstay)
var() class<Pickup> PickupClass;		// what class of pickup is associated with this inventory item
var() class<Pickup>	DroppedPickupClass;	// type of pickup class to spawn when weapon is dropped if different from Pickup Class (primary weapons & grenades)
var	 travel float	Charge;				// The Current charge

//-----------------------------------------------------------------------------
// Rendering information.

// Player view rendering info.
var(FirstPerson)	 vector      PlayerViewOffset;   // Offset from view center.
var(FirstPerson)    rotator     PlayerViewPivot;    // additive rotation offset for tweaks
var() bool bDrawingFirstPerson;
var	 float		 BobDamping;		 // how much to damp view bob

// 3rd person mesh.
var actor 	ThirdPersonActor;
var class<InventoryAttachment> AttachmentClass;

//-----------------------------------------------------------------------------
// HUD graphics.

var	 Material Icon;
var	 Material StatusIcon;         // Icon used with ammo/charge/power count on HUD.
var	 localized string	 ItemName;

//-----------------------------------------------------------------------------
// Alternate mesh for MP (for example)
var Mesh						AlternateHUDArmsMesh;


// Network replication.
replication
{
	// Things the server should send to the client.
	reliable if( bNetOwner && bNetDirty && (Role==ROLE_Authority) )
		Charge,ThirdPersonActor;
}

simulated function AttachToPawn(Pawn P)
{
	local name BoneName;
	local vector BoneLocation;
	local rotator BoneRotation;

	BoneName = P.GetWeaponBoneFor(self);

	if ( ThirdPersonActor == None )
	{
		if( BoneName == '' )
		{
			ThirdPersonActor = P.Spawn(AttachmentClass,Owner);			
		}
		else
		{
			BoneLocation = P.GetBoneLocation( BoneName );
			BoneRotation = P.GetBoneRotation( BoneName );			
			ThirdPersonActor = P.Spawn(AttachmentClass,Owner, , BoneLocation, BoneRotation);
		}
	}

	InventoryAttachment(ThirdPersonActor).InitFor(self);

	if ( BoneName == '' )			
		ThirdPersonActor.SetBase(P);	
	else			
		P.AttachToBone(ThirdPersonActor,BoneName);	
}

/* UpdateRelative()
For tweaking weapon positioning.  Pass in a new relativerotation, and use the weapon editactor
properties sheet to modify the relativelocation
*/
exec function updaterelative(int pitch, int yaw, int roll)
{
	local rotator NewRot;

	NewRot.Pitch = pitch;
	NewRot.Yaw = yaw;
	NewRot.Roll = roll;

	ThirdPersonActor.SetRelativeLocation(ThirdPersonActor.Default.RelativeLocation);
	ThirdPersonActor.SetRelativeRotation(NewRot);
}

simulated function DetachFromPawn(Pawn P)
{
	if ( ThirdPersonActor != None )
	{
		ThirdPersonActor.Destroy();
		ThirdPersonActor = None;
	}
}

/* RenderOverlays() - Draw first person view of inventory
Most Inventory actors are never rendered.  The common exception is Weapon actors.  
Inventory actors may be rendered in the first person view of the player holding them
using the RenderOverlays() function.  
*/
simulated event RenderOverlays( canvas Canvas )
{
	if ( (Instigator == None) || (Instigator.Controller == None))
		return;
	SetLocation( Instigator.Location + Instigator.CalcDrawOffset(self) );
	SetRotation( Instigator.GetViewRotation() );
	Canvas.DrawActor(self, false);
}

simulated function String GetHumanReadableName()
{
	if ( ItemName == "" )
		ItemName = GetItemName(string(Class));

	return ItemName;
}

function PickupFunction(Pawn Other);

//=============================================================================
// AI inventory functions.
simulated function Weapon RecommendWeapon( out float rating )
{
	if ( inventory != None )
		return inventory.RecommendWeapon(rating);
	else
	{
		rating = -1;
		return None;
	}
}

//=============================================================================
// Inventory travelling across servers.

//
// Called after a travelling inventory item has been accepted into a level.
//
event TravelPreAccept()
{
	//Log(""$self$"-> Inventory.uc:TravelPreAccept(): call");
	Super.TravelPreAccept();
	//Log(""$self$"-> Inventory.uc:TravelPreAccept(): calling  GiveTo( Pawn(Owner) );");
	GiveTo( Pawn(Owner) );
	//Log(""$self$"-> Inventory.uc:TravelPreAccept(): called  GiveTo( Pawn(Owner) );");
	//Log(""$self$"-> Inventory.uc:TravelPreAccept(): called");
}
 
function TravelPostAccept()
{
	Super.TravelPostAccept();
	PickupFunction(Pawn(Owner));
}

//=============================================================================
// General inventory functions.

//
// Called by engine when destroyed.
//
function Destroyed()
{
	// Remove from owner's inventory.
	if( Pawn(Owner)!=None )
		Pawn(Owner).DeleteInventory( Self );
	if ( ThirdPersonActor != None )
		ThirdPersonActor.Destroy();
}

//
// Give this inventory item to a pawn.
//
function bool GiveTo( pawn Other )
{
	//Log(""$self$"-> Inventory.uc:GiveTo(pawn Other): call");

	Instigator = Other;
	if ( Other.AddInventory( Self ) )
	{
		//Log(""$self$"-> Inventory.uc:GiveTo(pawn Other): given");
		//cg moved to Weapon.uc, there are no states here!  
		// GotoState('');
		return true;
	}
	else
	{
		//Log(""$self$"-> Inventory.uc:GiveTo(pawn Other): not given");
		Destroy();
	}
	
	//Log(""$self$"-> Inventory.uc:GiveTo(pawn Other): called");

	return false;
}

//
// Function which lets existing items in a pawn's inventory
// prevent the pawn from picking something up. Return true to abort pickup
// or if item handles pickup, otherwise keep going through inventory list.
//
function bool HandlePickupQuery( pickup Item )
{
	if ( Item.InventoryType == Class )
		return true;
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

//
// Select first activatable powerup.
//
function Powerups SelectNext()
{
	if ( Inventory != None )
		return Inventory.SelectNext();
	else
		return None;
}

//
// Toss this item out.
//
function Pickup DropFrom(vector StartLocation, rotator StartRotation)
{
	local Pickup P;
	local String pickupClassName;
	local String localizedPickupMessage;

	if ( Instigator != None )
	{
		DetachFromPawn(Instigator);	
		Instigator.DeleteInventory(self);
	}	
	SetDefaultDisplayProperties();
	Instigator = None;
	StopAnimating();
	GotoState('');			// make sure weapon isn't dropped in the fire state

	if (DroppedPickupClass == None)
		P = spawn(PickupClass,,,StartLocation,StartRotation);
	else
		P = spawn(DroppedPickupClass,,,StartLocation,StartRotation);

	if ( P == None )
	{
		destroy();
		return None;
	}
	
	// ******* SBD - Nasty hack to make sure the pickup has the localized pickup message.
	
	if ( DroppedPickupClass == None )
		pickupClassName = String( PickupClass.Name );
	else
		pickupClassName = String( DroppedPickupClass.Name );
	
	if ( Right( pickupClassName, 2 ) == "MP" )
		pickupClassName = Left( pickupClassName, Len( pickupClassName ) - 2 );
	else if ( Right( pickupClassName, 9 ) == "MultiPlay" )
		pickupClassName = Left( pickupClassName, Len( pickupClassName ) - 9 );
	else if ( Right( pickupClassName, 8 ) == "MPPickup" )
		pickupClassName = Left( pickupClassName, Len( pickupClassName ) - 8 ) $ "Pickup"; 
		
	localizedPickupMessage = Localize( pickupClassName, "PickupMessage", "CTInventory", True );
	if ( Len( localizedPickupMessage ) > 0 )
		P.PickupMessage = localizedPickupMessage;

	// *******
	
	P.InitDroppedPickupFor(self);
	P.Velocity = Velocity;
	Velocity = vect(0,0,0);
	return P;
}

//=============================================================================
// Using.

function Use( float Value );

//=============================================================================
// Weapon functions.

// Find a weapon in inventory that has an Inventory Group matching F.

simulated event Weapon WeaponChange( byte F, bool bSilent )
{
	if( Inventory == None)
		return None;
	else
		return Inventory.WeaponChange( F, bSilent );
}

simulated event Weapon WeaponChangeClass( class<Weapon> WeaponClass )
{
	if( Inventory == None)
		return None;
	else
		return Inventory.WeaponChangeClass( WeaponClass );
}

// Find the previous weapon (using the Inventory group)
simulated event Weapon PrevWeapon(Weapon CurrentChoice, Weapon CurrentWeapon)
{
	if ( Inventory == None )
		return CurrentChoice;
	else
		return Inventory.PrevWeapon(CurrentChoice,CurrentWeapon);
}

// Find the next weapon (using the Inventory group)
simulated event Weapon NextWeapon(Weapon CurrentChoice, Weapon CurrentWeapon)
{
	if ( Inventory == None )
		return CurrentChoice;
	else
		return Inventory.NextWeapon(CurrentChoice,CurrentWeapon);
}

//
// Used to inform inventory when owner event occurs (for example jumping or weapon change)
//
function OwnerEvent(name EventName)
{
	if( Inventory != None )
		Inventory.OwnerEvent(EventName);
}

// used to ask inventory if it needs to affect its owners display properties
function SetOwnerDisplay()
{
	if( Inventory != None )
		Inventory.SetOwnerDisplay();
}

static function string StaticItemName()
{
	return Default.ItemName;
}



defaultproperties
{
     BobDamping=0.96
     AttachmentClass=Class'Engine.InventoryAttachment'
     DrawType=DT_None
     bHidden=True
     bOnlyOwnerSee=True
     bOnlyRelevantToOwner=True
     bReplicateMovement=False
     bOnlyDirtyReplication=True
     bTravel=True
     bClientAnim=True
     RemoteRole=ROLE_SimulatedProxy
     NetPriority=1.4
}

