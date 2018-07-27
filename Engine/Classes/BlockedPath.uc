//=============================================================================
// BlockedPath.
// 
//=============================================================================
class BlockedPath extends NavigationPoint
	placeable
	native;

var(Collision) Actor MyBlocker;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

#exec Texture Import File=..\Engine\Textures\S_BlockedPath.dds  Name=S_BlockedPath Mips=Off MASKED=1

function Trigger( actor Other, pawn EventInstigator )
{
	SetBlocked(!bBlocked);
	Log("BlockedPath::Trigger: Toggled blocked status of "$self$" now bBlocked = "$bBlocked);
}



cpptext
{
	virtual void PrePath();
	virtual void PostPath();

}

defaultproperties
{
     bBlocked=True
     bStatic=False
     Texture=Texture'Engine.S_BlockedPath'
}

