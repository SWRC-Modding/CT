class BotSupport extends Actor native config(ModMPGameConfig);

#exec OBJ LOAD FILE="Gameplay.u"
#exec OBJ LOAD FILE="CTGame.u"
#exec OBJ LOAD FILE="MPGame.u"

var config bool bAutoImportPaths;

var bool bPathsImported;
var Array<Vector> NavPtFailLocations; // Used to debug Navigation points which failed to spawn
var Array<MPBot> Bots;
var Array<PlayerStart> SpawnPoints;
var ScriptedSequence BotScript; // Automatically spawned in native code

native final function SpawnNavigationPoint(Class<NavigationPoint> NavPtClass, Vector Loc, optional Rotator Rot);
native final function BuildPaths();
native final function ClearPaths();

function PostBeginPlay(){
	local PlayerStart P;
	local BotSupportBroadcastHandler B;

	foreach AllActors(class'PlayerStart', P)
		SpawnPoints[SpawnPoints.Length] = P;

	B = Spawn(class'BotSupportBroadcastHandler');

	B.BotSupport = Self;
	Level.Game.BroadcastHandler = B;
}

/*
 * Called by native code whenever paths are rebuilt.
 * TODO: Maybe get rid of the ScriptedSequence and use Pawn::PatrolRoute instead
 */
event SetupBotScript(){
	local NavigationPoint NavPt;
	local ScriptedAction S;

	BotScript.Actions.Length = 0;
	NavPt = Level.NavigationPointList;

	while(NavPt != None){
		if(NavPt.IsA('PatrolPoint')){
			NavPt.Tag = NavPt.Name;
			S = new Class'ACTION_MoveToPoint';
			ACTION_MoveToPoint(S).WalkToPoint = true;
			ACTION_MoveToPoint(S).OrientToPoint = true;
			ACTION_MoveToPoint(S).DestinationTag = NavPt.Name;
			BotScript.Actions[BotScript.Actions.Length] = S;
		}

		NavPt = NavPt.nextNavigationPoint;
	}

	S = new Class'ACTION_GotoAction';
	ACTION_GotoAction(S).ActionNumber = 1;
	BotScript.Actions[BotScript.Actions.Length] = S;
}

event AddBot(){
	local MPBot Bot;

	Bot = Spawn(class'MPBot');

	if(Bot != None){
		Bot.PlayerReplicationInfo.PlayerName = "Bot"$Bots.Length;
		Bot.bCanGesture = false;
		Bot.ChosenSkin = Rand(5);
		Bot.PlayerReplicationInfo.Team.TeamIndex = 255;
		Bots[Bots.Length] = Bot;
		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);

		Bot.GotoState('Dead', 'MPStart');

		++Level.Game.NumBots;

		//Level.Game.RestartPlayer(Bot);

		if(BotScript != None && BotScript.Actions.Length > 0){
			Bot.ClearScript();
			Bot.SetNewScript(BotScript);
			Bot.SetScriptingPriority(0.5f);
		}
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
