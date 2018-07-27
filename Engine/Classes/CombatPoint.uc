class CombatPoint extends NavigationPoint
	placeable native;

#exec Texture Import File=Textures\S_CombatPoint.dds  Name=S_CombatPoint Mips=Off MASKED=1

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	virtual UBOOL IsAvailable() const { return (referenceCount < 2); }

}

defaultproperties
{
     bCombatNode=True
     Texture=Texture'Engine.S_CombatPoint'
}

