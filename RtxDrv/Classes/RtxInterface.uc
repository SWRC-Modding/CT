class RtxInterface extends Object within RtxRenderDevice native transient config(RtxDrv) hidecategories(Object, None);

// uint64_t
struct noexport RtxHandle{
	var const int lo;
	var const int hi;
};

var(General)  config     float           AnchorTriangleSize;
var(General)  config     color           AnchorTriangleColor;
var(General)  config     bool            bShowAnchorTriangle;

var(Lighting)            bool            bEnableLights;
var(Lighting) editinline array<RtxLight> Lights;

var                      array<RtxLight> DestroyedLights; // Keep destroyed lights around to reduce overhead of creating/destroying short lived lights

native final function RtxLight CreateLight();
native final function DestroyLight(RtxLight Light);
native final function DestroyAllLights();

native static final function RtxInterface GetInstance();

cpptext
{
    void Init();
    void Exit();
    URtxLight* CreateLight(bool ForceDefaultConstructed = false);
    void DestroyLight(URtxLight* Light);
    void DestroyAllLights();
    void RenderLights();

    static bridgeapi_Interface BridgeInterface;
}

defaultproperties
{
	AnchorTriangleSize=512
	AnchorTriangleColor=(R=255,G=255,B=0,A=255)
	bEnableLights=True
}
