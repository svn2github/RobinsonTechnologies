#include "AppPrecomp.h"
#include "EntWorldCache.h"
#include "GameLogic.h"
#include "EntCollisionEditor.h"
#include "TileEntity.h"
#include "MovingEntity.h"

EntWorldCache::EntWorldCache(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	 SetName("worldcache");
     m_pWorld = NULL;
	 m_cacheCheckTimer = 0;
	 SetDrawWorldChunkGrid(false);
	 SetDrawCollision(false);
#ifdef _DEBUG
	 //SetDrawCollision(true);
#endif
	 
}

void EntWorldCache::SetWorld( World * pWorld)
{
	m_pWorld = pWorld;
	pWorld->SetMyWorldCache(this);
}
EntWorldCache::~EntWorldCache()
{
    ClearCache();
	//let the main engine know the world cache is gone
}

void EntWorldCache::ClearCache()
{

}

//TODO: these should be rewritten for speed later
CL_Vector2 EntWorldCache::WorldToScreen(float x, float y)
{
	return WorldToScreen(CL_Vector2(x,y));
}

CL_Vector2 EntWorldCache::WorldToScreen(const CL_Vector2 &vecWorld)
{
	return CL_Vector2( (vecWorld.x-GetCamera->GetPos().x)*GetCamera->GetScale().x , (vecWorld.y-GetCamera->GetPos().y)*GetCamera->GetScale().y);
}

CL_Vector2 EntWorldCache::WorldToModifiedWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod)
{
	CL_Vector2 vCam = GetCamera->GetPosCentered();
	return CL_Vector2(vecWorld.x + vCam.x * vecMod.x, vecWorld.y + vCam.y * vecMod.y);
}

CL_Vector2 EntWorldCache::ModifiedWorldToWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod)
{
	CL_Vector2 vCam = GetCamera->GetPosCentered();
	return CL_Vector2(vecWorld.x - vCam.x * vecMod.x, vecWorld.y - vCam.y * vecMod.y);
}

CL_Vector2 EntWorldCache::ScreenToWorld(const CL_Vector2 &vecScreen)
{

/*
#ifdef _DEBUG
	if (!GetWorld->IsValidCoordinate(GetCamera->GetPos()))
	{
		LogMsg("Bad camera coord!");
		assert(0);
		GetCamera->Reset();
	}
#endif
*/

	static CL_Vector2 v;
	v.x =  (GetCamera->GetPos().x)+ (vecScreen.x/GetCamera->GetScale().x);
	v.y =  (GetCamera->GetPos().y)+ (vecScreen.y/GetCamera->GetScale().y);

	return v;
}

