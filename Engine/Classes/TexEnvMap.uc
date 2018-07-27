class TexEnvMap extends TexModifier
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() enum ETexEnvMapType
{
	EM_CubeWorldSpaceReflection,
	EM_CubeCameraSpaceReflection,
	EM_SphereWorldSpaceReflection,
	EM_SphereCameraSpaceReflection,
	EM_CubeWorldSpaceNormal,
	EM_CubeCameraSpaceNormal,
	EM_SphereWorldSpaceNormal,
	EM_SphereCameraSpaceNormal,
} EnvMapType;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds);

}

defaultproperties
{
     TexCoordCount=TCN_3DCoords
}

