//precompiled headers that shouldn't change much

#ifndef _APPPRE
#define _APPPRE

#pragma once

#include "lua/etc/lua.hpp"
#include <luabind/luabind.hpp>
//#include <luabind/operator.hpp>

#include <Clanlib/core.h>
#include <Clanlib/application.h>


#define Zone CarbonZone
#include <ClanLib/display.h>

#include <Clanlib/gl.h> 
#include <misc/MiscUtils.h>
#include <Clanlib/gui.h>
#include <Clanlib/guistylesilver.h>
#include <Clanlib/signals.h>
#undef Zone


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