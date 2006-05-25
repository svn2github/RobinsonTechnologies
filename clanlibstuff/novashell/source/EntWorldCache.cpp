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

#ifdef _DEBUG
	if (!GetWorld->IsValidCoordinate(GetCamera->GetPos()))
	{
		LogMsg("Bad camera coord!");
		assert(0);
		GetCamera->Reset();
	}
#endif


	CL_Vector2 v;
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

//TODO: OPTIMIZE this later, we don't need all these look ups
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

void EntWorldCache::AddSectionToDraw(unsigned int renderID, CL_Rect &viewRect, vector<unsigned int>  & layerIDVec )
{
	//get all screens that we'll be able to see

	m_worldChunkVector.clear();
	m_pWorld->GetAllWorldChunksWithinThisRect(m_worldChunkVector, viewRect, false);

	tile_list *pTileList;
	tile_list::iterator tileListItor;
	Tile *pTile;

	//build the tile list
	for (EntWorldChunkVector::iterator itor = m_worldChunkVector.begin(); itor != m_worldChunkVector.end(); itor++)
	{
		for (unsigned int i=0; i < layerIDVec.size(); i++)
		{
			pTileList =  (*itor)->GetScreen()->GetTileList(layerIDVec[i]);

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
					m_tileLayerDrawList.push_back(pTile);
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

	vector<unsigned int> layerIdVecStandard; //default layers clumped together to process faster
	vector<unsigned int> layerIdVecSpecial; //special layers does separately
	bool bNoParallax = true;

	for (unsigned int i=0; i < layerMan.GetLayerCount(); i++)
	{
		
		Layer &layer = layerMan.GetLayerInfo(i);

		if (bMakingThumbnail)
		{
			if (!layer.GetUseInThumbnail()) continue;
		} else
		{
			//this way it's possible to have something ONLY rendered in the thumbnail
			if (!layer.IsDisplayed()) continue; //we're told not to show it
		}

		if (!GetGameLogic->GetEditorActive() && layer.GetShowInEditorOnly())
		{
			continue;
		}
		
		scrollMod = layer.GetScrollMod();

		 //|| !GetGameLogic->GetEditorActive())
		if ( GetGameLogic->GetParallaxActive() || (scrollMod !=  CL_Vector2::ZERO))
		{
			bNoParallax = false; //we need special handling
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
			AddSectionToDraw(renderID, modViewRect, layerIdVecSpecial);
			continue;
		}
	}

	//add all the normals
	AddSectionToDraw(renderID, viewRect, layerIdVecStandard);

	//sort the tiles/entities we're going to draw by layer
	std::sort(m_tileLayerDrawList.begin(), m_tileLayerDrawList.end(), compareTileByLayer);
}


void EntWorldCache::CullScreensNotUsedRecently(unsigned int timeRequiredToKeep)
{
	
}

//break this rect down into chunks to feed into the screens to get tile info
void EntWorldCache::AddTilesByRect(const CL_Rect &recArea, tile_list *pTileList, const vector<unsigned int> &layerIDVect)
{
	CL_Rect rec(recArea);
	rec.normalize();
    int startingX = rec.left;
	unsigned int scanID = GetApp()->GetUniqueNumber();
	CL_Rect scanRec;
	WorldChunk *pWorldChunk;
	CL_Rect screenRec;
	bool bScanMoreOnTheRight, bScanMoreOnTheBottom;

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
				}
			}
	 
			if (pTile->UsesTileProperties())
			{ 
				CollisionData col;
				//we need a customized version
				CreateCollisionDataWithTileProperties(pTile, col);
				RenderVectorCollisionData(pTile->GetPos(), col, pGC, false, NULL, pBody);
			} else
			{
				RenderVectorCollisionData(pTile->GetPos() , *pCol, pGC, false, NULL, pBody);
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

		for (unsigned int i=0; i < m_tileLayerDrawList.size(); i++)
		{
			m_tileLayerDrawList.at(i)->Update(step);
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



