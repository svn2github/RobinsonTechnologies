#include "AppPrecomp.h"
#include "Tile.h"
#include "Screen.h"
#include "CollisionData.h"
#include "MovingEntity.h"
#include "LayerManager.h"

#define C_DEFAULT_LAYER 1

Tile::Tile()
{
	m_lastScanID = 0;
	m_pParentScreen = NULL;
	m_type = C_TILE_TYPE_BLANK;
	m_layer = C_DEFAULT_LAYER;
	m_vecScale = CL_Vector2(1,1);
	m_pEdgeCaseList = NULL; //if not null, we're an edge case
	m_pFatherTile = NULL; //if not null, our type must be C_TILE_REFERENCE
	m_bitField.clear();
	m_pCollisionData = NULL;
	m_color = CL_Color(255,255,255);

}

CL_Rect Tile::GetWorldRectInt()
{
	static CL_Rectf r;
	r = GetWorldRect();
	return CL_Rect(r);
}

Tile::~Tile()
{
	if (m_pFatherTile)
	{
		//we need to remove outself from our parent, he exists because we still exist. (if
		//he was deleted, he would have deleted us already)
		m_pFatherTile->RemoveReference(this);
	} else
	{
		if (m_pEdgeCaseList)
		{
			//we have references floating around, eliminate them, mr jones
			std::list<Tile*>::iterator itor = m_pEdgeCaseList->begin();

			while (itor != m_pEdgeCaseList->end())
			{
				//by setting its parent blank, it won't attempt to notify us
				//that it is being deleted which would probably screw things up to no end
				(*itor)->m_pFatherTile = NULL;
				(*itor)->m_pParentScreen->RemoveTileByPointer(*itor);
				//no need to delete each link, will happen in a sec anyway
				itor++;
			}

			SAFE_DELETE(m_pEdgeCaseList);
		}
	}

	assert(!m_pEdgeCaseList);
}

const CL_Rect & Tile::GetBoundsRect()
{
	static CL_Rect r(0,0,64,64);
	return r;
}

Screen * Tile::GetParentScreen()
{
	return m_pParentScreen;
}
bool Tile::UsesTileProperties()
{
	CollisionData *pColData = GetCollisionData();
	if (pColData && pColData->GetLineList()->size() == 0) return false; //doesn't matter
	if ( GetBit(Tile::e_flippedX)  || GetBit(Tile::e_flippedY) || GetScale() != CL_Vector2(1,1))
	{
		return true;
	}

	return false;
}

void Tile::RemoveReference(Tile *pTileRef)
{
	assert(m_type != C_TILE_TYPE_REFERENCE && m_pEdgeCaseList && m_pEdgeCaseList->size() > 0);

	std::list<Tile*>::iterator itor = m_pEdgeCaseList->begin();

	while (itor != m_pEdgeCaseList->end())
	{
		if ((*itor) == pTileRef)
		{
			//bingo
			m_pEdgeCaseList->erase(itor);
			//all done
			break;
		}
		itor++;
	}

}

void Tile::SerializeBase(CL_FileHelper &helper)
{
	helper.process(m_layer);
	helper.process( *(cl_uint8*)&m_bitField);
	if (helper.IsWriting())
	{
		helper.process_const(GetPos());
	} else
	{
		helper.process(m_vecPos);
		SetPos(m_vecPos);
	}
	helper.process(m_vecScale);
	helper.process(*(cl_uint32*)&m_color);

}
//sometimes with blank/placeholders we need copy a blank tile
Tile * Tile::CreateClone()
{
	Tile *pNew = new Tile(); //this sets the type for us
	CopyFromBaseToClone(pNew);
	return pNew;
}

Tile * Tile::CopyFromBaseToClone(Tile *pNew)
{
	pNew->m_layer = m_layer;
	pNew->m_bitField = m_bitField;
	pNew->m_vecPos = m_vecPos;
	pNew->m_vecScale = m_vecScale;
	pNew->m_color = m_color;
	return pNew;
}

void Tile::SetScale(const CL_Vector2 &v)
{
	m_vecScale = v;
}
Tile * Tile::CreateReference(Screen *pScreen)
{
    Tile *pNew = new Tile();
	pNew->m_pParentScreen = pScreen;
	pNew->m_layer = m_layer;
	pNew->m_pFatherTile = this; //let it know who it is referencing
	pNew->m_type = C_TILE_TYPE_REFERENCE;

	//let our daddy know we exist and to kill us when he dies
	if (!m_pEdgeCaseList)
	{
		m_pEdgeCaseList = new std::list<Tile*>;
	}

	m_pEdgeCaseList->push_back(pNew);
	return pNew;
}

