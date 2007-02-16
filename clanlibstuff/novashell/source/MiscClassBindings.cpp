
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"

#include "VisualProfile.h"
#include "MaterialManager.h"
#include "MessageManager.h"
#include "TextManager.h"
#include "TagManager.h"
#include "InputManager.h"
#include "VisualProfileManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "EntCreationUtils.h"
#include "ListBindings.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


using namespace luabind;




enum
{
	//don't modify the order, the scripting counts on it
	C_RAY_ENTITIES = 0,
	C_RAY_TILE_PIC,
	C_RAY_EVERYTHING,
	C_RAY_DEBUG_MODE
};



Zone GetCollisionByRay(Map * pMap, CL_Vector2 vStartPos, CL_Vector2 vFacing, int actionRange, int raySpread, MovingEntity *pEntToIgnore, int mode, bool bIgnoreCreatures)
{

	int tileScanMode = C_TILE_TYPE_BLANK; //everything

	Tile *pTileToIgnore = NULL;

	if (pEntToIgnore) pTileToIgnore = pEntToIgnore->GetTile();
	bool bDebug = false;

	switch (mode)
	{
	case C_RAY_EVERYTHING: break;
	case C_RAY_ENTITIES: tileScanMode = C_TILE_TYPE_ENTITY; break;
	case C_RAY_TILE_PIC: tileScanMode = C_TILE_TYPE_PIC; break;
	case C_RAY_DEBUG_MODE:
		bDebug = true;
		break;
	default:
		LogError("GetCollisionByRay reports unknown mode: %d", mode);
	}


	CL_Vector2 vEndPos;
	vEndPos = vStartPos + (vFacing * actionRange);

	CL_Vector2 vCross = vFacing.cross();
	CL_Vector2 vColPos;

	Tile *pTile = NULL;

	tile_list tilelist;

	CL_Vector2 scanRectTopLeft = vStartPos;
	CL_Vector2 scanRectBottomRight = vEndPos;


	if (raySpread != 0) 
	{
		switch (  VectorToFacing(vFacing) )
		{
		case VisualProfile::FACING_DOWN:
		case VisualProfile::FACING_UP:
		case VisualProfile::FACING_LEFT:
		case VisualProfile::FACING_RIGHT:

			scanRectTopLeft -= vCross * raySpread*4;
			scanRectBottomRight += vCross * raySpread*4;
			break;

		}
	}


	CL_Rect r(scanRectTopLeft.x, scanRectTopLeft.y, scanRectBottomRight.x, scanRectBottomRight.y);

	//expand it to cover the other places we're going to scan

	pMap->GetMyMapCache()->AddTilesByRect(r, &tilelist,pMap->GetLayerManager().GetCollisionList(), true);
	GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode, bIgnoreCreatures );

	if (bDebug)
	{
		DrawRectFromWorldCoordinates(CL_Vector2(r.left, r.top), CL_Vector2(r.right, r.bottom),  CL_Color::white, CL_Display::get_current_window()->get_gc());
		DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
	}

	if (raySpread != 0)
	{
		if (!pTile)
		{
			//try again
			vEndPos += vCross * raySpread;

			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

		if (!pTile)
		{
			//try again
			vEndPos += vCross * -(raySpread*2);
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}



		//try even more

		if (!pTile)
		{
			//try again
			vEndPos += vCross * (raySpread*3);
			vStartPos += vCross * raySpread;
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

		if (!pTile)
		{
			//try again
			vEndPos += vCross * -(raySpread*4);
			vStartPos += vCross * (-raySpread*2);
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode, bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

	}



	if (bDebug)
	{
		CL_Display::flip(2); //show it now
	}

	if (pTile)
	{

		Zone z;
		z.m_entityID = C_ENTITY_NONE;
		z.m_materialID = CMaterial::C_MATERIAL_TYPE_NONE;


		z.m_vPos = vColPos;
		z.m_boundingRect = pTile->GetWorldColRect();
		line_list::iterator litor = pTile->GetCollisionData()->GetLineList()->begin();

		z.m_materialID = litor->GetType(); //kind of a hack, we're assuming there is only one type here.. bad us

		if (z.m_materialID == CMaterial::C_MATERIAL_TYPE_DUMMY)
		{
			//fine, we'll do one more check..
			if (pTile->GetCollisionData()->GetLineList()->size() > 1)
			{
				litor++;
				z.m_materialID = litor->GetType(); 
			}

		}

		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			z.m_entityID = ((TileEntity*)pTile)->GetEntity()->ID();
		}
		//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		return z;
	}

	return g_ZoneEmpty;

}


TileList GetTileListByRect(Map *pMap, const CL_Rect &r, vector<unsigned int> &layerList, bool bWithCollisionOnly)
{
	tile_list tilelist;
	pMap->GetMyMapCache()->AddTilesByRect(r, &tilelist,layerList, bWithCollisionOnly);

	return (TileList(&tilelist));
}


//more class definitions
void luabindMisc(lua_State *pState)
{
	module(pState)
		[

/*
Object: Color
The color object stores R G B A color information.

About:
Internally the color is stored as a 32 bit integer.


The range for the colors and alpha is between 0 and 255.

Usage:

(code)
myColor = Color(); //instantiate it

myColor:Set(255,255,255,255); //we just set it to white.

//Or we can save time by doing:

myColor = Color(255,255,255,255);

(end)

*/
		class_<CL_Color>("Color")
	
		//Group: Initialization
		
		/*
		func: Color()
		(code)
		Color(number red, number green, number blue, number alpha)
		(end)
		Creates the object with default values. (everything will be 0)
		*/

		.def(constructor<>())

		//func: Color(r,g,b,a)
		
		.def(constructor<unsigned int, unsigned int, unsigned int, unsigned int>())

		//Group: Member Functions

		//func: GetRed
		//(code)
		//number GetRed()
		//(end)
		.def("GetRed", &CL_Color::get_red)

		//func: GetGreen
		//(code)
		//number GetGreen()
		//(end)
		.def("GetGreen", &CL_Color::get_green)

		//Function: GetBlue
		//(code)
		//number GetBlue()
		//(end)
		.def("GetBlue", &CL_Color::get_blue)

		/*
		Function: GetAlpha
		(code)
		number GetAlpha()
		(end)
		
		Returns:
		the alpha component. (0 is transparent, 255 is opaque/fully visible)
		*/

		.def("GetAlpha", &CL_Color::get_alpha)

		//func: SetRed
		//(code)
		//nil SetRed(number red)
		//(end)
		.def("SetRed", &CL_Color::set_red)
		//func: SetGreen
		//(code)
		//nil SetGreen(number green)
		//(end)
		.def("SetGreen", &CL_Color::set_green)
		//func: SetBlue
		//(code)
		//nil SetBlue(number blue)
		//(end)
		.def("SetBlue", &CL_Color::set_blue)
		//func: SetAlpha
		//(code)
		//nil SetAlpha
		//(end)
		.def("SetAlpha", &CL_Color::set_alpha)

		/*
		func: Set
		(code)
		nil Set(number red, number green, number blue, number alpha)
		(end)
		Sets all the values at once.
		*/
		
		.def("Set", &CL_Color::set_color)
		//Group: -=-=-=-=-=-
	
		,class_<MaterialManager>("MaterialManager")
		
		.def("AddMaterial", &MaterialManager::AddMaterial)
		.def("GetMaterial", &MaterialManager::GetMaterial)


		,class_<CMaterial>("Material")
		.def("SetType", &CMaterial::SetType)
		.def("GetType", &CMaterial::GetType)
		.def("SetSpecial", &CMaterial::SetSpecial)
		.def("GetSpecial", &CMaterial::GetSpecial)

		,class_<GameLogic>("GameLogic")
		.def("ToggleEditMode", &GameLogic::ToggleEditMode)
		.def("SetUserProfileName", &GameLogic::SetUserProfileName)
		.def("GetUserProfileName", &GameLogic::GetUserProfileName)
		.def("ResetUserProfile", &GameLogic::ResetUserProfile)
		.def("UserProfileExists", &GameLogic::UserProfileExists)
		.def("SetRestartEngineFlag", &GameLogic::SetRestartEngineFlag)
		.def("UserProfileActive", &GameLogic::UserProfileActive)
		.def("SetPlayer", &GameLogic::SetMyPlayer)
		.def("Quit", &GameLogic::Quit)
		.def("ClearModPaths", &GameLogic::ClearModPaths)
		.def("AddModPath", &GameLogic::AddModPath)
		.def("Data", &GameLogic::Data)
		.def("InitGameGUI", &GameLogic::InitGameGUI)

		,class_<App>("App")
//		.def("SetGameLogicSpeed", &App::SetGameLogicSpeed)
	//	.def("SetGameSpeed", &App::SetGameSpeed)
		.def("GetGameTick", &App::GetGameTick)
		.def("GetTick", &App::GetTick)
		.def("ParmExists", &App::ParmExists)
		.def("SetSimulationSpeedMod", &App::SetSimulationSpeedMod)
		.def("GetSimulationSpeedMod", &App::GetSimulationSpeedMod)
		.def("SetScreenSize", &App::SetScreenSize)
		.def("GetEngineVersion", &App::GetEngineVersion)
		.def("GetEngineVersionAsString", &App::GetEngineVersionAsString)
		.def("SetWindowTitle", &App::SetWindowTitle)
		.def("SetCursorVisible", &App::SetCursorVisible)
		.def("GetCursorVisible", &App::GetCursorVisible)
		.def("GetPlatform", &App::GetPlatform)

		,class_<ISoundManager>("SoundManager")
	//	.def("PlayMusic", &ISoundManager::PlayMusic)
		.def("Play", &ISoundManager::PlayMixed)
		//.def("PlayMixed", &ISoundManager::PlayMixed)
		.def("PlayLooping", &ISoundManager::PlayLooping)
		.def("MuteAll", &ISoundManager::MuteAll)
		//.def("KillMusic", &ISoundManager::KillMusic)
		.def("Kill", &ISoundManager::KillChannel)
		.def("AddEffect", &ISoundManager::AddEffect)
		.def("SetVolume", &ISoundManager::SetVolume)
		.def("RemoveAllEffects", &ISoundManager::RemoveAllEffects)
		.def("SetSpeedFactor", &ISoundManager::SetSpeedFactor)
		.def("SetVolume", &ISoundManager::SetVolume)

		,class_<TextManager>("TextManager")
		.def("Add", &TextManager::Add)
		.def("AddCustom", &TextManager::AddCustom)
		.def("AddCustomScreen", &TextManager::AddCustomScreen)


		,class_<LayerManager>("LayerManager")
		.def("GetLayerIDByName", &LayerManager::GetLayerIDByName)
		.def("GetCollisionLayers", &LayerManager::GetCollisionListNoConst)
		.def("GetVisibleLayers", &LayerManager::GetDrawListNoConst)
		.def("GetAllLayers", &LayerManager::GetAllListNoConst)
		
		,class_<Map>("Map")
		.def("SetPersistent", &Map::SetPersistent)
		.def("GetPersistent", &Map::GetPersistent)
		.def("SetAutoSave", &Map::SetAutoSave)
		.def("GetAutoSave", &Map::GetAutoSave)
		.def("GetName", &Map::GetName)
		.def("GetLayerManager", &Map::GetLayerManager)
		.def("BuildLocalNavGraph", &Map::BuildNavGraph)
		.def("GetCollisionByRay", &GetCollisionByRay)
		.def("GetTilesByRect", &GetTileListByRect)


		,class_<MapManager>("MapManager")
		.def("SetActiveMapByName", &MapManager::SetActiveMapByName)
		.def("GetActiveMap", &MapManager::GetActiveWorld)
		.def("UnloadMapByName", &MapManager::UnloadMapByName)
		.def("LoadMapByName", &MapManager::LoadMapByName)
		
		,class_<TagManager>("TagManager")
		.def("GetFromString", &TagManager::GetFromString)
		.def("GetFromHash", &TagManager::GetFromHash)
		.def("GetPosFromName", &TagManager::GetPosFromName)
		.def("RegisterAsWarp", &TagManager::RegisterAsWarp)

		,class_<Camera>("Camera")
		.def("SetPos", &Camera::SetPos)
		.def("GetPos", &Camera::GetPos)
		.def("GetPosCentered", &Camera::GetPosCentered)
		.def("SetPosCentered", &Camera::SetPosCentered)
		.def("SetTargetPos", &Camera::SetTargetPos)
		.def("SetTargetPosCentered", &Camera::SetTargetPosCentered)
		.def("SetEntityTrackingByID", &Camera::SetEntTracking)
		.def("GetEntityTrackingByID", &Camera::GetEntTracking)
		.def("InstantUpdate", &Camera::InstantUpdate)
		.def("SetScale", &Camera::SetScale)
		.def("SetScaleTarget", &Camera::SetScaleTarget)
		.def("SetMoveLerp", &Camera::SetMoveLerp)
		.def("SetScaleLerp", &Camera::SetScaleLerp)
		.def("Reset", &Camera::Reset)
		.def("GetScale", &Camera::GetScale)
		.def("GetCameraSettings", &Camera::GetCameraSettings)
		.def("SetCameraSettings", &Camera::SetCameraSettings)

		,class_<TagObject>("TagObject")
		.def("GetMapName", &TagObject::GetMapName)
		.def("GetID", &TagObject::GetID)
		.def("GetPos", &TagObject::GetPos)

		,class_<Zone>("Zone")
		.def_readwrite("boundingRect", &Zone::m_boundingRect)
		.def_readwrite("entityID", &Zone::m_entityID)
		.def_readwrite("materialID", &Zone::m_materialID)
		.def_readwrite("vPos", &Zone::m_vPos)

		,class_<CameraSetting>("CameraSettings")

		,class_<InputManager>("InputManager")
		.def("AddBinding", &InputManager::AddBinding)
		.def("RemoveBinding", &InputManager::RemoveBinding)
		.def("RemoveBindingsByEntity", &InputManager::RemoveBindingsByEntity)
		.def("GetMousePos", &InputManager::GetMousePos)
		.def("SetMousePos", &InputManager::SetMousePos)

		,class_<Goal_Think>("GoalManager")
		.def("PushMoveToPosition", &Goal_Think::PushMoveToPosition)
		.def("AddMoveToPosition", &Goal_Think::AddMoveToPosition)
		.def("RemoveAllSubgoals", &Goal_Think::RemoveAllSubgoals)
		.def("PushDelay", &Goal_Think::PushDelay)
		.def("AddDelay", &Goal_Think::AddDelay)
		.def("AddApproach", &Goal_Think::AddApproach)
		.def("AddApproachAndSay", &Goal_Think::AddApproachAndSay)
		.def("PushApproach", &Goal_Think::PushApproach)
		.def("PushApproachAndSay", &Goal_Think::PushApproachAndSay)

		.def("AddSay", ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::AddSay)
		.def("AddSay", ( void(Goal_Think::*)(const string&, int)) &Goal_Think::AddSay)

		.def("PushSay",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::PushSay)
		.def("PushSay",  ( void(Goal_Think::*)(const string&, int)) &Goal_Think::PushSay)
		
		.def("AddSayByID",  ( void(Goal_Think::*)(const string&, int,int,int)) &Goal_Think::AddSayByID)
		.def("AddSayByID",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::AddSayByID)

		.def("PushSayByID",  ( void(Goal_Think::*)(const string&, int,int,int)) &Goal_Think::PushSayByID)
		.def("PushSayByID",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::PushSayByID)

		.def("PushRunScriptString", &Goal_Think::PushRunScriptString)
		.def("AddRunScriptString", &Goal_Think::AddRunScriptString)

		.def("PushMoveToTag", &Goal_Think::PushMoveToTag)
		.def("AddMoveToTag", &Goal_Think::AddMoveToTag)

		.def("PushNewGoal", &Goal_Think::PushNewGoal)
		.def("AddNewGoal", &Goal_Think::AddNewGoal)

		.def("IsGoalActiveByName", &Goal_Think::IsGoalActiveByName)

		.def("GetGoalCountByName", &Goal_Think::GetGoalCountByName)
		.def("GetGoalCount", &Goal_Think::GetGoalCount)

		,class_<WatchManager>("WatchManager")
		.def("Add", &WatchManager::Add)
		.def("Remove", &WatchManager::Remove)
		.def("GetWatchCount", &WatchManager::GetWatchCount)
		
		
		];
}
