class MusicTrigger extends Trigger
 	hidecategories(Sound);

var()	Sound	Episode;

function Touch( actor Other )
{
    if (IsRelevant(Other))
    {
		super.Touch(Other);
		PlayMusic( Episode );
	}
}


defaultproperties
{
}