//returns true if actually generated a new thumb nail, false if the screen was blank
bool EntWorldCache::GenerateThumbnail(ScreenID screenID)
{
	assert(m_pWorld);
	WorldChunk *pWorldChunk = m_pWorld->GetWorldChunk(screenID);

	if (!pWorldChunk->GetNeedsThumbnailRefresh())
	{
		assert(!"Don't need a thumbnail generated, why was this called?");
		return false;
	}

	pWorldChunk->SetThumbNail(NULL);

	pWorldChunk->SetNeedsThumbnailRefresh(false);

	if (pWorldChunk->IsEmpty()) 
	{
		//um.. we don't REALLY need this... although we should fill it with black I guess.
		pWorldChunk->KillThumbnail();
		return false;
	}

	World *pWorld = pWorldChunk->GetParentWorld();
	short width = pWorld->GetThumbnailWidth();
	short height = pWorld->GetThumbnailHeight();

	if (width == 0)
	{
		//thumbnails are disabled
		return false; 
	}

	bool bWasOriginallyInMem = pWorldChunk->IsScreenLoaded();
	//Screen *pScreen = pWorldChunk->GetScreen();

	//make thumbnail

	CL_PixelBuffer *pbuf = new CL_PixelBuffer(width, height, width*3, C_THUMBNAIL_FORMAT);
	//make thumbnail
	CL_Rect rectSrc = CL_Rect(0,0, width,height);

	GetApp()->GetBackgroundCanvas()->get_gc()->clear();
	
	CameraSetting oldCamSettings;
	GetCamera->GetCameraSettings(oldCamSettings);
	

	//setup cam to render stuff in the perfect position
	GetCamera->SetTargetPos(CL_Vector2(pWorldChunk->GetRect().left, pWorldChunk->GetRect().top));
	GetCamera->InstantUpdate();
	CL_Vector2 vecScale(float(GetScreenX)/float(pWorld->GetWorldChunkPixelSize()),
		float(GetScreenY)/float(pWorld->GetWorldChunkPixelSize()));

	//hack it up to only draw to the upper left at the size we want
	vecScale.x = float(rectSrc.right)/float(pWorld->GetWorldChunkPixelSize());
	vecScale.y = float(rectSrc.bottom)/float(pWorld->GetWorldChunkPixelSize());
	GetCamera->SetScaleRaw(vecScale);
	

	//render out our stuff to it
	CalculateVisibleList(CL_Rect(0,0,width, height), true);

	//ugly, but I need a way for each sprite to know when to use special behavior like ignoring
	//parallax repositioning when making the thumbnail during its draw phase
	
	GetGameLogic->SetMakingThumbnail(true);
	RenderViewList(GetApp()->GetBackgroundCanvas()->get_gc());
	GetGameLogic->SetMakingThumbnail(false);

	GetApp()->GetBackgroundCanvas()->get_pixeldata(rectSrc).convert(*pbuf);
	
	pWorldChunk->SetThumbNail(pbuf);

	if (bWasOriginallyInMem)
	{
		//RemoveScreenFromCache(screenID);
		//pWorldChunk->UnloadScreen();
	}
	
	//set camera back to how it was
	GetCamera->SetCameraSettings(oldCamSettings);
	
	return true;
}

bool compareTileByLayer(const Tile *pA, const Tile *pB) 
{
	LayerManager *pLayerManager = &GetWorld->GetLayerManager();

	return pLayerManager->GetLayerInfo(pA->GetLayer()).GetSort() <
			pLayerManager->GetLayerInfo(pB->GetLayer()).GetSort();
}

void EntWorldCache::RemoveTileFromList(Tile *pTile)
{
	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
	MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();
	if (pEnt)
	for (unsigned int i=0; i < m_entityList.size(); i++)
	{
		if (m_entityList[i] == pEnt)
		{
		/*	
			if (m_entityList[i])
			{
				LogMsg("Removing ent %d from list", m_entityList[i]->ID());
			} else
			{
				LogMsg("Removing NULL tile from list");
			}
		  */

			m_entityList[i] = NULL;
			break;
		}
	}
	}

   //next remove  tile from our draw list
	for (unsigned int i=0; i < m_tileLayerDrawList.size(); i++)
	{
		if (m_tileLayerDrawList[i] == pTile)
		{
			//this is it
			m_tileLayerDrawList[i] = NULL;
		}
	}
}

void EntWorldCache::ProcessPendingEntityMovementAndDeletions()
{
	//process pending movement/deletions
  //	LogMsg("Processing %d ents", m_entityList.size());
	for (unsigned int i=0; i < m_entityList.size(); i++)
	{
		if (!m_entityList[i]) continue;
		if (m_entityList[i]->GetDeleteFlag())
		{
			//remove tile completely
			assert(m_entityList[i]->GetTile());
			m_entityList[i]->GetTile()->GetParentScreen()->RemoveTileByPointer(m_entityList[i]->GetTile());
		} else
		{
			m_entityList[i]->UpdateTilePosition();
		}
	}

}

void EntWorldCache::ResetPendingEntityMovementAndDeletions()
{
	m_entityList.clear();
}

