class ACTION_SuggestEnemy extends ScriptedAction;

var(Action) name EnemyTag;

function bool InitActionFor(ScriptedController C)
{
	local Actor Enemy;
	local CTBot Bot;

	//Log("ACTION_SuggestEnemy::InitActionFor "$C.Pawn);
	if ( EnemyTag != '' )
	{
		Bot = CTBot(C);

		if (EnemyTag == 'None')
			Bot.SuggestEnemy(None);
		else
		{
			ForEach C.AllActors(class'Actor',Enemy,EnemyTag)
				break;

			if ( Enemy != None )
			{
				Bot.SuggestEnemy(Enemy);
			}
		}
	}
	return false;
}

function string GetActionString()
{
	if (Comment != "")
		return ActionString@EnemyTag$" // "$Comment;
	return ActionString@EnemyTag;
}


defaultproperties
{
     ActionString="Suggest Enemy"
     bValidForTrigger=False
}

