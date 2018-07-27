//=============================================================================
// Perch.uc
//
// Created: August 17, 2003 Nathan Martz
//=============================================================================
class Perch extends NavigationPoint
	placeable abstract native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() bool bCeiling;

var pconly StaticMesh WallMesh;
var pconly StaticMesh CeilingMesh;

function Trigger( Actor Other, Pawn EventInstigator )
{
	bEnabled = !bEnabled;
}



cpptext
{
	virtual UBOOL CanBeValidAnchorFor(APawn *Pawn);

}

defaultproperties
{
     RadiusOverride=75
     bNotBased=True
     IconScale=5
     bDirectional=True
}

