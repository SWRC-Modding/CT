class BotSupportBroadcastHandler extends BroadcastHandler;

var BotSupport BotSupport;

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	if(Msg ~= "AddBot")
		BotSupport.AddBot();
	else if(Msg ~= "RemoveBot")
		BotSupport.RemoveBot();
	else
		Super.Broadcast(Sender, Msg, Type);
}
