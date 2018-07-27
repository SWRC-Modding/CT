//=============================================================================
// EditorEngine: The UnrealEd subsystem.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class EditorEngine extends Engine
	native
	noexport
	transient;

#exec Texture Import File=Textures\Bad.pcx
#exec Texture Import File=Textures\BadHighlight.pcx
#exec Texture Import File=Textures\Bkgnd.pcx
#exec Texture Import File=Textures\BkgndHi.pcx
#exec Texture Import File=Textures\MaterialArrow.pcx MASKED=1
#exec Texture Import File=Textures\MaterialBackdrop.pcx

#exec NEW StaticMesh File="models\TexPropCube.Ase" Name="TexPropCube"
#exec NEW StaticMesh File="models\TexPropSphere.Ase" Name="TexPropSphere"

// Objects.
var const level       Level;
var const model       TempModel;
var const texture     CurrentTexture;
var const staticmesh  CurrentStaticMesh;
var const mesh		  CurrentMesh;
var const class       CurrentClass;
var const class       LastClass;
var const transbuffer Trans;
var const textbuffer  Results;
var const int         Pad[8];

// Textures.
var const texture Bad, Bkgnd, BkgndHi, BadHighlight, MaterialArrow, MaterialBackdrop;

// Used in UnrealEd for showing materials
var staticmesh	TexPropCube;
var staticmesh	TexPropSphere;

// Toggles.
var const bool bFastRebuild, bBootstrapping;

// Other variables.
var const config int AutoSaveIndex;
var const int AutoSaveCount, Mode, TerrainEditBrush, ClickFlags;
var const bool MightNeedSave;
var const config float MovementSpeed;
var const package PackageContext;
var const vector AddLocation;
var const plane AddPlane;

// Misc.
var const array<Object> Tools;
var const class BrowseClass;

// Grid.
var const int ConstraintsVtbl;
var(Grid) config bool GridEnabled;
var(Grid) config bool SnapVertices;
var(Grid) config float SnapDistance;
var(Grid) config vector GridSize;

// Rotation grid.
var(RotationGrid) config bool RotGridEnabled;
var(RotationGrid) config rotator RotGridSize;

// Advanced.
var(Advanced) config bool UseSizingBox;
var(Advanced) config bool UseAxisIndicator;
var(Advanced) config float FovAngleDegrees;
var(Advanced) config bool GodMode;
var(Advanced) config bool AutoSave;
var(Advanced) config byte AutosaveTimeMinutes;
var(Advanced) config string GameCommandLine;
var(Advanced) config array<string> EditPackages;
var(Advanced) config array<string> NativePackages;
var(Advanced) config bool AlwaysShowTerrain;
// changed by Demiurge (3DDragRot)
var(Advanced) config bool UseOldInterface; 
var(Advanced) config bool DontUseArcball;
var(Advanced) config bool HideOrthoAxes;
// end Demiurge (3DDragRot)
var(Advanced) config bool MayaStyleMovement;


defaultproperties
{
     Bad=Texture'Editor.Bad'
     Bkgnd=Texture'Editor.Bkgnd'
     BkgndHi=Texture'Editor.BkgndHi'
     BadHighlight=Texture'Editor.BadHighlight'
     MaterialArrow=Texture'Editor.MaterialArrow'
     MaterialBackdrop=Texture'Editor.MaterialBackdrop'
     TexPropCube=StaticMesh'Editor.TexPropCube'
     TexPropSphere=StaticMesh'Editor.TexPropSphere'
     AutoSaveIndex=6
     MovementSpeed=4
     GridEnabled=True
     SnapDistance=10
     GridSize=(X=16,Y=16,Z=16)
     RotGridEnabled=True
     RotGridSize=(Pitch=1024,Yaw=1024,Roll=1024)
     UseAxisIndicator=True
     FovAngleDegrees=90
     GodMode=True
     AutoSave=True
     AutoSaveTimeMinutes=5
     GameCommandLine="-windowed"
     EditPackages(0)="Core"
     EditPackages(1)="Engine"
     EditPackages(2)="Editor"
     EditPackages(3)="UWindow"
     EditPackages(4)="UnrealEd"
     EditPackages(5)="IpDrv"
     EditPackages(6)="UDebugMenu"
     EditPackages(7)="CTAudio"
     EditPackages(8)="GamePlay"
     EditPackages(9)="CTGame"
     EditPackages(10)="MPGame"
     EditPackages(11)="CTInventory"
     EditPackages(12)="CTCharacters"
     EditPackages(13)="CTMarkers"
     EditPackages(14)="XGame"
     EditPackages(15)="XInterface"
     EditPackages(16)="XInterfaceCommon"
     EditPackages(17)="XInterfaceLive"
     EditPackages(18)="XInterfaceMP"
     EditPackages(19)="XInterfaceCTMenus"
     EditPackages(20)="XInterfaceGamespy"
}

