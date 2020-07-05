class AdminControlBroadcastHandler extends BroadcastHandler;

var() AdminControl AdminControl;

function PostBeginPlay(){
	Super.PostBeginPlay();

	foreach AllActors(class'AdminControl', AdminControl)
		break;
}

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	local string Cmd;
	local PlayerController PC;
	local name Event;

	PC = PlayerController(Sender);

	if(PC != None && InStr(Msg, "/") == 0){
		Cmd = Right(Msg, Len(Msg) - 1);
		AdminControl.ExecCmd(Cmd, PC);
	}else{
		Cmd = "Server";

		if(Type == ''){
			Event = 'Message';
		}else{
			Event = Type;

			if(PC != None)
				Cmd = PC.PlayerReplicationInfo.PlayerName;
		}

		AdminControl.EventLog("(" $ Cmd $ "): " $ Msg, Event);
		Super.Broadcast(Sender, Msg, Type);
	}
}
