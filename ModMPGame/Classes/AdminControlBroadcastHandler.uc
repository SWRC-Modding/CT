class AdminControlBroadcastHandler extends BroadcastHandler;

var() AdminControl AdminControl;

function PostBeginPlay(){
	Super.PostBeginPlay();

	foreach AllActors(class'AdminControl', AdminControl)
		break;
}

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	local String Cmd;
	local PlayerController PC;

	PC = PlayerController(Sender);

	if(PC != None && InStr(Msg, "/") == 0){
		Cmd = Right(Msg, Len(Msg) - 1);
		Log(PC.PlayerReplicationInfo.PlayerName $ ": " $ Cmd, 'Command');
		AdminControl.ExecCmd(Cmd, PC);
	}else{
		if(PC != None)
			Log(PlayerController(Sender).PlayerReplicationInfo.PlayerName $ ": " $ Msg, 'ChatMessage');

		Super.Broadcast(Sender, Msg, Type);
	}
}
