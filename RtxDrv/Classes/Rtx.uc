class Rtx extends Object within RtxRenderDevice native transient config(RtxDrv) hidecategories(Object, None);

var(General)  config     color           AnchorTriangleColor;
var(General)  config     bool            bDrawAnchorTriangle;
var(General)  config     bool            bCaptureMode;

var(Lighting)            bool            bEnableLights;
var(Lighting) editinline array<RtxLight> Lights;

var const editconst      array<RtxLight> DestroyedLights; // Keep destroyed lights around to reduce overhead of creating/destroying short lived lights

native final function RtxLight CreateLight();
native final function DestroyLight(RtxLight Light);
native final function DestroyAllLights();

native static final function Rtx GetInstance();

cpptext
{
    void Init();
    void Exit();
    URtxLight* CreateLight(bool ForceDefaultConstructed = false);
    void DestroyLight(URtxLight* Light);
    void DestroyAllLights();
    void RenderLights();
}

defaultproperties
{
	AnchorTriangleColor=(R=255,G=255,B=0,A=255)
	bDrawAnchorTriangle=True
	bEnableLights=True
}
