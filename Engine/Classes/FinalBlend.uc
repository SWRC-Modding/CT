class FinalBlend extends Modifier
	showcategories(Material)
	native;

enum EFrameBufferBlending
{
	FB_Overwrite,
	FB_Modulate,
	FB_AlphaBlend,
	FB_AlphaModulate_MightNotFogCorrectly,
	FB_Translucent,
	FB_Darken,
	FB_Brighten,
	FB_Invisible,
	FB_ShadowBlend
};

var() EFrameBufferBlending FrameBufferBlending;
var() bool ZWrite;
var() bool ZTest;
var() bool AlphaTest;
var() bool TwoSided;
var() bool ColorWriteEnable;	// Disable color writes if desired
var() bool AlphaWriteEnable;	// Disable alpha writes if desired
//scion jg -- So that alpha can be overridden as needed
var() bool OverrideAlpha;
var() bool AlphaEnable;

var() byte AlphaRef;


defaultproperties
{
     ZWrite=True
     ZTest=True
     ColorWriteEnable=True
     AlphaWriteEnable=True
}

