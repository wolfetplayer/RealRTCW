

#include "steam.h"

#include "../steamshim/steamshim_child.h"

#include <stdint.h>

#ifdef WIN32
	#include "windows.h"
#endif


const uint32_t steam_app_id = 1379630u;

void steamRun(void)
{
#ifdef WIN32

	STEAMSHIM_pump();
	
#endif

	return;
}

int steamInit(void)
{ 
#ifdef WIN32

	return STEAMSHIM_init();

#endif
	return false;
}

bool steamRestartIfNecessary(void)
{
#ifdef WIN32

	STEAMSHIM_restartIfNecessary(steam_app_id);

	while (STEAMSHIM_alive())
	{
		const STEAMSHIM_Event *e = STEAMSHIM_pump();

	    if (e && e->type == SHIMEVENT_APPRESTARTED)
	        return e->okay;

	    usleep(100 * 1000); // 1/10sec
	}

#endif

	return false;
}

void steamSetAchievement(const char* id)
{
#ifdef WIN32

	STEAMSHIM_setAchievement(id, 1);

#endif

	return;
}

bool steamAlive()
{
#ifdef WIN32

	return STEAMSHIM_alive();

#endif

	return false;
}
