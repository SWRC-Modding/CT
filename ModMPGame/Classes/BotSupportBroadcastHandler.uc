class BotSupportBroadcastHandler extends BroadcastHandler;

var BotSupport BotSupport;

function PostBeginPlay(){
	Super.PostBeginPlay();
	BotSupport = Spawn(class'BotSupport');
	Level.Game.BroadcastHandler = Self; // TODO: Do this via the GameInfo::BroadcastHandlerClass property
}

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	if(Msg ~= "AddBot")
		BotSupport.AddBot();
	else if(Msg ~= "RemoveBot")
		BotSupport.RemoveBot();
	else
		Super.Broadcast(Sender, Msg, Type);
}