void RenderTilePic(TilePic *pTile, CL_GraphicContext *pGC)
{
	static CL_OpenGLSurface *pSurf;
	static CL_Vector2 vecScale;
	
	vecScale = GetCamera->GetScale();
	
	pSurf = (CL_OpenGLSurface*) GetHashedResourceManager->GetResourceByHashedID(pTile->m_resourceID);
	if (!pSurf) return; //error

	static CL_Vector2 vecPos;

	static bool bUseParallax;
	static Layer *pLayer;
	static World *pWorld;
	pWorld = pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
	static EntWorldCache *pWorldCache;
	
	pWorldCache = pWorld->GetMyWorldCache();


	if (GetGameLogic->GetParallaxActive())
	{
		bUseParallax = true;
		pLayer = &pWorld->GetLayerManager().GetLayerInfo(pTile->GetLayer());
		if (GetGameLogic->GetMakingThumbnail())
		{
			if (!pLayer->GetUseParallaxInThumbnail())
			{
				bUseParallax = false;
			}
		}
	} else
	{
		bUseParallax = false;
	}

	if (bUseParallax)
	{
		vecPos = pWorldCache->WorldToScreen(pWorldCache->WorldToModifiedWorld(pTile->GetPos(), 
		pLayer->GetScrollMod()));
	} else
	{
		vecPos = pWorldCache->WorldToScreen(pTile->GetPos());
	}

	static CL_Rectf rectDest;
	rectDest.left = vecPos.x;
	rectDest.top = vecPos.y;
	rectDest.right = vecPos.x+ ( float(pTile->m_rectSrc.get_width()) * vecScale.x * pTile->GetScale().x);
	rectDest.bottom = vecPos.y+ float(pTile->m_rectSrc.get_height()) * vecScale.y* pTile->GetScale().y;

	if (pTile->GetBit(Tile::e_flippedX))
	{
		pSurf->set_angle_yaw(-180);
	} else
	{
		pSurf->set_angle_yaw(0);
	}

	if (pTile->GetBit(Tile::e_flippedY))
	{
		pSurf->set_angle_pitch(-180);
	} else
	{
		pSurf->set_angle_pitch(0);
	}

	//fix holes that can appear when zoomed way in.  I suspect this is due to a pixel rounding error when
	//doing the blit? 
   
/*
	rectDest.bottom = int(rectDest.bottom);
	rectDest.right = int(rectDest.right);
	rectDest.top = int(rectDest.top);
	rectDest.left = int(rectDest.left);
*/

	//this fixes glitches with tiling when the scale isn't exactly 1.0

	rectDest.bottom = RoundNearest(rectDest.bottom, 1.0f);
	rectDest.right = RoundNearest(rectDest.right, 1.0f);
	rectDest.top = RoundNearest(rectDest.top, 1.0f);
	rectDest.left = RoundNearest(rectDest.left, 1.0f);

	pSurf->set_color(pTile->GetColor());
	CL_OpenGLWindow *pGLW = (CL_OpenGLWindow*) GetApp()->GetMainWindow();
	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

	pSurf->draw(pTile->m_rectSrc, rectDest, pGC);
}


void TilePic::Render(CL_GraphicContext *pGC)
{
	RenderTilePic(this, pGC);
}

CollisionData * TilePic::GetCollisionData()
{
	if (m_pCollisionData) return m_pCollisionData;

	//no collision data, let's make a default entry for it
	m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(m_resourceID, m_rectSrc);

	return m_pCollisionData;
}

const CL_Rect & TilePic::GetBoundsRect()
{
	static CL_Rect r(0,0,0,0);
	r.right = m_rectSrc.get_width()*m_vecScale.x;
	r.bottom = m_rectSrc.get_height()*m_vecScale.y;

	return r;
}

Tile * TilePic::CreateClone()
{
	//required
	TilePic *pNew = new TilePic;
	CopyFromBaseToClone(pNew);

	//custom things to copy
	pNew->m_resourceID = m_resourceID;
	pNew->m_rectSrc = m_rectSrc;
	pNew->m_rot = m_rot;
	pNew->m_color = m_color;
	//LogMsg("Copying resource %u", m_resourceID);
	return pNew;
}
void TilePic::Serialize(CL_FileHelper &helper)
{
	//base class saves
	SerializeBase(helper);

	//save our own data
	helper.process(m_rectSrc);
	helper.process(m_resourceID);
	helper.process(m_rot);
	
}


