class ProjectorMultiMaterial extends RenderedMaterial
	native
	noteditinlinenew;
	
	
/* scion ======================================================================
 * Author: sz
 * ============================================================================
 *
 *	Container class for batching projectors so that multiple projectors can be 
 *	rendered together in one pass using a vertex/pixel shader.
 * ============================================================================
 */

#exec Texture Import file=Textures\White.tga Name=WhiteOpaque Mips=Off UCLAMPMODE=CLAMP VCLAMPMODE=CLAMP DXT=1
	
struct native ProjectorRenderEntry
{
	var Material	ProjectedTexture;			// texture from projector
	var Matrix		Matrix;						// transform for projector
	var Vector		Direction;					// world space projector direction
	// Location of the projector
	var Vector		Origin;
	// 1 / the max trace distance
	var float		InvMaxTraceDist;
	// Whether to project on back facing polys or not
	var bool		bProjectOnBackfaces;
};

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
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var transient array<ProjectorRenderEntry>	RenderEntries;
var transient int							RemainingEntries;		// Remaining entries still to render
var transient Material						BaseMaterial;
var transient byte							FrameBufferBlending;	// see Projector.EProjectorBlending

var Texture		DefaultOpacity;
var HardwareShader	AddShaders[4];
var HardwareShader	BlendShaders[4];
var HardwareShader	ModShaders[4];



cpptext
{
	inline UHardwareShader* GetHardwareShader( int NumProjectors )
	{
		switch( FrameBufferBlending )
		{
		case PB_Add:
			return AddShaders[ NumProjectors - 1 ];
		case PB_AlphaBlend:
			return BlendShaders[ NumProjectors - 1 ];
		case PB_Modulate:
		case PB_Modulate2X:
			return ModShaders[ NumProjectors - 1 ];
		}
		return NULL;
	}

}

defaultproperties
{
     DefaultOpacity=Texture'Engine.WhiteOpaque'
     AddShaders(0)=HardwareShader'HardwareShaders.Projectors.Add1'
     AddShaders(1)=HardwareShader'HardwareShaders.Projectors.Add2'
     AddShaders(2)=HardwareShader'HardwareShaders.Projectors.Add3'
     AddShaders(3)=HardwareShader'HardwareShaders.Projectors.Add4'
     BlendShaders(0)=HardwareShader'HardwareShaders.Projectors.Blend1'
     BlendShaders(1)=HardwareShader'HardwareShaders.Projectors.Blend2'
     BlendShaders(2)=HardwareShader'HardwareShaders.Projectors.Blend3'
     BlendShaders(3)=HardwareShader'HardwareShaders.Projectors.Blend4'
     ModShaders(0)=HardwareShader'HardwareShaders.Projectors.Mod1'
     ModShaders(1)=HardwareShader'HardwareShaders.Projectors.Mod2'
     ModShaders(2)=HardwareShader'HardwareShaders.Projectors.Mod3'
     ModShaders(3)=HardwareShader'HardwareShaders.Projectors.Mod4'
}

