#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

string MovingEntityToString(MovingEntity * pEnt)
{
	return "Entity " + CL_String::from_int(pEnt->ID()) + pEnt->GetName();
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


			,class_<MovingEntity>("Entity")
			.def(constructor<>())
			.def("ID", &MovingEntity::ID)
			.def("GetPos", &MovingEntity::GetPos)
			.def("SetPos", &MovingEntity::SetPos)
			.def("GetName", &MovingEntity::GetName)
			.def("SetName", &MovingEntity::SetName)
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
			.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
			.def("AddBrain", &MovingEntity::AddBrain)
			.def("__tostring", &MovingEntityToString)
			.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)
			.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)
			.def("EnableRotation", &MovingEntity::EnableRotation)
			.def("SetDensity", &MovingEntity::SetDensity)
			.def("SetListenCollision", &MovingEntity::SetListenCollision)
			.def("GetListenCollision", &MovingEntity::GetListenCollision)
			.def("SetListenCollisionStatic", &MovingEntity::SetListenCollisionStatic)
			.def("GetListenCollisionStatic", &MovingEntity::GetListenCollisionStatic)
			.def("SetDeleteFlag", &MovingEntity::SetDeleteFlag)
			.def("SetMass", &MovingEntity::SetMass)
			.def("GetMass", &MovingEntity::GetMass)
			.def("AddForce", &MovingEntity::AddForce)
			.def("AddForceAndTorque", &MovingEntity::AddForceAndTorque)
			.def("GetLinearVelocity", &MovingEntity::GetLinearVelocity)
			.def("SetPersistant", &MovingEntity::SetPersistant)
			.def("GetPersistant", &MovingEntity::GetPersistant)
			.def("InZoneByMaterialType", &MovingEntity::InZoneByMaterialType)
			.def("GetOnLadder", &MovingEntity::GetOnLadder)
			.def("SetDefaultTextColor", &MovingEntity::SetDefaultTextColor)
			.def("Data", &MovingEntity::GetData)
			.def("SetPosAndMapByName", &MovingEntity::SetPosAndMapByName)

		];
}


		