void EntWorldCache::AddSectionToDraw(unsigned int renderID, CL_Rect &viewRect, vector<unsigned int>  & layerIDVec, bool bDontAddToDrawList )
{
	
	/*
	//slower way I was testing with
	
	//get all screens that we'll be able to see

	tile_list tileList;
	AddTilesByRect(viewRect, &tileList, layerIDVec);

	static tile_list::iterator itor;

	itor = tileList.begin();

	while (itor != tileList.end())
	{
		if (!bDontAddToDrawList)
		{
			m_tileLayerDrawList.push_back(*itor);
		}

		if ( (*itor)->GetType() == C_TILE_TYPE_ENTITY)
		{
			m_entityList.push_back(   ((TileEntity*)(*itor))->GetEntity()    );
		} 

		itor++;
	}
	*/

	//get all screens that we'll be able to see

	m_worldChunkVector.clear();
	m_pWorld->GetAllWorldChunksWithinThisRect(m_worldChunkVector, viewRect, false);

	tile_list *pTileList;
	tile_list::iterator tileListItor;
	Tile *pTile;
	Layer *pLayer;

	//build the tile list
	for (EntWorldChunkVector::iterator itor = m_worldChunkVector.begin(); itor != m_worldChunkVector.end(); itor++)
	{
		for (unsigned int i=0; i < layerIDVec.size(); i++)
		{
			pTileList =  (*itor)->GetScreen()->GetTileList(layerIDVec[i]);

			pLayer = &m_pWorld->GetLayerManager().GetLayerInfo(layerIDVec[i]);

			tileListItor = pTileList->begin();

			while (tileListItor != pTileList->end())
			{
				//add our tiles?
				pTile = *tileListItor;
				if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
				{
					pTile = pTile->GetTileWereAReferenceFrom();
				}

				if (pTile->GetLastScanID() != renderID)
				{
					pTile->SetLastScanID(renderID); //helps us not draw something twice, due to ghost references

					if (!pTile->GetWorldRectInt().is_overlapped(viewRect))
					{
						tileListItor++;
						continue;
					} 

					if (
						(!GetGameLogic->GetEditorActive() && pLayer->GetShowInEditorOnly())
						|| !pLayer->IsDisplayed() )
					{
						//don't render this
						
					} else
					{
						m_tileLayerDrawList.push_back(pTile);
					}

					if (pTile->GetType() == C_TILE_TYPE_ENTITY)
					{
						m_entityList.push_back( ((TileEntity*)pTile)->GetEntity());
					}
				}

				tileListItor++;
			}

		}
	}
}


//erm, never do this

//only used here!
LayerManager *g_pLayerManager;

bool compareTileBySortLevelOptimized(Tile *pA, Tile *pB) 
{
	static int aLayer, bLayer;
	static CollisionData *pCol;
	aLayer = g_pLayerManager->GetLayerInfo(pA->GetLayer()).GetSort();
	bLayer = g_pLayerManager->GetLayerInfo(pB->GetLayer()).GetSort();
	static float aBottom, bBottom;

	if (aLayer == bLayer)
	{
	
		//extra checking for depth queuing.  We depth cue by the bottom of the
		//collision box when possible, it's more accurate than the picture.

		pCol = pA->GetCollisionData();

		if (pCol && pCol->GetLineList()->size() > 0)
		{
			
			if (pA->GetType() == C_TILE_TYPE_ENTITY)
			{
				aBottom = pA->GetPos().y + (pCol->GetLineList()->begin()->GetRect().get_height()/2);
			} else
			{
				aBottom = pA->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);
			}
		} else
		{
			aBottom = pA->GetWorldRect().bottom;
		}
		pCol = pB->GetCollisionData();
	
		if (pCol && pCol->GetLineList()->size() > 0)
		{
			if (pB->GetType() == C_TILE_TYPE_ENTITY)
			{
				bBottom = pB->GetPos().y + (pCol->GetLineList()->begin()->GetRect().get_height()/2);
			} else
			{
				bBottom = pB->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);
			}
		} else
		{
			bBottom = pB->GetWorldRect().bottom;
		}

		return aBottom < bBottom;
	} else
	{
		return aLayer < bLayer;
	}
}


