#include "../../build/build.h"

#if COMPILER != COMPILER_MSVC
	#error "Unsupported compiler"
#endif

begin_project(CT) {
	Target mod;
	Target modMpGame;
	Target openglDrv;
	Target ct;

	/*
	 * Common options
	 */

	install_directory("../GameData/System");

	if(config() == Debug) {
		defines("DO_CHECK_SLOW=1,"
		        "DO_GUARD_SLOW=1,"
		        "DO_CLOCK_SLOW=1");
	} else {
		defines("DO_CHECK_SLOW=0,"
		        "DO_GUARD_SLOW=0,"
		        "DO_CLOCK_SLOW=0");
	}

	compile_flags("/EHsc,"
	              "/MD,"
	              "/W4,"
	              "/Gr");

	include_paths("CTGame/Inc,"
	              "CTMarkers/Inc,"
                "Core/Inc,"
                "D3DDrv/Inc,"
                "Editor/Inc,"
                "Engine/Inc,"
                "GameSpyMgr/Inc,"
                "Gameplay/Inc,"
                "IpDrv/Inc,"
                "Mod/Inc,"
                "ModMPGame/Inc,"
                "Window/Inc/ XGame/Inc,"
                "XInterface/Inc");

	/*
	 * Mod
	 */

	project_directory("Mod");

	mod = shared_library("Mod",
		"Src/ExportBumpMapsCommandlet.cpp,"
		"Src/FunctionOverride.cpp,"
		"Src/ModClasses.cpp,"
		"Src/ModRenderDevice.cpp,"
		"Src/SWRCFix.cpp");
	target_defines(mod, "MOD_API=DLL_EXPORT");
	target_link_libraries(mod, "user32");

	/*
	 * ModMPGame
	 */

	project_directory("ModMPGame");

	modMpGame = shared_library("ModMPGame",
		"Src/ModMPGame.cpp,"
		"Src/ModMPGameClasses.cpp");
	target_defines(modMpGame, "MODMPGAME_API=DLL_EXPORT");

	/*
	 * OpenGLDrv
	 */

	project_directory("OpenGLDrv");

	openglDrv = shared_library("OpenGLDrv",
		"Src/OpenGLMaterial.cpp,"
		"Src/OpenGLRenderDevice.cpp,"
		"Src/OpenGLRenderInterface.cpp,"
		"Src/OpenGLResource.cpp,"
		"Src/Shader.cpp,"
		"Src/ShaderConversion.cpp,"
		"Src/ShaderGenerator.cpp");
	target_defines(openglDrv, "OPENGLDRV_API=DLL_EXPORT");
	target_link_libraries(openglDrv, "user32 gdi32");

	/*
	 * UCC
	 */

	project_directory("UCC");

	executable("UCC",
		"Src/ucc.cpp,"
		"Src/ServerCommandlet.cpp,");

	/*
	 * CT
	 */

	project_directory("CT");

	ct = executable("CT",
		"Src/Launcher.cpp,"
		"Res/Launcher.rc");
	target_link_libraries(ct, "user32");

} end_project
