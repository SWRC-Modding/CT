//=============================================================================
// Clone Troooper Pickup base class
//
// Created: 2003 Jan 27, John Hancock
//=============================================================================

class CTPickup extends Pickup
	Abstract;

var bool bUsableBySquad;


defaultproperties
{
     bAlignBottom=True
     bProjTarget=True
     bBlockZeroExtentTraces=True
     CollisionRadius=84
     CollisionHeight=8
}

