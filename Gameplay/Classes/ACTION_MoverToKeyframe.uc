class ACTION_MoverToKeyframe extends LatentScriptedAction;

var(Action)		int			KeyFrame;
var(Action)		name		MoverTag;
var(Action)		bool		bWaitUntilFinish;
var				bool		bBackwards;

function bool InitActionFor(ScriptedController C)
{
	local Mover m;
	local int	frame, i;
	local float Duration;

	if(MoverTag != 'None')
	{
		ForEach C.AllActors(class'Mover', m, MoverTag)
		{	
			if (Keyframe != m.KeyNum)
			{
				// right now, it doesn't loop.  will this be a feature we'd want?
				m.ScriptTriggeredKeyframe = Keyframe;

				Log("move to: "$keyframe$" currently "$m.KeyNum$" numkeys "$m.NumKeys);

				if (m.KeyNum > Keyframe)
					bBackwards = true;

				if (bBackwards)
				{
					//Log("backwards.  move to: "$keyframe$" currently "$m.KeyNum$" numkeys "$m.NumKeys);
					//get the time
					for (i=m.KeyNum; i > Keyframe; i--)
					{
						Duration += m.MoveTimes[i-1];
						Log("Duration "$Duration$" moveTimes["$i$"-1] "$m.MoveTimes[i-1]);
					}
					frame = m.KeyNum-1;
					if (frame < 0)
						frame = 0;	//m.NumKeys-1;
					if ( (frame >= 0) && (frame < m.NumKeys) )
						m.InterpolateTo(frame, m.MoveTimes[frame]);
				}
				else
				{
					//Log("Forward");
					//get the time
					for (i=m.KeyNum; i < Keyframe; i++)
						Duration += m.MoveTimes[i];
					frame = m.KeyNum+1;
					//Log("frame :"$frame$" m.NumKeys "$m.NumKeys);
					if (frame > m.NumKeys)
						frame = m.KeyNum;	//m.InterpolateTo(0, m.MoveTimes[m.ReturnLoopTime]);
					else if ( (frame > 0) && (frame <= m.NumKeys) )
						m.InterpolateTo(frame, m.MoveTimes[frame-1]);		
				}

				if (bWaitUntilFinish)
				{
					//Log("waituntilfinish - duration "$Duration);
					C.CurrentAction = self;
					C.SetTimer(Duration, false);
					return true;
				}
			}
		}
	}

	return false;
}

function bool CompleteWhenTimer()
{
	//Log("Action_MoverToKeyframe::CompleteWhenTimer");
	return true;
}

// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(MoverTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}



defaultproperties
{
     bWaitUntilFinish=True
     ActionString="Move mover to specified keyframe"
}

