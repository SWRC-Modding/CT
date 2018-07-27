class TexMatrix extends TexModifier
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var Matrix Matrix;


cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds) { return &Matrix; }

}

defaultproperties
{
}

