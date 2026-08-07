class RtxParticleComponent extends RtxComponent;

var() config bool bDisableFlipbookAnimations;

function PostBeginPlay()
{
	if(bDisableFlipbookAnimations)
	{
		ConsoleCommand("SET SpriteEmitter TextureUSubdivisions 0");
		ConsoleCommand("SET SpriteEmitter TextureVSubdivisions 0");
	}
}

function Tick(float DeltaTime)
{
}

defaultproperties
{
	bDisableFlipbookAnimations=True
}
