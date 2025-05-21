class Rtx extends Object native transient config(Rtx) hidecategories(Object, None);

var(Components)  config                      array<class<RtxComponent> > ComponentClasses;
var(Components)  native editconst editinline array<RtxComponent>         Components; // Must be native so the property is excluded from gc. The array is cleared in native code when the level changes.

var(General)  config     color                         AnchorTriangleColor;
var(General)  config     bool                          bDrawAnchorTriangle;
var(General)  config     bool                          bCaptureMode;
var(General)  config     bool                          bReplaceHardwareShaderMaterials;

var(Lighting) config     bool                          bEnableD3DLights;
var(Lighting) config     bool                          bEnableLights;
var(Lighting) const editinline array<RtxLight>         Lights;

var const editconst      array<RtxLight> DestroyedLights; // Keep destroyed lights around to reduce overhead of creating/destroying short lived lights

native final function RtxLight CreateLight();
native final function DestroyLight(RtxLight Light);
native final function SetConfigVariable(string Key, string Value);

native static final function Rtx GetInstance();

cpptext
{
    void Init();
    void Exit();
    void SetConfigVariable(const TCHAR* Key, const TCHAR* Value);
    URtxLight* CreateLight(bool ForceDefaultConstructed = false);
    void DestroyLight(URtxLight* Light);
    void LevelChanged(class ULevel* Level);
    void RenderLights();
}

defaultproperties
{
	AnchorTriangleColor=(R=255,G=255,B=0,A=255)
	bDrawAnchorTriangle=True
	bReplaceHardwareShaderMaterials=True
	bEnableD3DLights=False
	bEnableLights=True
	ComponentClasses(0)=class'RtxFlashlightComponent'
}