void EntWorldCache::CalculateVisibleList(const CL_Rect &recScreen, bool bMakingThumbnail)
{

	//viewable area in world coords
	CL_Rect viewRect;
	CL_Vector2 posTmp = ScreenToWorld(CL_Vector2(recScreen.left,recScreen.top));
	viewRect.left = int(posTmp.x);
	viewRect.top = int(posTmp.y);

	posTmp = ScreenToWorld(CL_Vector2(recScreen.right,recScreen.bottom));
	viewRect.right = int(posTmp.x);
	viewRect.bottom = int(posTmp.y);

	m_tileLayerDrawList.clear();
	ResetPendingEntityMovementAndDeletions();
	unsigned int renderID = GetApp()->GetUniqueNumber();

	//add the actual tile/entity data to our draw list
	LayerManager &layerMan = m_pWorld->GetLayerManager();
	CL_Vector2 scrollMod;

	static vector<unsigned int> layerIdVecStandard; //default layers clumped together to process faster
	static vector<unsigned int> layerIdVecSpecial; //special layers does separately
	
	layerIdVecStandard.clear();

	static bool bNoParallax;
	static bool bDontAddToDrawList;

	for (unsigned int i=0; i < layerMan.GetLayerCount(); i++)
	{
		Layer &layer = layerMan.GetLayerInfo(i);

		if (bMakingThumbnail)
		{
			if (!layer.GetUseInThumbnail()) continue;
		} else
		{
			//this way it's possible to have something ONLY rendered in the thumbnail
			if (!layer.IsDisplayed() && !layer.GetShowInEditorOnly()) continue; //we're told not to show it
		}

		bNoParallax = true;
		bDontAddToDrawList = false;
	
		scrollMod = layer.GetScrollMod();
			 
		if ( GetGameLogic->GetParallaxActive() && (scrollMod !=  CL_Vector2::ZERO))
		{
			bNoParallax = false; //we need special handling
		}
	
		if (layer.GetShowInEditorOnly())
		{
			if (!GetGameLogic->GetEditorActive() || !layer.IsDisplayed())
			{
				//well, we don't want to show this, but we do want its entities to get processed.
				bNoParallax = false;
				bDontAddToDrawList = true;
			}
		}
		
		if (bMakingThumbnail)
		{
			if (!layer.GetUseParallaxInThumbnail())
			{
				bNoParallax = true;
			}
		}

		if (bNoParallax)
		{
			layerIdVecStandard.push_back(i);
		} else
		{
			
			//requires special handling
			layerIdVecSpecial.clear();
			layerIdVecSpecial.push_back(i);
			//map rect to modified position
			CL_Rect modViewRect = viewRect;
			CL_Vector2 vOffset = WorldToModifiedWorld(CL_Vector2(0,0), scrollMod);
			modViewRect.apply_alignment(origin_top_left, vOffset.x, vOffset.y);
			AddSectionToDraw(renderID, modViewRect, layerIdVecSpecial, bDontAddToDrawList );
			continue;
		}
	}

	//LogMsg("Specials gave us %d tiles.", m_tileLayerDrawList.size());
	//add all the normals
	AddSectionToDraw(renderID, viewRect, layerIdVecStandard, false);

	//LogMsg("Sorting %d tiles.  %d entities found. %d normal layers.", m_tileLayerDrawList.size(), m_entityList.size(),
	//	layerIdVecStandard.size());
	//LogMsg("Viewrect is %s.  Drawing %d tiles", PrintRect(viewRect).c_str(), m_tileLayerDrawList.size());
	//sort the tiles/entities we're going to draw by layer
	g_pLayerManager = &GetWorld->GetLayerManager();
	std::sort(m_tileLayerDrawList.begin(), m_tileLayerDrawList.end(), compareTileBySortLevelOptimized);

	if (m_bDrawWorldChunkGrid)
	{
		m_worldChunkVector.clear();
		m_pWorld->GetAllWorldChunksWithinThisRect(m_worldChunkVector, viewRect, false);
	}

}

void EntWorldCache::CullScreensNotUsedRecently(unsigned int timeRequiredToKeep)
{
	
}

