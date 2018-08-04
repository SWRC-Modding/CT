class ModGameEngine extends GameEngine native;

var() config float CustomTickRate;

cpptext
{
	//Overrides
	virtual UBOOL Exec(const char* Cmd, FOutputDevice& Ar);
	virtual void Tick(float DeltaTime);
	virtual float GetMaxTickRate();
}

defaultproperties
{
	CustomTickRate=60.0f
}