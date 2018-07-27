class HsBumpMetal extends HsBumpDiffSpec;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// Make sure to implement this function in UnHardwareShaderWrapper.cpp
	virtual INT SetupShaderWrapper( FRenderInterface* RI );

}

defaultproperties
{
     ShaderImplementation=HardwareShader'HardwareShaders.Bump.DOT3Metal'
}

