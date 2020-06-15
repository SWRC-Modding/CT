class BotSupport extends AdminService native;

#exec OBJ LOAD FILE="Gameplay.u"
#exec OBJ LOAD FILE="CTGame.u"
#exec OBJ LOAD FILE="MPGame.u"

var() config float BotAccuracy;

var config bool bAutoImportPaths;
var config bool bAutoBuildPaths;
var config bool bShowPaths;

var bool bPathsImported; // Paths were imported using ImportPaths
var Array<vector> NavPtFailLocations; // Used to debug Navigation points which failed to spawn
var Array<MPBot> Bots;
var Array<Pawn.PatrolPoint> BotPatrolRoute;

function PostBeginPlay(){
	ConsoleCommand("set MPPawn Accuracy " $ BotAccuracy);
}

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local String ErrorMsg;

	if(ParseCommand(Cmd, "ADDBOT")){
		if(Level.Game.NumPlayers + Level.Game.NumBots < Level.Game.MaxPlayers)
			AddBot();
		else{
			ErrorMsg = "Cannot add more bots. Maximum number of players is " $ Level.Game.MaxPlayers;

			if(PC != None)
				PC.ClientMessage(ErrorMsg);
			else
				Log(ErrorMsg);
		}

		return true;
	}else if(ParseCommand(Cmd, "REMOVEBOT")){
		RemoveBot();

		return true;
	}

	return Super.ExecCmd(Cmd, PC);
}

event SetupPatrolRoute(){
	local NavigationPoint NavPt;
	local Pawn.PatrolPoint P;

	BotPatrolRoute.Length = 0;

	for(NavPt = Level.NavigationPointList; NavPt != None; NavPt = NavPt.nextNavigationPoint){
		if(PatrolPoint(NavPt) != None){
			P.Node = NavPt;
			P.RunToNode = true;
			P.ShootWhileMoving = true;
			P.OrientToNode = true;
			P.PatrolPriorityOverride = 0.5;
			BotPatrolRoute[BotPatrolRoute.Length] = P;
		}
	}
}

function AddBot(){
	local MPBot Bot;

	Bot = Spawn(class'MPBot', self);

	if(Bot != None){
		Bot.PlayerReplicationInfo.PlayerName = "Bot" $ Bots.Length;
		Bot.bCanGesture = false;
		Bot.ChosenSkin = Rand(5);
		Bots[Bots.Length] = Bot;
		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);

		Bot.GotoState('Dead', 'MPStart');

		++Level.Game.NumBots;
	}
}

function RemoveBot(){
	if(Bots.Length > 0){
		Bots[Bots.Length - 1].Pawn.Destroy();
		Bots[Bots.Length - 1].Destroy();
		Bots.Length = Bots.Length - 1;

		--Level.Game.NumBots;
	}
}

cpptext
{
	void SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation = FRotator(0, 0, 0));
	void ImportPaths();
	void ExportPaths();
	void BuildPaths();
	void ClearPaths();

	// Overrides
	virtual void Spawned();
	virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);
	virtual bool ExecCmd(const char* Cmd, class APlayerController* PC);
}

defaultproperties
{
	BotAccuracy=1.0
	bHidden=true
	bAutoImportPaths=true
	bAutoBuildPaths=false
	bShowPaths=false
}
