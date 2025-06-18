class RtxComponent extends Actor native transient config(Rtx) hidecategories(Advanced, AI, Collision, CollisionAdvanced, Display, DisplayAdvanced, Events, Karma, LightColor, Lighting, marker, Movement, MovementAdvanced, Object, Sound);

var Rtx Rtx;

event OnSaveConfig();

function PostBeginPlay()
{
	Rtx = class'Rtx'.static.GetInstance();
}

defaultproperties
{
	bHidden=True
	DrawType=DT_None
}
