
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "ListBindings.h"
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


#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

TileList::TileList()
{
	m_bItorInitted = false;
}

TileList::TileList(tile_list *pTileList)
{
	
	m_bItorInitted = false;

	tile_list::iterator itor=pTileList->begin();

	while (itor != pTileList->end())
	{
		m_tileList.push_back(*itor);

		itor++;
	}
}

int TileList::GetCount()
{
	return m_tileList.size();
}

Tile * TileList::GetNext()
{
	if (!m_bItorInitted)
	{
		ResetNext();
		m_itor = m_tileList.begin();
	}


	if (m_itor != m_tileList.end())
	{
		
		return * (m_itor++);
	}

	ResetNext();
	return NULL; //all done
}

void TileList::ResetNext()
{
	m_bItorInitted = true;
	m_itor = m_tileList.begin();

}

int NumberGet(vector<unsigned int> *pVec, unsigned int index)
{
	if (index > pVec->size())
	{
		LogError("Index %d is invalid in NumberList, it only has %d items.", index, pVec->size());
			return 0;
	}

	return pVec->at(index);
}

using namespace luabind;

void luabindList(lua_State *pState)
{
	module(pState)
		[

			class_<TileList>("TileList")
			.def(constructor<>())
			.def("GetNext", &TileList::GetNext)
			.def("ResetNext", &TileList::ResetNext)
			.def("GetCount", &TileList::GetCount)
			,
			
			class_< vector<unsigned int> >("LayerList")
			.def(constructor<>())
			.def("Add", &vector<unsigned int>::push_back)
			.def("GetCount", &vector<unsigned int>::size)
			.def("Get",  &NumberGet)


		];
}