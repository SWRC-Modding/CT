//=============================================================================
// Material: Abstract material class
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Material extends Object
	native
	hidecategories(Object)
	collapsecategories
	runtimestatic
	noexport;

#exec Texture Import File=Textures\DefaultTexture.dds MIPS=OFF

var() Material FallbackMaterial;

var Material DefaultMaterial;
// TimR (StateCaching)
var transient byte NumPassesCached;
var transient int CachedMaterialPasses[8];
var	transient int MaterialCacheFlags;
var	transient int CachedType;
// End TimR
var() Actor.EMaterialType TypeOfMaterial;	

var const transient bool UseFallback;	// Render device should use the fallback.
var const transient bool Validated;		// Material has been validated as renderable.


function Reset()
{
	if( FallbackMaterial != None )
		FallbackMaterial.Reset();
}

function Trigger( Actor Other, Actor EventInstigator )
{
	if( FallbackMaterial != None )
		FallbackMaterial.Trigger( Other, EventInstigator );
}


defaultproperties
{
     DefaultMaterial=Texture'Engine.DefaultTexture'
}

