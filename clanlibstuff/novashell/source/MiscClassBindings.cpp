
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
	if (!t) return false; //can't find it?
	if (t->m_entID == 0)
	{
		LogMsg("Found entity %s, but he hasn't been loaded yet. Hrm.  Returning NIL", name.c_str());
		return false; //not loaded yet
	}
	
	return (MovingEntity*)EntityMgr->GetEntityFromID(t->m_entID);
}

MovingEntity * GetEntityByWorldPos(CL_Vector2 v)
{
	if (!GetWorld)
	{

		LogMsg("GetEntityByWorldPos: Error, no world is active right now.");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	GetWorldCache->AddTilesByRect(recArea, &tileList, GetWorld->GetLayerManager().GetDrawList());

	//now we need to sort them
	tileList.sort(compareTileByLayer);

	if (tileList.rbegin() != tileList.rend())
	{
		pTile = (*tileList.rbegin());
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			//looks good to me!
			return ((TileEntity*)pTile)->GetEntity();
		}
	}

	//couldn't find anything
	return NULL;
}


//more class definitions
void luabindMisc(lua_State *pState)
{
	module(pState)
		[

		class_<CL_Color>("Color")
		.def(constructor<>())
		.def(constructor<unsigned int, unsigned int, unsigned int, unsigned int>())
		.def("GetRed", &CL_Color::get_red)
		.def("GetGreen", &CL_Color::get_green)
		.def("GetBlue", &CL_Color::get_blue)
		.def("Getalpha", &CL_Color::get_alpha)

		.def("SetRed", &CL_Color::set_red)
		.def("SetGreen", &CL_Color::set_green)
		.def("SetBlue", &CL_Color::set_blue)
		.def("Setalpha", &CL_Color::set_alpha)

		.def("Set", &CL_Color::set_color)

		,class_<MaterialManager>("MaterialManager")
		.def("AddMaterial", &MaterialManager::AddMaterial)
		.def("GetMaterial", &MaterialManager::GetMaterial)


		,class_<CMaterial>("Material")
		.def("SetType", &CMaterial::SetType)
		.def("GetType", &CMaterial::GetType)
		.def("SetSpecial", &CMaterial::SetSpecial)
		.def("GetSpecial", &CMaterial::GetSpecial)

		,class_<GameLogic>("GameLogic")
		.def("SetLeftMouseButtonCallback", &GameLogic::SetLeftMouseButtonCallback)
		.def("ToggleEditMode", &GameLogic::ToggleEditMode)
		.def("SetUserProfileName", &GameLogic::SetUserProfileName)
		.def("GetUserProfileName", &GameLogic::GetUserProfileName)
		.def("ResetUserProfile", &GameLogic::ResetUserProfile)
		.def("ClearAllMapsFromMemory", &GameLogic::ClearAllMapsFromMemory)
		.def("UserProfileExists", &GameLogic::UserProfileExists)
		.def("SetRestartEngineFlag", &GameLogic::SetRestartEngineFlag)
		.def("UserProfileActive", &GameLogic::UserProfileActive)
		.def("Quit", &GameLogic::Quit)

		,class_<App>("App")
		.def("SetGameLogicSpeed", &App::SetGameLogicSpeed)
		.def("SetGameSpeed", &App::SetGameSpeed)
		.def("GetGameTick", &App::GetGameTick)
		.def("GetTick", &App::GetTick)

		,class_<ISoundManager>("SoundManager")
		.def("PlayMusic", &ISoundManager::PlayMusic)
		.def("Play", &ISoundManager::Play)
		.def("PlayMixed", &ISoundManager::PlayMixed)
		.def("MuteAll", &ISoundManager::MuteAll)
		.def("KillMusic", &ISoundManager::KillMusic)
		.def("KillChannel", &ISoundManager::KillChannel)

		,class_<TextManager>("TextManager")
		.def("Add", &TextManager::Add)

		,class_<World>("Map")
		.def("SetPersistent", &World::SetPersistent)
		.def("GetPersistent", &World::GetPersistent)
		.def("SetAutoSave", &World::SetAutoSave)
		.def("GetAutoSave", &World::GetAutoSave)
		.def("GetName", &World::GetName)
		

		,class_<WorldManager>("MapManager")
		.def("SetActiveMapByName", &WorldManager::SetActiveWorldByName)
		.def("GetActiveMap", &WorldManager::GetActiveWorld)
		.def("UnloadMapByName", &WorldManager::UnloadWorldByName)
		
		,class_<TagManager>("TagManager")
		.def("GetFromString", &TagManager::GetFromString)
		.def("GetFromHash", &TagManager::GetFromHash)

		,class_<Camera>("Camera")
		.def("SetPos", &Camera::SetPos)
		.def("GetPos", &Camera::GetPos)
		.def("GetPosCentered", &Camera::GetPosCentered)
		.def("SetPosCentered", &Camera::SetPosCentered)
		.def("SetTargetPos", &Camera::SetTargetPos)
		.def("SetTargetPosCentered", &Camera::SetTargetPosCentered)
		.def("SetEntityTrackingByID", &Camera::SetEntTracking)
		.def("InstantUpdate", &Camera::InstantUpdate)
		.def("SetScale", &Camera::SetScale)
		.def("SetScaleTarget", &Camera::SetScaleTarget)
		.def("SetMoveLerp", &Camera::SetMoveLerp)
		.def("SetScaleLerp", &Camera::SetScaleLerp)
		.def("Reset", &Camera::Reset)
		.def("GetScale", &Camera::GetScale)

		,class_<TagObject>("TagObject")
		.def("GetMapName", &TagObject::GetMapName)
		.def("GetID", &TagObject::GetID)
		.def("GetPos", &TagObject::GetPos)

		,class_<ScriptKeyManager>("KeyManager")
		.def("AssignKey", &ScriptKeyManager::AssignKey)

		,
		//stand alone functions

		def("CreateEntity", &CreateEntity),
		def("Schedule", &Schedule),
		def("ScheduleSystem", &ScheduleSystem),
		def("ScreenToWorld", &ScreenToWorld),
		def("WorldToScreen", &WorldToScreen),
		def("GetEntityByWorldPos", &GetEntityByWorldPos),
		def("GetEntityByID", &GetEntityByID),
		def("GetEntityByName", &GetEntityByName),
		def("ShowMessage", &ShowMessage),
		def("FacingToVector", &FacingToVector)
		];
}
