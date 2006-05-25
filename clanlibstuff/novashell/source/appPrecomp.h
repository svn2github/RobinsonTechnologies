//precompiled headers that shouldn't change much

#ifndef _APPPRE
#define _APPPRE

#ifdef __cplusplus

#include <ClanLib/core.h>
#include <ClanLib/application.h>

#define Zone CarbonZone
#define check CarbonCheck

#include <ClanLib/display.h>

#include <ClanLib/gl.h> 
#include <misc/MiscUtils.h>
#include <ClanLib/gui.h>
#include <ClanLib/guistylesilver.h>
#include <ClanLib/signals.h>
#undef Zone
#undef check

#include "lua/etc/lua.hpp"


#ifdef _WIN32
#include <float.h> //need FLT_MAX const
#endif

#include <misc/CL_VirtualFileManager.h>
#include "EntityManager.h"

using namespace std;

//#include "All.h"
//#include "std_all.h"
//#include "CBit8.h"

#include <list>
#include <string>
#include <vector>
#include <map>



#pragma warning (disable:4244)
#endif

#endif