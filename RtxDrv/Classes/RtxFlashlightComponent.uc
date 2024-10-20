class RtxFlashlightComponent extends RtxComponent;

struct SquadMemberFlashlight{
	var Pawn          Pawn;
	var RtxFlashlight Flashlight;
};

var   array<SquadMemberFlashlight> SquadFlashlights;
var() editinline RtxFlashlight     PlayerFlashlight;

function OnSaveConfig()
{
	PlayerFlashlight.SaveConfig();
}

function SetupSquadLights(Squad Squad)
{
	local int i, j;

	for(i = 1; i < Squad.SquadMembers.Length; ++i)
	{
		for(j = 0; j < SquadFlashlights.Length; ++j)
		{
			if(SquadFlashlights[j].Pawn == Squad.SquadMembers[i].Pawn)
				goto end;

			if(SquadFlashlights[j].Pawn == None)
			{
				SquadFlashlights.Remove(j, 1);
				goto end;
			}
		}

		j = SquadFlashlights.Length;
		SquadFlashlights.Length = j + 1;
		SquadFlashlights[j].Pawn = Squad.SquadMembers[i].Pawn;
		SquadFlashlights[j].Flashlight = new class'RtxFlashlight';
		SquadFlashlights[j].Flashlight.Init();
end:
	}
}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if(PlayerFlashlight == None)
	{
		PlayerFlashlight = new class'RtxFlashlight';
		PlayerFlashlight.Init();
	}
}

function Destroyed()
{
	local int i;

	for(i = 0; i < SquadFlashlights.Length; ++i)
		SquadFlashlights[i].Flashlight.Exit();

	SquadFlashlights.Length = 0;
	PlayerFlashlight.Exit();
	Super.Destroyed();
}

function Tick(float DeltaTime)
{
	local PlayerController PC;
	local Pawn Pawn;
	local int i;

	PC = Level.GetLocalPlayerController();

	if(PC == None || PC.Pawn == None)
		return;

	Pawn = PC.Pawn;

	if(Pawn.Squad != None)
		SetupSquadLights(PC.Pawn.Squad);

	PlayerFlashlight.Update(Pawn, Pawn.Flashlight.bIsOn);

	// Disable light spawned in Pawn::SetFlashlight
	if(PC.PlayerSpotlight != None)
	{
		PC.PlayerSpotlight.LightType = LT_None;
		PC.PlayerSpotlight.bLightPriorityOverride = false;
	}

	for(i = 0; i < SquadFlashlights.Length; ++i)
		SquadFlashlights[i].Flashlight.Update(SquadFlashlights[i].Pawn, PC.Pawn.Flashlight.bIsOn);
}
