class MaterialSequence extends Modifier
	editinlinenew
	hidecategories(Modifier)
	native;
	
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

enum EMaterialSequenceAction
{
	MSA_ShowMaterial,
	MSA_FadeToMaterial,
};

struct native MaterialSequenceItem
{
	var() editinlineuse Material Material;
	var() float Time;
	var() EMaterialSequenceAction Action;
};

enum EMaterialSequenceTriggerActon
{
	MSTA_Ignore,
	MSTA_Reset,
	MSTA_Pause,
	MSTA_Stop,
};

var() array<MaterialSequenceItem> SequenceItems;
var() EMaterialSequenceTriggerActon TriggerAction;
var() bool Loop;
var() bool Paused;
var transient float CurrentTime;
var transient float LastTime;
var float TotalTime;

function Reset()
{
	CurrentTime = 0;
	LastTime = 0;
	Paused = default.Paused;
}

function Trigger( Actor Other, Actor EventInstigator )
{
	switch(TriggerAction)
	{
	case MSTA_Reset:
		CurrentTime = 0;
		LastTime = 0;
		break;
	case MSTA_Pause:
		Paused = !Paused;
		break;
	case MSTA_Stop:
		Paused = True;
		break;
	}		
}



cpptext
{
	virtual void PostEditChange();
	virtual UBOOL CheckCircularReferences( TArray<class UMaterial*>& History );
	virtual void PreSetMaterial(FLOAT TimeSeconds);
	virtual void Serialize(FArchive& Ar);

}

defaultproperties
{
     Loop=True
}

