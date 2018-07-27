class TexScaler extends TexModifier
	editinlinenew
	native;

// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

var Matrix M;
var() float UScale;
var() float VScale;
var() float UOffset;
var() float VOffset;



cpptext
{
	// UTexModifier interface
	virtual FMatrix* GetMatrix(FLOAT TimeSeconds);

}

defaultproperties
{
     UScale=1
     VScale=1
}

