class TexPanner extends TexModifier
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var() rotator PanDirection;
var() float PanRate;
var Matrix M;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds);

}

defaultproperties
{
     PanRate=0.1
}

