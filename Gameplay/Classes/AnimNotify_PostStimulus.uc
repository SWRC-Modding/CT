class AnimNotify_PostStimulus extends AnimNotify_Scripted;

var() Actor.EStimulusType	Stimulus;
var() bool					PostOnlyToSelf;

event NotifyScript( Actor Owner )
{
	if( PostOnlyToSelf )
		Owner.PostStimulusToIndividual( Stimulus, Owner );
	else
		Owner.PostStimulusToWorld( Stimulus );
}

defaultproperties
{
}

