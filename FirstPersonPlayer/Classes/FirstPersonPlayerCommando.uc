class FirstPersonPlayerCommando extends PlayerCommando;

function PostBeginPlay(){
	local FirstPersonPlayer FP;

	Tag = 'PlayerCommando';
	Super.PostBeginPlay();

	foreach AllActors(class'FirstPersonPlayer', FP)
	 break;

	if(FP == None)
		Spawn(class'FirstPersonPlayer');
}


defaultproperties
{
	Skins(0)=Shader'CloneTextures.CloneTextures.CloneCommando38_Shader'
}
