class ConstantColor extends ConstantMaterial
	native
	editinlinenew;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() Color Color;


cpptext
{
	//
	// UConstantMaterial interface
	//
	virtual FColor GetColor(FLOAT TimeSeconds) { return Color; }

}

defaultproperties
{
}

