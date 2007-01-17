#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"

#include "VisualProfile.h"
#include "MaterialManager.h"
#include "MessageManager.h"
#include "TextManager.h"
#include "TagManager.h"
#include "ScriptKeyManager.h"
#include "VisualProfileManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "EntCreationUtils.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


using namespace luabind;


CL_Vector2 ScreenToWorld(CL_Vector2 v)
{
	if (GetWorld)
	{
		return GetWorldCache->ScreenToWorld(v);
	}	

	LogMsg("ScreenToWorld error: No world active");
	return v;

}

CL_Vector2 WorldToScreen(CL_Vector2 v)
{
	if (GetWorld)
	{
		return GetWorldCache->WorldToScreen(v);
	} 

	LogMsg("WorldToScreen error: No world active");
	return v;
}

MovingEntity * GetEntityByID(int ID)
{
	return (MovingEntity*)EntityMgr->GetEntityFromID(ID);
}


MovingEntity * GetEntityByName(const string &name)
{
	TagObject *t = GetTagManager->GetFromString(name);
	if (!t) return NULL; //can't find it?
	if (t->m_entID == 0)
	{
		//let's load it right now
		GetWorldManager->LoadWorld(t->m_pWorld->GetDirPath(), false);
		t->m_pWorld->PreloadMap();

		if (t->m_entID == 0)
		{
			LogMsg("GetEntityByName: Entity %s couldn't be located.  Outdated tag? Removing it.", name.c_str());
			GetTagManager->RemoveByHashID(t->m_hashID, 0);
			return NULL; //not loaded yet
		}
	}

	return (MovingEntity*)EntityMgr->GetEntityFromID(t->m_entID);
}

BaseGameEntity * GetSpecialEntityByName(const string &name)
{
	return EntityMgr->GetEntityByName(name);
}

int GetEntityIDByName(const string &name)
{
	MovingEntity *pEnt = GetEntityByName(name);

	if (pEnt) return pEnt->ID();
	return 0;
}

Tile * GetTileByWorldPos(World *pWorld, CL_Vector2 v, vector<unsigned int> layerIDVec, bool bPixelAccurate)
{
	if (!pWorld)
	{

		LogMsg("GetTileByWorldPos: Error, invalid map");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	pWorld->GetMyWorldCache()->AddTilesByRect(recArea, &tileList, layerIDVec);

	//now we need to sort them
	g_pLayerManager = &pWorld->GetLayerManager();
	tileList.sort(compareTileBySortLevelOptimized);

	tile_list::reverse_iterator itor = tileList.rbegin();

	for( ;itor != tileList.rend(); itor++)
	{
		pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY || pTile->GetType() == C_TILE_TYPE_PIC)
		{

			//looks good to me!

			//looks good to me!

			if (!bPixelAccurate || PixelAccurateHitDetection(v, pTile))
			{
				return pTile;
			} else
			{
				continue;
			}

		}
	}

	//couldn't find anything
	return NULL;

}



MovingEntity * GetEntityByWorldPos(CL_Vector2 v, MovingEntity *pEntToIgnore, bool bPixelAccurate)
{
	if (!GetWorld)
	{

		LogMsg("GetEntityByWorldPos: Error, no map is active right now.");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	GetWorldCache->AddTilesByRect(recArea, &tileList, GetWorld->GetLayerManager().GetDrawList());

	//now we need to sort them
	g_pLayerManager = &GetWorld->GetLayerManager();
	tileList.sort(compareTileBySortLevelOptimized);

	tile_list::reverse_iterator itor = tileList.rbegin();
	MovingEntity *pEnt;

	for( ;itor != tileList.rend(); itor++)
	{
		pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			pEnt = ((TileEntity*)pTile)->GetEntity();

			if (pEnt->GetMap()->GetLayerManager().GetLayerInfo(pTile->GetLayer()).GetShowInEditorOnly())
			{
				continue;
			}
			if (pEnt == pEntToIgnore)
			{
				continue;
			}
			//looks good to me!

			if (!bPixelAccurate || PixelAccurateHitDetection(v, pTile))
			{
				return pEnt;
			} else
			{
				continue;
			}
		}
	}

	//couldn't find anything
	return NULL;
}

void LogErrorLUA(const char *pMessage)
{
	string s = pMessage;
	if (s.length() > C_LOGGING_BUFFER_SIZE)
	{
		s.resize(C_LOGGING_BUFFER_SIZE);
	}
	LogError(s.c_str());
}

bool RunScript(const string &scriptName)
{

	string fileName = scriptName;

	if (!g_VFManager.LocateFile(fileName))
	{
		LogError("Unable to locate script file %s.", fileName.c_str());
		return false;
	}

	GetScriptManager->LoadMainScript(fileName.c_str());
	return true;
}


/*
Section: Global Functions
These functions are global and can be run from anywhere, they aren't attached to any specific object.
*/

void luabindGlobalFunctions(lua_State *pState)
{
	module(pState)
		[

			//stand alone functions
			
			//func: CreateEntity
			
			//Creates a new entity at a specific location.  It expects the position and script to use.
			def("CreateEntity", &CreateEntity),

			//func: Schedule
			
			//Schedule a piece of script to be run by an entity at a specific time.
			
			def("Schedule", &Schedule),
			
			//func: ScheduleSystem
			//
			def("ScheduleSystem", &ScheduleSystem),
			//func: ScreenToWorld
			//
			def("ScreenToWorld", &ScreenToWorld),
			def("WorldToScreen", &WorldToScreen),
			def("GetEntityByWorldPos", &GetEntityByWorldPos),
			def("GetEntityByID", &GetEntityByID),
			def("GetEntityByName", &GetEntityByName),
			def("GetSpecialEntityByName", &GetSpecialEntityByName),
			def("GetEntityIDByName", &GetEntityIDByName),
			def("ShowMessage", &ShowMessage),
			def("FacingToVector", &FacingToVector),
			def("VectorToFacing", &VectorToFacing),
			def("LogError", &LogErrorLUA),
			def("RunScript", &RunScript),
			def("CreateEntitySpecial", &CreateEntitySpecial),
			def("ColorToString", &ColorToString),
			def("StringToColor", &StringToColor),
			def("VectorToString", &VectorToString),
			def("StringToVector", &StringToVector),
			def("RectToString", &RectToString),
			//func: StringToRect
			//
			def("StringToRect", &StringToRect)

			//Group: -=-=-=-=-=-
		];
}