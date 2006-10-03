#include "AppPrecomp.h"
#include "BaseGameEntity.h"
#include "EntityManager.h"
#include "TileEntity.h"

int BaseGameEntity::m_iNextValidID = 101; //we reserve ID's less than this for special things

//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
BaseGameEntity::BaseGameEntity(int ID):m_iType(C_ENTITY_TYPE_BASE),
                                       m_bTag(false)
{
    m_deleteFlag = false;
	m_pTile = NULL;
    SetID(ID);
	EntityMgr->RegisterEntity(this);
}

BaseGameEntity::~BaseGameEntity()
{
	if (!m_deleteFlag)
	{
		//special case for when we're abruptly deleted without using SetDeleteFlag
		sig_delete(m_ID);
	}
	
	//remove from the index map
	if (m_pTile)
	{
		m_pTile->SetEntity(NULL); //so it won't try to delete us again
	}

	EntityMgr->RemoveEntity(this);
}

void BaseGameEntity::SetDeleteFlag(bool bNew)
{
	if (bNew && bNew != m_deleteFlag)
	{
		sig_delete(m_ID);
	}
	
	m_deleteFlag = bNew;	
}


//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
void BaseGameEntity::SetID(int val)
{
  //make sure the val is equal to or greater than the next available ID
  assert ( (val >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");
  m_ID = val;
  m_iNextValidID = m_ID + 1;
}
