//=============================================================================
// DamageVariance: this is a base class (and set of subclasses in Properties) 
// which describes what damage types deal an actor damage, and by how much;
// If something is in the include list, the amount of damage taken from that
// sort of thing is multiplied by the given factor; otherwise it is 1.0 (unless
// it appears in the exclude list)
//=============================================================================
class DamageVariance extends Object
	native
	abstract;


struct DamageIncludedInfo 
{
	var() class<DamageType>	DamageTypeClass;
	var() float             Multiplier;
};


var() editinline Array< DamageIncludedInfo > IncludeTypes;
var() Array< class<DamageType> >  ExcludeTypes;


defaultproperties
{
}

