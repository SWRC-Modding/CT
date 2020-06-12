class AdminControlBroadcastHandler extends BroadcastHandler;

var() AdminControl AdminControl;

function PostBeginPlay(){
	Super.PostBeginPlay();

	foreach AllActors(class'AdminControl', AdminControl)
		break;
}

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	local String Cmd;

	if(InStr(Msg, "/") == 0){
		Cmd = Right(Msg, Len(Msg) - 1);
		Log(PlayerController(Sender).PlayerReplicationInfo.PlayerName $ ": " $ Cmd, 'Command');
		AdminControl.ExecCmd(Cmd, PlayerController(Sender));
	}else{
		Log(PlayerController(Sender).PlayerReplicationInfo.PlayerName $ ": " $ Msg, 'ChatMessage');
		Super.Broadcast(Sender, Msg, Type);
	}
}
