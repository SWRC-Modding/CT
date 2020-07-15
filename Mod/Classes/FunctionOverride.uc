class FunctionOverride extends Object native;

/*
 * FunctionOverride
 * This allows overriding a specific function either for a single object or all objects of a specified class with another function from
 * a completely unrelated object. The same function cannot be overridden multiple times!
 * Usage:
 * 1. Create a FunctionOverride instance for each function you want to override and store it in a class property so it is not garbage collected.
 *     KillEventOverride = new class'FunctionOverride';
 * 2. Initialize the override instance with the necessary information:
 *     KillEventOverride.Init(Level.Game, 'KillEvent', self, 'GameInfoKillEventOverride');
 *
 *     This will redirect all calls to Level.Game.KillEvent to self.KillEventOverride. It is very important that both of these functions
 *     have the exact same signature or else the game will crash (or at the very least glitch out).
 *     If you need to call the original function you can safely do that from within your override function.
 *     Instead of a specific object you can also pass a class to the Init function. This will affect all objects of that class instead of just a single one.
 */

var native const Object   TargetObject;
var native const Object   OverrideObject;
var native const Function TargetFunction;
var native const Function OverrideFunction;

var private const noexport int OriginalNative;
var private const noexport int OriginalFunctionFlags;

native final function Init(Object InTargetObject, name TargetFuncName, Object InOverrideObject, name OverrideFuncName);
native final function Deinit();

cpptext
{
	Native OriginalNative;
	DWORD  OriginalFunctionFlags;

	void Deinit();

	// Overrides
	virtual void Destroy();
}
