#exec OBJ LOAD FILE="MPGame.u"
#exec OBJ LOAD FILE="CTGame.u"

class BotSupport extends Actor native;

var array<MPBot> Bots;
var array<struct Points{var array<PlayerStart> SpawnPoints;}> SpawnPointsByTeam;
var ScriptedSequence BotScript;

native final function TestNativeFunc();
/*
*	Imports paths from map specified so that bots can move around without having
*	to distribute a modified version of the default maps to the client
*	The specified map should obviously be the same as the one that is currently loaded
*	with the only difference that it contains paths for AI
*/
native(1024) final function ImportPaths(string MapName);

function PostNetBeginPlay(){
	local PlayerStart SpawnPoint;

	Super.PostNetBeginPlay();

	Level.Game.BroadcastHandler.Destroy();

	Level.Game.BroadcastHandler = spawn(class'BotSupportBroadcastHandler', self);

	foreach AllActors(class'PlayerStart', SpawnPoint){
		if(SpawnPointsByTeam.Length <= SpawnPoint.TeamNumber)
			SpawnPointsByTeam.Length = SpawnPoint.TeamNumber + 1;
TestNativeFunc();
		//Ugly as fuck, but whatever...
		SpawnPointsByTeam[SpawnPoint.TeamNumber].SpawnPoints[SpawnPointsByTeam[SpawnPoint.TeamNumber].SpawnPoints.Length] = SpawnPoint;
	}

	foreach AllActors(class'ScriptedSequence', BotScript, 'BotScript')
		break;

	SetTimer(2.0f, true);
	ImportPaths(Mid(Left(Level, InStr(Level, ".")), Len(Level.Game.MapPrefix) + 1)$"_Paths");
}

function Timer(){
	local int i;

	for(i = 0; i < Bots.Length; i++){
		if(Bots[i].Pawn == None)
			SpawnBotPawn(Bots[i]);

		if(BotScript != None && StateIdle(Bots[i].CurrentState) != None && Bots[i].Pawn != None)
			Bots[i].SetNewScript(BotScript);
	}
}

function BotAdd(int Team){
	local MPBot Bot;

	Bot = Spawn(class'MPBot');

	if(Bot == None){
		Warn("Couldn't spawn bot");

		return;
	}

	Bot.Team = Team;
	Bot.ChosenSkin = rand(5);
	Bot.PlayerReplicationInfo = Spawn(class'PlayerreplicationInfo', Bot);
	Bot.PlayerReplicationInfo.PlayerName = "Bot"$Bots.Length;
	Bot.bIsPlayer = true;	//So that the controller won't be destroyed when the Pawn is killed;
							//Must be set here and not in defaultproperties or else there's two scoreboard entries for one bot

	BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);	

	Bots[Bots.Length] = Bot;

	if(Level.Game.bTeamGame){
		if(Team < 0)
			Team = 0;
		else if(Team > 1)
			Team = 1;

		TDGame(Level.Game).Teams[Team].AddToTeam(Bot);
		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 3, Bot.PlayerReplicationInfo, None, TDGame(Level.Game).Teams[Team]);
	}
}

function SpawnBotPawn(MPBot Bot){
	local int Team;
	local PlayerStart Spawnpoint;
	local MPPawn BotPawn;
	local class<MPPawn> PawnClass;

	if(Level.Game.bTeamGame)
		Team = Bot.Team;
	else
		Team = Rand(SpawnPointsByTeam.Length);

	SpawnPoint = SpawnPointsByTeam[Team].SpawnPoints[rand(SpawnPointsByTeam[Team].SpawnPoints.Length)];

	if(Bot.Team == 0) //TODO: Use 'ValidPawnClasses' instead
		PawnClass = class'MPClone';
	else
		PawnClass = class'MPTrandoshan';

	BotPawn = Spawn(PawnClass,,,Spawnpoint.Location, Spawnpoint.Rotation);

	if(BotPawn == None)	//Spawn might fail if there's another pawn at this spawnpoint so just return and spawn next time...
		return;

	BotPawn.PatrolMode = PM_Once;
	ConsoleCommand("Set Pawn Accuracy "$0.88);
	BotPawn.ChosenSkin = Bot.ChosenSkin;
	BotPawn.AutoDetectRadius = 1024;

	Bot.Pawn = BotPawn;
	BotPawn.Controller = Bot;

	Bot.Possess(BotPawn);

	BotPawn.PlayerReplicationInfo = Bot.PlayerReplicationInfo;

	BotPawn.DoCustomizations();
	BotPawn.AddDefaultInventory();
}

defaultproperties
{

}