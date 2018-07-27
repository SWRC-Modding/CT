class Projector extends Actor
	placeable
	native;

#exec Texture Import File=Textures\Proj_IconMasked.pcx Name=Proj_Icon Mips=Off MASKED=1
#exec Texture Import file=Textures\GRADIENT_Fade.dds Name=GRADIENT_Fade Mips=Off UCLAMPMODE=CLAMP VCLAMPMODE=CLAMP 

// NathanM: No one is using this texture
// #exec Texture Import file=Textures\GRADIENT_Clip.dds Name=GRADIENT_Clip Mips=Off UCLAMPMODE=CLAMP VCLAMPMODE=CLAMP 


// Projector blending operation.

enum EProjectorBlending
{
	PB_None,
	PB_Modulate,
	PB_AlphaBlend,
	PB_Add,
	PB_Modulate2X
};

var() EProjectorBlending	MaterialBlendingOp,		// The blending operation between the material being projected onto and ProjTexture.
							FrameBufferBlendingOp;	// The blending operation between the framebuffer and the result of the base material blend.

// Projector properties.

var() Material		ProjTexture;
var() int			FOV;
var() int			MaxTraceDistance;
var() bool			bProjectBSP;
var() bool			bProjectTerrain;
var() bool			bProjectStaticMesh;
var() bool			bProjectParticles;
var() bool			bProjectActor;
var() bool			bLevelStatic;
var() bool			bClipBSP;
var() bool			bClipStaticMesh;
var() bool			bProjectOnUnlit;
var() bool			bGradient;
var() bool			bProjectOnBackfaces;
var() bool			bProjectOnAlpha;
var() bool			bProjectOnParallelBSP;
var() bool			bDynamicAttach;
var() name			ProjectTag;
var() bool			bShadow;            // this projector is a dynamic shadow

const EXCLUSION_LIST_SIZE = 4;
var() Actor			ExclusionList[EXCLUSION_LIST_SIZE];


// Internal state.

var const transient plane FrustumPlanes[6];
var const transient vector FrustumVertices[8];
var const transient Box Box;
var const transient ProjectorRenderInfoPtr RenderInfo;
var Texture GradientTexture;
var transient Matrix GradientMatrix;
var transient Matrix Matrix;
var transient Vector OldLocation;

// Native interface.

simulated native function AttachProjector();
simulated native function DetachProjector(optional bool Force);
simulated native function AbandonProjector(optional float Lifetime);

simulated native function AttachActor( Actor A );
simulated native function DetachActor( Actor A );
 
simulated native function bool ExcludeActor( Actor A );
simulated native function bool IsActorExcluded( Actor A );

simulated event PostBeginPlay()
{
	AttachProjector();
	if( bLevelStatic )
	{
		AbandonProjector();
		Destroy();
	}
	if( bProjectActor )
		SetCollision(True, False, False);
}

simulated event Touch( Actor Other )
{
	if(Other==None)
		return;

	// TimR: Don't project onto owner
  	if( Other == Owner || (Owner != None && Other.Owner == Owner) )
  		return;

	// Changed by Demiurge (ProjectorOptimize)
	// Changed so that we don't AttachActor() for static projector/static object combinations
	if( Other.bAcceptsProjectors 
		&& (ProjectTag=='' || Other.Tag==ProjectTag) 
		&& (bProjectStaticMesh || Other.StaticMesh == None) 
		&& !(Other.bStatic && /*bStatic &&*/ Other.StaticMesh!=None)  // Removed to prevent shadows from drawing twice
        && (!bShadow || Other.bReceiveDynamicShadows) )
		AttachActor(Other);
}

simulated event Untouch( Actor Other )
{
	DetachActor(Other);
}

simulated native function Material GetProjTexture();


// The following function used as part of the editor for search functionality
simulated function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ProjectTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}




defaultproperties
{
     FrameBufferBlendingOp=PB_Modulate
     FOV=50
     MaxTraceDistance=1000
     bProjectBSP=True
     bProjectTerrain=True
     bProjectStaticMesh=True
     bProjectParticles=True
     bProjectActor=True
     GradientTexture=Texture'Engine.GRADIENT_Fade'
     bStatic=True
     bHidden=True
     bUseHWOcclusionTests=True
     RemoteRole=ROLE_None
     Texture=Texture'Engine.Proj_Icon'
     bDirectional=True
}

