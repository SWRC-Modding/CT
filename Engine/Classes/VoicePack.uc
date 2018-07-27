//=============================================================================
// VoicePack.
//=============================================================================
class VoicePack extends Info
	abstract;

var Sound	Phrase[8];
var string	PhraseString[8];
var int		PhraseNum;
var PlayerReplicationInfo DelayedSender;

var() Sound AckSound[16];
var() localized string AckString[16];
var() int numAcks;

var() Sound FFireSound[16];
var() localized string FFireString[16];
var() int numFFires;

var() Sound TauntSound[16];
var() localized string TauntString[16];
var() int numTaunts;

var() Sound OrderSound[16];
var() localized string OrderString[16];
var() int numOrders;

//ClientInitialize() sets up playing the appropriate voice segment
function ClientInitialize(PlayerReplicationInfo Sender, PlayerReplicationInfo Recipient, name messagetype, byte messageIndex)
{
	if(PlayerController(Owner).PlayerReplicationInfo != Sender && messagetype == 'ORDER' && 
		(Recipient != None) && (PlayerController(Owner).PlayerReplicationInfo != Recipient) )
	{
		Destroy();
		return;
	}

	if (PlayerController(Owner).bNoVoiceMessages
		|| (PlayerController(Owner).bNoVoiceTaunts && (MessageType == 'TAUNT' || MessageType == 'AUTOTAUNT'))
		|| (PlayerController(Owner).bNoAutoTaunts && MessageType == 'AUTOTAUNT'))
	{
		Destroy();
		return;
	}

	SetTimer(0.6, false);

	if ( messagetype == 'ACK' )
		SetAckMessage(messageIndex, Recipient);
	else if ( messagetype == 'FRIENDLYFIRE' )
		SetFFireMessage(messageIndex, Recipient);
	else if ( messagetype == 'ORDER' )
		SetOrderMessage(messageIndex, Recipient);
	else if ( messagetype == 'TAUNT' || messagetype == 'AUTOTAUNT' )
		SetTauntMessage(messageIndex, Recipient);

	DelayedSender = Sender;
}

static function int PickRandomTauntFor(controller C, bool bNoMature)
{
	local int result, tryCount;
	local bool foundTaunt;

	// Not a while - worried about inifite loops with small number of taunts!
	for(tryCount = 0; !foundTaunt && tryCount < 100; tryCount++)
	{
		result = rand(Default.NumTaunts);

		if(C.DontReuseTaunt(result))
			continue;

		foundTaunt = true;
	}

	if(!foundTaunt)
		Log("PickRandomTauntFor: Could Not Find Suitable Taunt.");

	return result;
}

function SetAckMessage(int messageIndex, PlayerReplicationInfo Recipient)
{
	messageIndex		= Clamp(messageIndex, 0, numAcks-1);
	Phrase[0]			= AckSound[messageIndex];
	PhraseString[0]		= AckString[messageIndex];
}

function SetFFireMessage(int messageIndex, PlayerReplicationInfo Recipient)
{
	messageIndex	= Clamp(messageIndex, 0, numFFires-1);
	Phrase[0]		= FFireSound[messageIndex];
	PhraseString[0] = FFireString[messageIndex];
}

function SetOrderMessage(int messageIndex, PlayerReplicationInfo Recipient)
{
	messageIndex	= Clamp(messageIndex, 0, numOrders-1);
	Phrase[0]		= OrderSound[messageIndex];
	PhraseString[0] = OrderString[messageIndex];
}

function SetTauntMessage(int messageIndex, PlayerReplicationInfo Recipient)
{
	messageIndex	= Clamp(messageIndex, 0, numTaunts-1);
	Phrase[0]		= TauntSound[messageIndex];
	PhraseString[0] = TauntString[messageIndex];
}

function Timer()
{
	local PlayerController PlayerOwner;

	PlayerOwner = PlayerController(Owner);

	if(Phrase[PhraseNum] != None)
	{
		if(DelayedSender != None && PlayerOwner.MyHUD != None)
			PlayerOwner.MyHud.Message(DelayedSender, PhraseString[PhraseNum], 'Say');

		if((Level.TimeSeconds - PlayerOwner.LastPlaySpeech > 2) || (PhraseNum > 0))
		{
			PlayerOwner.LastPlaySpeech = Level.TimeSeconds;
//gdr Fix for listen server sending the speech to all connected players.  CPSL doesn't play the sound if there's no view target
//If it causes problems it may need to be changed.
			PlayerOwner.ClientPlaySoundLocally(Phrase[PhraseNum]);
/*			
if ( (PlayerOwner.ViewTarget != None) )
			{
				PlayerOwner.ViewTarget.PlaySound(Phrase[PhraseNum], SLOT_Interface,,,,,false);
			}
			else
			{
				PlayerOwner.PlaySound(Phrase[PhraseNum], SLOT_Interface,,,,,false);
			}
*/
			if ( Phrase[PhraseNum+1] == None )
			{
				Destroy();
			}
			else
			{
				SetTimer(GetSoundDuration(Phrase[PhraseNum]), false);
				PhraseNum++;
			}
			return;
		}
	}
	Destroy();
}


static function PlayerSpeech( name Type, int Index, string Callsign, Actor PackOwner )
{
	local name SendMode;
	local int i;
	local PlayerReplicationInfo Recipient;
	local GameReplicationInfo GRI;

	switch (Type)
	{
		case 'ACK':					// Acknowledgements
		case 'FRIENDLYFIRE':		// Friendly Fire
			SendMode = 'TEAM';		// Only send to team.
			Recipient = None;		// Send to everyone.
			break;

		case 'ORDER':				// Orders
			SendMode = 'TEAM';		// Only send to team.
            GRI = PlayerController(PackOwner).GameReplicationInfo;
			if ( GRI.bTeamGame )
			{
				if ( Callsign == "" )
					Recipient = None;
				else 
				{
					for ( i=0; i<GRI.PRIArray.Length; i++ )
						if ( (GRI.PRIArray[i] != None) && (GRI.PRIArray[i].GetPlayerName() == Callsign)
							&& (GRI.PRIArray[i].Team == PlayerController(PackOwner).PlayerReplicationInfo.Team) )
						{
							Recipient = GRI.PRIArray[i];
							break;
						}
				}
			}
			break;

		case 'TAUNT':				// Taunts
			SendMode = 'GLOBAL';	// Send to all teams.
			Recipient = None;		// Send to everyone.
			break;

		default:
			SendMode = 'GLOBAL';
			Recipient = None;
	}
	if (!PlayerController(PackOwner).GameReplicationInfo.bTeamGame)
		SendMode = 'GLOBAL';  // Not a team game? Send to everyone.

	//Log("PlayerSpeech: "$Type $" Ix:"$Index $" Callsign:"$Callsign $" Recip:"$Recipient);
	Controller(PackOwner).SendVoiceMessage( Controller(PackOwner).PlayerReplicationInfo, Recipient, Type, Index, SendMode );
}


defaultproperties
{
     LifeSpan=10
}

