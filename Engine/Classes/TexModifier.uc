class TexModifier extends Modifier
	noteditinlinenew
	native;

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

var enum ETexCoordSrc
{
	TCS_Stream0,
	TCS_Stream1,
	TCS_Stream2,
	TCS_Stream3,
	TCS_Stream4,
	TCS_Stream5,
	TCS_Stream6,
	TCS_Stream7,
	TCS_WorldCoords,
	TCS_CameraCoords,
	TCS_CubeWorldSpaceReflection,
	TCS_CubeCameraSpaceReflection,
	TCS_ProjectorCoords,
	TCS_NoChange,				// don't specify a source, just modify it
	TCS_SphereWorldSpaceReflection,			// TimR (BumpMapping)
	TCS_SphereCameraSpaceReflection,	// TimR (BumpMapping)
	TCS_CubeWorldSpaceNormal,
	TCS_CubeCameraSpaceNormal,
	TCS_SphereWorldSpaceNormal,
	TCS_SphereCameraSpaceNormal,
	TCS_BumpSphereCameraSpaceNormal,
	TCS_BumpSphereCameraSpaceReflection,
} TexCoordSource;

var enum ETexCoordCount
{
	TCN_2DCoords,
	TCN_3DCoords,
	TCN_4DCoords
} TexCoordCount;

var(Texture) enum ETexClampModeOverried
{
	TCO_UseTextureMode,
	TCO_Wrap,
	TCO_Clamp,
} UClampMode, VClampMode;

var bool TexCoordProjected;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds) { return NULL; }

	// Material interface.
	virtual INT MaterialUSize();
	virtual INT MaterialVSize();
	virtual BYTE RequiredUVStreams();
	virtual UBOOL GetValidated();
	virtual void SetValidated( UBOOL InValidated );

}

defaultproperties
{
     TexCoordSource=TCS_NoChange
}

