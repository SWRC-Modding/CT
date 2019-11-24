class BotSupport extends Actor native;

#exec OBJ LOAD FILE="MPGame.u"

var Array<Vector> NavPtFailLocations; // Used to debug Navigation points which failed to spawn
var Array<MPBot> Bots;
var Array<PlayerStart> SpawnPoints;

function PostBeginPlay(){
	local PlayerStart P;

	foreach AllActors(class'PlayerStart', P)
		SpawnPoints[SpawnPoints.Length] = P;

	SetTimer(5.0f, true);
}

function Timer(){
	local int i;
	local Pawn P;
	local PlayerStart SpawnPoint;

	for(i = 0; i < Bots.Length; ++i){
		if(Bots[i].Pawn == None){
			SpawnPoint = SpawnPoints[Rand(SpawnPoints.Length)];
			P = Spawn(class'MPClone',,, SpawnPoint.Location, SpawnPoint.Rotation);

			if(P != None){
				P.AddDefaultInventory();
				Bots[i].Possess(P);
			}
		}
	}
}

function AddBot(){
	local MPBot Bot;

	Bot = Spawn(class'MPBot');

	if(Bot != None){
		Bot.PlayerReplicationInfo.PlayerName = "Bot"$Bots.Length;
		Bot.bCanGesture = false;
		Bots[Bots.Length] = Bot;
		BroadcastLocalizedMessage(Level.Game.GameMessageClass, 1, Bot.PlayerReplicationInfo);
	}
}

function RemoveBot(){
	Bots[Bots.Length - 1].Pawn.Destroy();
	Bots[Bots.Length - 1].Destroy();
	Bots.Length = Bots.Length - 1;
}

cpptext
{
	virtual void Spawned();
	virtual void Destroy();
	virtual UBOOL Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void PostRender(class FLevelSceneNode* SceneNode, class FRenderInterface* RI);

	void SpawnNavigationPoint(UClass* NavPtClass, const FVector& Location, const FRotator& Rotation = FRotator(0, 0, 0));
	void ImportPaths();
	void BuildPaths();

	// Static because it needs to be accessed by UExportPathsCommandlet
	static void ExportPaths(ALevelInfo* LevelInfo);
}

defaultproperties
{
	//bHidden=true
}
