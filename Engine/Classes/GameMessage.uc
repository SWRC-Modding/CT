class GameMessage extends LocalMessage;

var(Message) localized string SwitchLevelMessage;
var(Message) localized string LeftMessage;
var(Message) localized string FailedTeamMessage;
var(Message) localized string FailedPlaceMessage;
var(Message) localized string FailedSpawnMessage;
var(Message) localized string EnteredMessage;
var(Message) localized string MaxedOutMessage;
var(Message) localized string OvertimeMessage;
var(Message) localized string GlobalNameChange;
var(Message) localized string NewTeamMessage;
var(Message) localized string NewTeamMessageTrailer;
var(Message) localized string NoNameChange;
var(Message) localized string VoteStarted;
var(Message) localized string VotePassed;
var(Message) localized string MustHaveStats;
var(Message) localized string CantBeSpectator;
var(Message) localized string CantBePlayer;
var(Message) localized string BecameSpectator;
var(Message) localized string StrTooMangRep;
var(Message) localized string StrTooMangTran;

var localized string NewPlayerMessage;
var localized string KickWarning;
var localized string NewSpecMessage, SpecEnteredMessage;

//
// Messages common to GameInfo derivatives.
//
static function string GetString(
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local string PName;

	if (RelatedPRI_1 != None)
		PName = GetPlayerName(RelatedPRI_1);
	
	switch (Switch)
	{
		case 0:
			return Default.OverTimeMessage;
			break;
		case 1:
			if (RelatedPRI_1 == None)
				return Default.NewPlayerMessage;

			return PName$Default.EnteredMessage;
			break;
		case 2:
			if (RelatedPRI_1 == None)
				return "";

			//Ignore this on XBox live since the name doesn't matter, only the gamertag that can't be changed.
			if (RelatedPRI_1.ConsoleCommand("XLIVE GETAUTHSTATE") == "ONLINE")
				return "";

			return RelatedPRI_1.OldName@Default.GlobalNameChange@RelatedPRI_1.PlayerName;
			break;
		case 3:
			if (RelatedPRI_1 == None)
				return "";
			if (OptionalObject == None)
				return "";

            return PName@Default.NewTeamMessage@TeamInfo(OptionalObject).GetHumanReadableName()$Default.NewTeamMessageTrailer;
			break;
		case 4:
			if (RelatedPRI_1 == None)
				return "";

			return PName$Default.LeftMessage;
			break;
		case 5:
			return Default.SwitchLevelMessage;
			break;
		case 6:
			return Default.FailedTeamMessage;
			break;
		case 7:
			return Default.MaxedOutMessage;
			break;
		case 8:
			return Default.NoNameChange;
			break;
        case 9:
			return PName@Default.VoteStarted;
            break;
        case 10:
            return Default.VotePassed;
            break;
        case 11:
			return Default.MustHaveStats;
			break;
	case 12:
		return Default.CantBeSpectator;
		break;
	case 13:
		return Default.CantBePlayer;
		break;
	case 14:
		return PName@Default.BecameSpectator;
		break;
	case 15:
		return Default.KickWarning;
		break;
	case 16:
            if (RelatedPRI_1 == None)
                return Default.NewSpecMessage;

            return PName$Default.SpecEnteredMessage;
            break;
	case 17:
		return Default.StrTooMangRep;
		break;
	case 18:
		return Default.StrTooMangTran;
		break;
	}
	return "";
}


defaultproperties
{
     SwitchLevelMessage="Switching Levels"
     LeftMessage=" left the game."
     FailedTeamMessage="Could not find team for player"
     FailedPlaceMessage="Could not find a starting spot"
     FailedSpawnMessage="Could not spawn player"
     EnteredMessage=" entered the game."
     MaxedOutMessage="Server is already at capacity."
     OvertimeMessage="Score tied at the end of regulation. Sudden Death Overtime!!!"
     GlobalNameChange="changed name to"
     NewTeamMessage="is now on"
     NoNameChange="Name is already in use."
     VoteStarted="started a vote."
     VotePassed="Vote passed."
     MustHaveStats="Must have stats enabled to join this server."
     CantBeSpectator="Sorry, you cannot become a spectator at this time."
     CantBePlayer="Sorry, you cannot become an active player at this time."
     BecameSpectator="became a spectator."
     StrTooMangRep="Too many Republic players."
     StrTooMangTran="Too many Trandoshan players."
     NewPlayerMessage="A new player entered the game."
     KickWarning="You are about to be kicked for idling!"
     NewSpecMessage="A spectator entered the game"
     SpecEnteredMessage=" joined as a spectator."
     bIsSpecial=False
}

