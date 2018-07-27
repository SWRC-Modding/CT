//=============================================================================
// ScriptedTexture: A scriptable Unreal texture
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class ScriptedTexture extends BitmapMaterial
	native;

var const transient int			RenderTarget;
var const transient Viewport	RenderViewport;

var Actor				Client;
var transient int		Revision;
var transient const int	OldRevision;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

native final function SetSize(int Width,int Height);

native final function DrawText(int StartX,int StartY,coerce string Text,Font Font,Color Color);
native final function TextSize(coerce string Text,Font Font,out int Width,out int Height);
native final function DrawTile(float X,float Y,float XL,float YL,float U,float V,float UL,float VL,Material Material,Color Color);

native final function DrawPortal(int X,int Y,int Width,int Height,Actor CamActor,vector CamLocation,rotator CamRotation,optional int FOV,optional bool ClearZ);



cpptext
{
	void Render(FRenderInterface* RI);

	virtual UBitmapMaterial* Get(FTime Time,UViewport* Viewport);
	virtual FBaseTexture* GetRenderInterface();
	virtual void Destroy();
	virtual void PostEditChange();

}

defaultproperties
{
}

