class MaterialSwitch extends Modifier
	editinlinenew
	hidecategories(Modifier)
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() transient int Current;
var() editinlineuse array<Material> Materials;

function Reset()
{
	Current = 0;
	if( Materials.Length > 0 )
		Material = Materials[0];
	else
		Material = None;

	if( Material != None )
		Material.Reset();
	if( FallbackMaterial != None )
		FallbackMaterial.Reset();
}

function Trigger( Actor Other, Actor EventInstigator )
{
	Current++;
	if( Current >= Materials.Length )
		Current = 0;

	if( Materials.Length > 0 )
		Material = Materials[Current];
	else
		Material = None;

	if( Material != None )
		Material.Trigger( Other, EventInstigator );
	if( FallbackMaterial != None )
		FallbackMaterial.Trigger( Other, EventInstigator );
}



cpptext
{
	virtual void PostEditChange();
	virtual UBOOL CheckCircularReferences( TArray<class UMaterial*>& History );

}

defaultproperties
{
}

