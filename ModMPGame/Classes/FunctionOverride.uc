class FunctionOverride extends Object native;

/*
 * FunctionOverride
 * This allows overriding a specific function either for a single object or all objects of a specified class with another function from
 * a completely unrelated object.
 * Usage:
 * 1. Create a FunctionOverride instance for each function you want to override and store it in a class property so it is not garbage collected.
 *     KillEventOverride = new class'FunctionOverride';
 * 2. Initialize the override instance with the necessary information:
 *     KillEventOverride.Init(Level.Game, 'KillEvent', self, 'GameInfoKillEventOverride');
 *
 *     This will redirect all calls to Level.Game.KillEvent to self.KillEventOverride. It is very important that both of these functions
 *     have the exact same signature or else the game will crash (or at the very least glitch out).
 *     If you need to call the original function you can safely do that from within your override function.
 *     Instead of a specific objecy you can also pass a class to the Init function. This will affect all objects of that class instead of just a single one.
 */

var const Object   TargetObject;
var const Object   OverrideObject;
var const Function TargetFunction;
var const Function OverrideFunction;

var const noexport int OriginalFunc; // Native

native final function Init(Object TargetObj, name TargetFunc, Object OverrideObj, name OverrideFunc);
native final function Deinit();
native final function bool IsActive();

cpptext
{

	Native OriginalFunc;

	void Init(UObject* TargetObj, FName TargetFunc, UObject* OverrideObj, FName OverrideFunc);
	void Deinit();
	bool IsActive();

	// Overrides
	virtual void Destroy();
}
