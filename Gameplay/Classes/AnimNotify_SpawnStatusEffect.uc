class AnimNotify_SpawnStatusEffect extends AnimNotify_Scripted;

var() class<StatusEffect>	StatusEffect;
var() float					Duration;
var() float					Radius;
var() float					DamagePerSecond;

event NotifyScript( Actor Owner )
{
    StatusEffect.Static.CreateStatusEffects( Owner, Duration, Radius, DamagePerSecond );	
}

defaultproperties
{
}

