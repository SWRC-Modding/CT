class BotSupportBroadcastHandler extends BroadcastHandler;

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	local string Upper;

	Upper = Caps(Msg);

	if(InStr(Upper, "BOTADD") == 0){
		BotSupport(Owner).BotAdd(int(Mid(Msg, Len("BOTADD"))));
	}else{
		Super.Broadcast(Sender, Msg, Type);
	}
}