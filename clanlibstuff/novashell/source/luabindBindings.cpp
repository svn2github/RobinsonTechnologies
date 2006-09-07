#include "AppPrecomp.h"
#include "main.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

#include "VectorBindings.h"
#include "EntityBindings.h"
#include "MiscClassBindings.h"
#include "MaterialManager.h"
#include "TextManager.h"
#include "GameLogic.h"
#include "AI/WatchManager.h"

void RegisterLuabindBindings(lua_State *pLuaState)
{
	luabindVector(pLuaState);
	luabindEntity(pLuaState);
	luabindMisc(pLuaState);

	//add some global functions
	luabind::globals(pLuaState)["GetMaterialManager"] = &g_materialManager;
	luabind::globals(pLuaState)["GetApp"] = GetApp();
	luabind::globals(pLuaState)["GetSoundManager"] = g_pSoundManager;
	luabind::globals(pLuaState)["GetTextManager"] = &g_textManager;
	luabind::globals(pLuaState)["GetMapManager"] = GetGameLogic->GetMyWorldManager();

	luabind::globals(pLuaState)["GetGameLogic"] = GetGameLogic;
	luabind::globals(pLuaState)["GetTagManager"] = &g_TagManager;
	luabind::globals(pLuaState)["GetCamera"] = GetCamera;
	luabind::globals(pLuaState)["GetKeyManager"] = &g_keyManager;
	luabind::globals(pLuaState)["GetWatchManager"] = &g_watchManager;

	luabind::globals(pLuaState)["g_playerID"] = 0;	 //will be set later
#ifdef _DEBUG
	luabind::globals(pLuaState)["g_isDebug"] = true;	 //will be set later
#else
	luabind::globals(pLuaState)["g_isDebug"] = false;	 //will be set later
#endif

	luabind::globals(pLuaState)["C_SCREEN_X"] = GetScreenX;
	luabind::globals(pLuaState)["C_SCREEN_Y"] = GetScreenY;
	

}