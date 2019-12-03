class BotSupport extends Actor native config(ModMPGameConfig);

#exec OBJ LOAD FILE="Gameplay.u"
#exec OBJ LOAD FILE="CTGame.u"
#exec OBJ LOAD FILE="MPGame.u"

var config bool bAutoImportPaths;

var bool bPathsImported;
var Array<Vector> NavPtFailLocations; // Used to debug Navigation points which failed to spawn
var Array<MPBot> Bots;
var Array<Pawn.PatrolPoint> BotPatrolRoute;

native final function SpawnNavigationPoint(Class<NavigationPoint> NavPtClass, Vector Loc, optional Rotator Rot);
native final function BuildPaths();
native final function ClearPaths();

function PostBeginPlay(){
	local BotSupportBroadcastHandler B;

	B = Spawn(class'BotSupportBroadcastHandler');

	B.BotSupport = Self;
	Level.Game.BroadcastHandler = B;
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

event AddBot(){
	local MPBot Bot;

	if(Level.Game.NumPlayers + Level.Game.NumBots >= Level.Game.MaxPlayers){
		Warn("Cannot more bots. Maximum number of players is"@Level.Game.MaxPlayers);

		return;
	}

	Bot = Spawn(class'MPBot', Self);

	if(Bot != None){
		Bot.PlayerReplicationInfo.PlayerName = "Bot"$Bots.Length;
		Bot.bCanGesture = false;
		Bot.ChosenSkin = Rand(5);
		Bot.PlayerReplicationInfo.Team.TeamIndex = 255;
		Bots[Bots.Length] = Bot;
		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);

		Bot.GotoState('Dead', 'MPStart');

		++Level.Game.NumBots;
	}
}

event RemoveBot(){
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
	virtual void Destroy();
	virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);
}

defaultproperties
{
	bHidden=true
	bAutoImportPaths=true
}
