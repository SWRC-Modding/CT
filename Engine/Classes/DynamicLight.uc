//=============================================================================
// The dynamic light class.
// Used to create lights that need to move or should be treated as moveable
//=============================================================================
class DynamicLight extends Light
	native
	placeable;


defaultproperties
{
     bStatic=False
     bNoDelete=False
     bDynamicLight=True
     bMovable=True
}

