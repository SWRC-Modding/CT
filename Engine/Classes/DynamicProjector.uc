class DynamicProjector extends Projector;

simulated function Tick(float DeltaTime)
{
	DetachProjector();
	AttachProjector();
}


defaultproperties
{
     bDynamicAttach=True
     bStatic=False
}

