class FlyPoint extends NavigationPoint
	placeable native;

#exec Texture Import File=Textures\S_FlyPoint.dds  Name=S_FlyPoint Mips=Off MASKED=1

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	virtual UBOOL CanBeValidAnchorFor(APawn *Pawn);

}

defaultproperties
{
     bNotBased=True
     Texture=Texture'Engine.S_FlyPoint'
}

