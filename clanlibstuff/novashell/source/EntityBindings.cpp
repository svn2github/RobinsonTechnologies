#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"
#include "AI/Goal_Think.h"
#include "ListBindings.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#include <luabind/object.hpp>

using namespace luabind;

string EntityToString(BaseGameEntity * pEnt)
{
	return "Entity " + CL_String::from_int(pEnt->ID()) + " (" + pEnt->GetName()+")";
}


TileList GetNearbyTileListForScript(MovingEntity *pEnt)
{
	return TileList(&pEnt->GetNearbyTileList());
}

void luabindEntity(lua_State *pState)
{
	module(pState)
		[

			class_<DataManager>("DataManager")
			.def("Delete", &DataManager::Delete)
			.def("Exists", &DataManager::Exists)
			.def("Get", &DataManager::Get)
			.def("Set", &DataManager::Set)
			.def("SetNum", &DataManager::SetNum)
			.def("GetNum", &DataManager::GetNum)
			.def("ModNum", &DataManager::ModNum)
			.def("Clear", &DataManager::Clear)
			.def("SetIfNull", &DataManager::SetIfNull)

			,class_<State>("State")
			.def("GetName", &State::GetName)

			,class_<BrainManager>("BrainManager")
			.def("Add", &BrainManager::Add)
			.def("SendToBrainByName", &BrainManager::SendToBrainByName)
			.def("Remove", &BrainManager::Remove)
			.def("AskBrainByName", &BrainManager::AskBrainByName)
			.def("SetStateByName", &BrainManager::SetStateByName)
			.def("GetStateByName", &BrainManager::GetStateByName)
			.def("LastStateWas", &BrainManager::LastStateWas)
			.def("InState", &BrainManager::InState)
			.def("SendToBrainBase", &BrainManager::SendToBrainBase)
			

			,class_<Brain>("Brain")
			.def("GetName", &State::GetName)

			,class_<BaseGameEntity>("BaseEntity")
			.def("GetID", &BaseGameEntity::ID)
			.def("GetName", &BaseGameEntity::GetName)
			.def("SetName", &BaseGameEntity::SetName)
			.def("__tostring", &EntityToString)
			.def("SetDeleteFlag", &BaseGameEntity::SetDeleteFlag)
			.def("Send", &BaseGameEntity::HandleMessageString)


/*

Object: Entity
The Entity object.

About:

The heart of the Novashell system is the flexible <Entity> object.

An <Entity> can represent a patch of ground, an chair, a persistent database, a warp, a health bar, a text overlay or a drooling monster.

Advanced abilities such as:

 * An attached script
 * The BrainManager
 * The GoalManager
 * Collision information
 * Pathfinding system
 * Triggers
 
are initialized on demand, only wasting memory if they are actually used.

Group: Member Functions
*/

			,class_<MovingEntity, BaseGameEntity>("Entity")
			.def(constructor<>())
			
			/*
			func: GetPos
			(code)
			Vector2 GetPos()
			(end)

			Returns:

			A <Vector2> object containing the entity's position.
			*/
			
			.def("GetPos", &MovingEntity::GetPosSafe)

			/*
			func: SetPos
			(code)
			nil SetPos(Vector2 vPos)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to.
			*/

			.def("SetPos", &MovingEntity::SetPos)
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
			.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
			.def("GetBrainManager", &MovingEntity::GetBrainManager)
			.def("__tostring", &EntityToString)
			.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)
			.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)
			.def("SetCanRotate", &MovingEntity::EnableRotation)
			.def("GetCanRotate", &MovingEntity::GetEnableRotation)
			.def("SetDensity", &MovingEntity::SetDensity)
			.def("SetListenCollision", &MovingEntity::SetListenCollision)
			.def("GetListenCollision", &MovingEntity::GetListenCollision)
			.def("SetListenCollisionStatic", &MovingEntity::SetListenCollisionStatic)
			.def("GetListenCollisionStatic", &MovingEntity::GetListenCollisionStatic)
			.def("SetMass", &MovingEntity::SetMass)
			.def("GetMass", &MovingEntity::GetMass)
			.def("SetDesiredSpeed", &MovingEntity::SetDesiredSpeed)
			.def("SetMaxMovementSpeed", &MovingEntity::SetMaxWalkSpeed)
			.def("SetTurnSpeed", &MovingEntity::SetTurnSpeed)
			.def("GetTurnSpeed", &MovingEntity::GetTurnSpeed)

			.def("SetGravityOverride", &MovingEntity::SetGravityOverride)
			.def("GetGravityOverride", &MovingEntity::GetGravityOverride)

			.def("Stop", &MovingEntity::Stop)
			.def("StopX", &MovingEntity::StopX)
			.def("StopY", &MovingEntity::StopY)
			.def("AddForce", &MovingEntity::AddForceBurst)
			.def("AddForceAndTorque", &MovingEntity::AddForceAndTorqueBurst)

			.def("AddForceConstant", &MovingEntity::AddForce)
			.def("AddForceAndTorqueConstant", &MovingEntity::AddForceAndTorque)

			.def("GetLinearVelocity", &MovingEntity::GetLinearVelocity)
			.def("SetPersistent", &MovingEntity::SetPersistent)
			.def("GetPersistent", &MovingEntity::GetPersistent)
			.def("GetOnLadder", &MovingEntity::GetOnLadder)
			.def("GetOnGround", &MovingEntity::IsOnGround)
			.def("GetOnGroundAccurate", &MovingEntity::IsOnGroundAccurate)
			.def("SetOnGround", &MovingEntity::SetIsOnGround)
			.def("SetDefaultTextColor", &MovingEntity::SetDefaultTextColor)
			.def("Data", &MovingEntity::GetData)
			.def("SetPosAndMapByTagName", &MovingEntity::SetPosAndMapByTagName)
			.def("OnDamage", &MovingEntity::OnDamage)
			.def("SetAnimByName", &MovingEntity::SetAnimByName)
			.def("SetSpriteByVisualStateAndFacing", &MovingEntity::SetSpriteByVisualStateAndFacing)
			.def("GetFacing", &MovingEntity::GetFacing)
			.def("SetFacing", &MovingEntity::SetFacing)

			.def("SetFacingTarget", &MovingEntity::SetFacingTarget)
			.def("GetFacingTarget", &MovingEntity::GetFacingTarget)

			.def("GetVectorFacing", &MovingEntity::GetVectorFacing)
			.def("GetVectorFacingTarget", &MovingEntity::GetVectorFacingTarget)
			.def("SetVectorFacing", &MovingEntity::SetVectorFacing)
			.def("SetVectorFacingTarget", &MovingEntity::SetVectorFacingTarget)
			.def("IsFacingTarget", &MovingEntity::IsFacingTarget)

			.def("SetVisualState", &MovingEntity::SetVisualState)
			.def("GetVisualState", &MovingEntity::GetVisualState)
			.def("GetLayerID", &MovingEntity::GetLayerID)
			.def("SetLayerID", &MovingEntity::SetLayerID)
			.def("SetLayerByName", &MovingEntity::SetLayerByName)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
			.def("FunctionExists", &MovingEntity::FunctionExists)
			.def("GetScale", &MovingEntity::GetScale)
			.def("SetScale", &MovingEntity::SetScale)
			.def("GetSizeX", &MovingEntity::GetSizeX)
			.def("GetSizeY", &MovingEntity::GetSizeY)

			.def("SetCollisionScale", &MovingEntity::SetCollisionScale)
			.def("GetCollisionScale", &MovingEntity::GetCollisionScale)
			.def("SetTrigger", &MovingEntity::SetTrigger)
			.def("DumpScriptInfo", &MovingEntity::DumpScriptInfo)
			.def("SetCollisionMode", &MovingEntity::SetCollisionMode)
	
			.def("GetDistanceFromEntityByID", &MovingEntity::GetDistanceFromEntityByID)
			.def("GetDistanceFromPosition", &MovingEntity::GetDistanceFromPosition)
			
			.def("SetBaseColor", &MovingEntity::SetBaseColor)
			.def("GetBaseColor", &MovingEntity::GetBaseColor)
			.def("GetGoalManager", &MovingEntity::GetGoalManager)
			.def("IsPlaced", &MovingEntity::IsPlaced)
			.def("SetNavNodeType", &MovingEntity::SetNavNodeType)
			.def("SetHasPathNode", &MovingEntity::SetHasPathNode)
			.def("SetVisibilityNotifications", &MovingEntity::SetVisibilityNotifications)

			.def("HasLineOfSightToPosition", &MovingEntity::CanWalkTo)
			.def("GetVectorToEntity", &MovingEntity::GetVectorToEntity)
			.def("GetVectorToEntityByID", &MovingEntity::GetVectorToEntityByID)
			.def("GetVectorToPosition", &MovingEntity::GetVectorToPosition)
			.def("IsOnSameMapAsEntityByID", &MovingEntity::IsOnSameMapAsEntityByID)
			.def("IsCloseToEntity", &MovingEntity::IsCloseToEntity)
			.def("IsCloseToEntityByID", &MovingEntity::IsCloseToEntityByID)
			.def("GetMap", &MovingEntity::GetMap)
			.def("PlaySound", &MovingEntity::PlaySound)
			.def("PlaySoundPositioned", &MovingEntity::PlaySoundPositioned)
			.def("IsValidPosition", &MovingEntity::IsValidPosition)
			.def("CanWalkTo", &MovingEntity::CanWalkTo)
			.def("SetAnimPause", &MovingEntity::SetAnimPause)
			.def("CreateEntity", &MovingEntity::CreateEntity)
			.def("SetAnimFrame", &MovingEntity::SetAnimFrame)
			.def("GetAnimFrame", &MovingEntity::GetAnimFrame)
			.def("SetText", &MovingEntity::SetText)
			.def("GetText", &MovingEntity::GetText)
			.def("GetTextBounds", &MovingEntity::GetTextBounds)
			.def("SetTextAlignment", &MovingEntity::SetTextAlignment)
			.def("SetTextColor", &MovingEntity::SetTextColor)
			.def("SetTextScale", &MovingEntity::SetTextScale)
			.def("GetTextScale", &MovingEntity::GetTextScale)
			.def("SetTextRect", &MovingEntity::SetTextRect)
			.def("SetAttach", &MovingEntity::SetAttach)
			.def("Clone", &MovingEntity::Clone)
			.def("SetAlignment", &MovingEntity::SetAlignment)
			.def("SetImage", &MovingEntity::SetImage)
			.def("SetImageByID", &MovingEntity::SetImageByID)
			.def("GetImageClipRect", &MovingEntity::GetImageClipRect)
			.def("GetImageID", &MovingEntity::GetImageID)
			.def("GetAttachedEntityID", &MovingEntity::GetAttachEntityID)
			.def("GetAttachOffset", &MovingEntity::GetAttachOffset)
			.def("SetAttachOffset", &MovingEntity::SetAttachOffset)
			.def("SetRunUpdateEveryFrame", &MovingEntity::SetRunUpdateEveryFrame)
			.def("GetRunUpdateEveryFrame", &MovingEntity::GetRunUpdateEveryFrame)
			.def("SetDampening", &MovingEntity::SetDampening)

			.def("InZoneByMaterialType", &MovingEntity::InZoneByMaterialType)
			.def("InNearbyZoneByMaterialType", &MovingEntity::InNearbyZoneByMaterialType)
			.def("GetActiveZoneByMaterialType", &MovingEntity::GetNearbyZoneByCollisionRectAndType)
			.def("GetNearbyZoneByMaterialType", &MovingEntity::GetNearbyZoneByPointAndType)
			.def("GetWorldCollisionRect", &MovingEntity::GetWorldCollisionRect)
			.def("GetCollisionRect", &MovingEntity::GetCollisionRect)

			.def("GetNearbyTileList", &GetNearbyTileListForScript)

			//Group -=-=-=-
	];
}


		
