//=============================================================================
// PathNode.
//=============================================================================
class PathNode extends NavigationPoint
	placeable
	native;

#exec Texture Import File=Textures\S_UsedPathnode.pcx  Name=S_UsedPathnode Mips=Off MASKED=1
#exec Texture Import File=Textures\S_SearchAndDestroy.tga Name=S_SearchAndDestroy Mips=Off MASKED=1

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	virtual INT AddMyMarker(AActor *S);
	virtual void PostEditChange();
	virtual void PostEditLoad();
	void	UpdateNodeTexture();

protected:
	UBOOL   IsSquadObjective() const;

public:

}

defaultproperties
{
     Texture=Texture'Engine.S_Pickup'
}