//break this rect down into chunks to feed into the screens to get tile info
void EntWorldCache::AddTilesByRect(const CL_Rect &recArea, tile_list *pTileList, const vector<unsigned int> &layerIDVect)
{
	CL_Rect rec(recArea);
	rec.normalize();
    static int startingX;
	unsigned int scanID = GetApp()->GetUniqueNumber();
	static CL_Rect scanRec;
	static WorldChunk *pWorldChunk;
	static CL_Rect screenRec;
	static bool bScanMoreOnTheRight, bScanMoreOnTheBottom;

	startingX = rec.left;

	while (1)
	{
		scanRec = rec;

		assert(rec.top <= rec.bottom && "this should never happen");
		//get the screen the upper left is on
		pWorldChunk = m_pWorld->GetWorldChunk(CL_Vector2(scanRec.left, scanRec.top));
		screenRec = pWorldChunk->GetRect();
		//truncate to the screen size if we need to
		
		if (scanRec.right > screenRec.right)
		{
			//we have more on our right to scan after this, truncate it for now
			scanRec.right = screenRec.right;
			bScanMoreOnTheRight = true;
		} else
		{
			bScanMoreOnTheRight = false;
		}

		if (scanRec.bottom > screenRec.bottom)
		{
			//we have more on our bottom to scan after this, truncate it for now
			scanRec.bottom = screenRec.bottom;
			bScanMoreOnTheBottom = true;
		} else
		{
			bScanMoreOnTheBottom = false;
		}
	
		pWorldChunk->GetScreen()->GetTilesByRect(scanRec, pTileList, layerIDVect, scanID);

		//do we have more to scan on our right after this?

		if (bScanMoreOnTheRight)
		{
			rec.left = scanRec.right;
		} else if (bScanMoreOnTheBottom)
			{
				rec.top = scanRec.bottom;
				rec.left = startingX;
			} else
		{
			//all done
			break;
		}
	}
}

void EntWorldCache::RenderViewList(CL_GraphicContext *pGC)
{
	Tile *pTile;

	for (unsigned int i=0; i < m_tileLayerDrawList.size(); i++)
	{
		pTile = m_tileLayerDrawList.at(i);
		if (!pTile) continue;

		pTile->Render(pGC);

	if (m_bDrawCollisionData)
	{
		if (pTile->GetCollisionData())
		{
			CollisionData *pCol = pTile->GetCollisionData();
			CL_Vector2 vOffset = CL_Vector2::ZERO;

			CBody *pBody = NULL;

			if (pCol->GetLineList()->size() > 0)
			{
				if (pTile->GetType() == C_TILE_TYPE_ENTITY)
				{
					pBody = ((TileEntity*)pTile)->GetEntity()->GetBody();
					vOffset = ((TileEntity*)pTile)->GetEntity()->GetVisualOffset();
				}
			}
	 
			if (pTile->UsesTileProperties())
			{ 
				CollisionData col;
				//we need a customized version
				CreateCollisionDataWithTileProperties(pTile, col);
				RenderVectorCollisionData(pTile->GetPos()+vOffset, col, pGC, false, NULL, pBody);
			} else
			{
				RenderVectorCollisionData(pTile->GetPos()+vOffset, *pCol, pGC, false, NULL, pBody);
			}
		}
	}
	} 

}

void EntWorldCache::Update(float step)
{
	m_uniqueDrawID = GetApp()->GetUniqueNumber();
	unsigned int gameTick = GetApp()->GetGameTick();
	if (gameTick > m_cacheCheckTimer)
	{
		unsigned int cacheCheckSpeed = 500;

		//time to check it
		//note, disabled for now
		//CullScreensNotUsedRecently(gameTick - cacheCheckSpeed);
		m_cacheCheckTimer = gameTick+cacheCheckSpeed;
	}

	ProcessPendingEntityMovementAndDeletions();
	CalculateVisibleList(CL_Rect(0,0,GetScreenX,GetScreenY), false);

	if (GetGameLogic->GetGamePaused() == false)
	{

		//note, if tiles ever actually need to DO something in their think phase, this should use
		//m_tileLayerDrawList instead of entityList
		for (unsigned int i=0; i < m_entityList.size(); i++)
		{
			m_entityList.at(i)->Update(step);
		}

		for (unsigned int i=0; i < m_entityList.size(); i++)
		{
			m_entityList.at(i)->PostUpdate(step);
		}
	}

}

void EntWorldCache::Render(void *pTarget)
{
	CL_GraphicContext* pGC = (CL_GraphicContext*)pTarget;
	
	RenderViewList(pGC);

	if (m_bDrawWorldChunkGrid)
	{
		CL_Rect recScreen;
		for (EntWorldChunkVector::iterator itor = m_worldChunkVector.begin(); itor != m_worldChunkVector.end(); itor++)
		{
			recScreen = (*itor)->GetRect();
			DrawRectFromWorldCoordinates(CL_Vector2(recScreen.left, recScreen.top),
				CL_Vector2(recScreen.right, recScreen.bottom), CL_Color(0,255,255,255), pGC);
		}
	}
}



