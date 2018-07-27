//=============================================================================
// BroadcastHandler
//
// Message broadcasting is delegated to BroadCastHandler by the GameInfo.  
// The BroadCastHandler handles both text messages (typed by a player) and 
// localized messages (which are identified by a LocalMessage class and id).  
// GameInfos produce localized messages using their DeathMessageClass and 
// GameMessageClass classes.
//
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class BroadcastHandler extends Info;

var	int			    SentText;
var config bool		bMuteSpectators;			// Whether spectators are allowed to speak.
var config bool		bPartitionSpectators;			// Whether spectators are can only speak to spectators.

function UpdateSentText()
{
	SentText = 0;
}

/* Whether actor is allowed to broadcast messages now.
*/
function bool AllowsBroadcast( actor broadcaster, int Len )
{
	if ( bMuteSpectators && (PlayerController(Broadcaster) != None)
		&& !PlayerController(Broadcaster).PlayerReplicationInfo.bAdmin
		&& (PlayerController(Broadcaster).PlayerReplicationInfo.bOnlySpectator) )
		return false;

	SentText += Len;
	return ( (Level.Pauser != None) || (SentText < 400) );
}


function BroadcastText( PlayerReplicationInfo SenderPRI, PlayerController Receiver, coerce string Msg, optional name Type )
{
	Receiver.TeamMessage( SenderPRI, Msg, Type );
}

function BroadcastLocalized( Actor Sender, PlayerController Receiver, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	Receiver.ReceiveLocalizedMessage( Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );
}

function Broadcast( Actor Sender, coerce string Msg, optional name Type )
{
	local Controller C;
	local PlayerController P;
	local PlayerReplicationInfo PRI;

	// see if allowed (limit to prevent spamming)
	if ( !AllowsBroadcast(Sender, Len(Msg)) )
		return;

	if ( Pawn(Sender) != None )
		PRI = Pawn(Sender).PlayerReplicationInfo;
	else if ( Controller(Sender) != None )
		PRI = Controller(Sender).PlayerReplicationInfo;

	if ( bPartitionSpectators && (PRI != None) && PRI.bOnlySpectator )
	{
		For ( C=Level.ControllerList; C!=None; C=C.NextController )
		{
			P = PlayerController(C);
			if ( (P != None) && P.PlayerReplicationInfo.bOnlySpectator )
				BroadcastText(PRI, P, Msg, Type);
		}
	}
	else
	{
		For ( C=Level.ControllerList; C!=None; C=C.NextController )
		{
			P = PlayerController(C);
			if ( P != None )
				BroadcastText(PRI, P, Msg, Type);
		}
	}
}

function BroadcastTeam( Controller Sender, coerce string Msg, optional name Type )
{
	local Controller C;
	local PlayerController P;

	// see if allowed (limit to prevent spamming)
	if ( !AllowsBroadcast(Sender, Len(Msg)) )
		return;

	if ( bPartitionSpectators && (Sender != None) && (Sender.PlayerReplicationInfo.bOnlySpectator) )
	{
		For ( C=Level.ControllerList; C!=None; C=C.NextController )
		{
			P = PlayerController(C);
			if ( (P != None) && (P.PlayerReplicationInfo.Team == Sender.PlayerReplicationInfo.Team)
				&& (P.PlayerReplicationInfo.bOnlySpectator) )
				BroadcastText(Sender.PlayerReplicationInfo, P, Msg, Type);
		}
	}
	else
	{
		For ( C=Level.ControllerList; C!=None; C=C.NextController )
	  	{
			P = PlayerController(C);
		  	if ( (P != None) && (P.PlayerReplicationInfo.Team == Sender.PlayerReplicationInfo.Team) )
				BroadcastText(Sender.PlayerReplicationInfo, P, Msg, Type);
		}
	}
}

/*
 Broadcast a localized message to all players.
 Most messages deal with 0 to 2 related PRIs.
 The LocalMessage class defines how the PRI's and optional actor are used.
*/
event AllowBroadcastLocalized( actor Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	local Controller C;
	local PlayerController P;

	For ( C=Level.ControllerList; C!=None; C=C.NextController )
	{
		P = PlayerController(C);
		if ( P != None )
		BroadcastLocalized(Sender, P, Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
	}
}

defaultproperties
{
}

