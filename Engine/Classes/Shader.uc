class Shader extends RenderedMaterial
	editinlinenew
	native;

var() editinlineuse Material Diffuse;
var() editinlineuse Material Opacity;

var() editinlineuse Material Specular;
var() editinlineuse Material SpecularityMask;

var() editinlineuse Material SelfIllumination;
var() editinlineuse Material SelfIlluminationMask;

var() editinlineuse Material Detail;
var() float DetailScale;

// Added By TimR (BumpMapping)
var() editinlineuse Material Bumpmap;
var() editinlineuse Material DiffuseEnvMap;
var(BumpMapSettings) enum EBumpMapType
{
	BMT_Static_Diffuse,
	BMT_Static_Specular,
	BMT_Static_Combined,
	BMT_Diffuse,
	BMT_Specular,
	BMT_Combined,
} BumpMapType;
var(BumpMapSettings) float BumpSize;
var(BumpMapSettings) float BumpUVScale;
var(BumpMapSettings) byte DiffuseStrength;
var(BumpMapSettings) byte DiffuseMaskStrength;
var(BumpMapSettings) enum ESpecularSharpness
{
	BSS_Matte,
	BSS_Dull,
	BSS_Polished,
	BSS_Shiny
} SpecularSharpness;
var(BumpMapSettings) byte SpecularStrength;
var(BumpMapSettings) byte SpecularMaskStrength;
var(BumpMapSettings) bool SpecularInTheDark;
var(BumpMapSettings) bool SpecularBumpX2;
var bool  UseTinting;
var color DiffuseColorAdd;
var color DiffuseColorMultiplyX2;
var color SpecularColorAdd;
var color SpecularColorMultiplyX2;
var bool  bInvertCubemap;
// end add by TimR

var() enum EOutputBlending
{
	OB_Normal,
	OB_Masked,
	OB_Modulate,
	OB_Translucent,
	OB_Invisible,
	OB_Brighten,
	OB_Darken
} OutputBlending;

var() bool TwoSided;
var() bool Wireframe;
var   bool ModulateStaticLighting2X;
var() bool PerformLightingOnSpecularPass;

function Reset()
{
	if(Diffuse != None)
		Diffuse.Reset();
	if(Opacity != None)
		Opacity.Reset();
	if(Specular != None)
		Specular.Reset();
	if(SpecularityMask != None)
		SpecularityMask.Reset();
	if(SelfIllumination != None)
		SelfIllumination.Reset();
	if(SelfIlluminationMask != None)
		SelfIlluminationMask.Reset();
	if(FallbackMaterial != None)
		FallbackMaterial.Reset();
}

function Trigger( Actor Other, Actor EventInstigator )
{
	if(Diffuse != None)
		Diffuse.Trigger(Other,EventInstigator);
	if(Opacity != None)
		Opacity.Trigger(Other,EventInstigator);
	if(Specular != None)
		Specular.Trigger(Other,EventInstigator);
	if(SpecularityMask != None)
		SpecularityMask.Trigger(Other,EventInstigator);
	if(SelfIllumination != None)
		SelfIllumination.Trigger(Other,EventInstigator);
	if(SelfIlluminationMask != None)
		SelfIlluminationMask.Trigger(Other,EventInstigator);
	if(FallbackMaterial != None)
		FallbackMaterial.Trigger(Other,EventInstigator);
}


defaultproperties
{
     DetailScale=8
     BumpSize=1
     BumpUVScale=1
     DiffuseStrength=255
     DiffuseMaskStrength=255
     SpecularSharpness=BSS_Polished
     SpecularStrength=255
     SpecularMaskStrength=255
     DiffuseColorMultiplyX2=(B=128,G=128,R=128,A=128)
     SpecularColorMultiplyX2=(B=128,G=128,R=128,A=128)
}

