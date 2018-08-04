#include "ModMPGamePrivate.h"

void ABotSupport::execTestNativeFunc(FFrame& Stack, void* Result){
	P_FINISH;
	MessageBox(NULL, "TESTNATIVEFUNC", "TEST", MB_OK);
}

void ABotSupport::execImportPaths(FFrame& Stack, void* Result){
	P_GET_STR(MapName);
	P_FINISH;
	MessageBox(NULL, "IMPORTPATHS", "IMPORT", MB_OK);
}