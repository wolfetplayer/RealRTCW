

#include "steam.h"
#include <stdint.h>
#ifdef WIN32
	#include "Windows.h"
#endif


const uint32_t steam_app_id = 1379630u;


HMODULE getLib(void){
#ifdef ARCH_32
	HMODULE mod = LoadLibraryA("steamcppwrappercustom32");
#endif
#ifdef ARCH_64
	HMODULE mod = LoadLibraryA("steamcppwrappercustom64");;
#endif
	return mod;
}

bool steamInit(void)
{ 
#ifdef WIN32
	HMODULE mod = getLib();
	FARPROC p1 = GetProcAddress(mod, "c_SteamAPI_Init");
	bool (*proc1)(void) 	= p1;
	bool q = p1();
	return q;
#endif
	return false;
}

bool steamRestartIfNecessary(void){
#ifdef WIN32
	HMODULE mod = getLib();
	FARPROC p2 = GetProcAddress(mod, "c_SteamAPI_RestartAppIfNecessary");
	bool (*proc2)(uint32_t) = p2;
	bool r = p2(steam_app_id);
	return r;
#endif
	return false;
}


