//=============================================================================
// JumpDest.
// specifies positions that can be reached with greater than normal jump
// forced paths will check for greater than normal jump capability
// NOTE these have NO relation to JumpPads
//=============================================================================
class JumpDest extends NavigationPoint
	placeable
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

function Trigger( Actor Other, Pawn EventInstigator )
{
	bEnabled = !bEnabled;
}



cpptext
{
	virtual UBOOL CanBeValidAnchorFor(class APawn *Pawn);

}

defaultproperties
{
     bNotBased=True
}

