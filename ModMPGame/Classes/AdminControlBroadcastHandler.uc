class AdminControlBroadcastHandler extends BroadcastHandler;

var() AdminControl AdminControl;

function PostBeginPlay(){
	Super.PostBeginPlay();

	foreach AllActors(Class'AdminControl', AdminControl)
		break;
}

function Broadcast(Actor Sender, coerce string Msg, optional name Type){
	local String Cmd;

	if(InStr(Msg, "/") == 0){
		Cmd = Right(Msg, Len(Msg) - 1);
		AdminControl.ExecCmd(PlayerController(Sender), Cmd);
	}else{
		Super.Broadcast(Sender, Msg, Type);
	}
}
