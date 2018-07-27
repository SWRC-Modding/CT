class HardwareShaderWrapper extends RenderedMaterial
	native;
var() HardwareShader	ShaderImplementation;

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
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)



cpptext
{
	// Make sure derived classes implement the setup funcion
	virtual INT SetupShaderWrapper( FRenderInterface* RI ) { return 0; }

	// TimR: Use first texture as the default size for the browsers
	virtual INT MaterialUSize() 
	{ 
		if( ShaderImplementation && ShaderImplementation->Textures[0] )
			return ShaderImplementation->Textures[0]->MaterialUSize();

		return 0; 
	}
	virtual INT MaterialVSize() 
	{ 
		if( ShaderImplementation && ShaderImplementation->Textures[0] )
			return ShaderImplementation->Textures[0]->MaterialVSize();

		return 0; 
	}
	virtual UBOOL RequiresSorting()
	{
		if( ShaderImplementation )
			return ShaderImplementation->RequiresSorting();

		return 0;
	}


}

defaultproperties
{
}

