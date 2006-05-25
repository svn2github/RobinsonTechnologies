#include "AppPrecomp.h"
#include "Tile.h"
#include "Screen.h"
#include "CollisionData.h"
#include "MovingEntity.h"
#include "LayerManager.h"
#include "TileEntity.h"

TileEntity::TileEntity()
{
	m_type = C_TILE_TYPE_ENTITY;
	m_pEntity = NULL;
	//SetBit(Tile::e_notPersistant, true); //don't save this, we don't know how yet
	SetLayer(C_LAYER_ENTITY);
}

TileEntity::~TileEntity()
{
	SAFE_DELETE(m_pEntity);
}

void TileEntity::Serialize(CL_FileHelper &helper)
{

	if (!helper.IsWriting())
	{
		//we're loading so we have to do some extra init stuff
		this->SetEntity(new MovingEntity);
	}

	//base class saves
	SerializeBase(helper);


	assert(m_pEntity);
	//save our own data
	m_pEntity->Serialize(helper);
}

void TileEntity::Render(CL_GraphicContext *pGC)
{
	m_pEntity->Render(pGC);
}

void TileEntity::Update(float step)
{
	m_pEntity->Update(step);
}

Tile * TileEntity::CreateClone()
{
	//required
	TileEntity *pNew = new TileEntity;
	CopyFromBaseToClone(pNew);

	//custom things to copy
	if (m_pEntity)
	{
		assert(m_pEntity);
		pNew->m_pEntity = (MovingEntity*) m_pEntity->CreateClone();
		assert(pNew->m_pEntity && "No cloning function created for this entity type yet");
		pNew->m_pEntity->SetTile(pNew);
	}
	return pNew;
}

CL_Vector2 TileEntity::GetBoundsSize()
{
	return CL_Vector2(m_pEntity->GetSizeX(),m_pEntity->GetSizeY());
}

CL_Rect TileEntity::GetBoundsRect()
{
	CL_Rectf rf(m_pEntity->GetWorldRect());
	CL_Rect r(rf.left, rf.top, rf.right, rf.bottom);
	return r;

}

CL_Rectf TileEntity::GetWorldRect()
{
	return m_pEntity->GetWorldRect();

}
const CL_Vector2 & TileEntity::GetPos()
{
	assert(m_pEntity);
	return m_pEntity->GetPos();
}

void TileEntity::SetPos(const CL_Vector2 &vecPos)
{
	assert(m_pEntity);
	m_pEntity->SetPos(vecPos);
}

CollisionData * TileEntity::GetCollisionData()
{
	return m_pEntity->GetCollisionData();
}
void TileEntity::SetEntity(MovingEntity *pEnt)
{
	m_pEntity = pEnt;
	if (m_pEntity)	
	{
		m_pEntity->SetTile(this);
	}
}

CBody * TileEntity::GetCustomBody()
{
	assert(m_pEntity);
	return m_pEntity->GetBody();
}