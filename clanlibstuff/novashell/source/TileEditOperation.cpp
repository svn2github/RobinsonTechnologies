#include "AppPrecomp.h"
#include "TileEditOperation.h"
#include "EntWorldCache.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "MovingEntity.h"

const SelectedTile& SelectedTile::operator=(const SelectedTile &rhs)
{
  SAFE_DELETE(m_pTile);
  if (rhs.m_pTile)
  {
	  m_pTile = rhs.m_pTile->CreateClone();
  } 
  return *this;
}

SelectedTile::SelectedTile(const SelectedTile &copyFrom)
{
	m_pTile = NULL;
	*this = copyFrom;
}


TileEditOperation::TileEditOperation()
{
	ClearSelection();
}



TileEditOperation::TileEditOperation(const TileEditOperation &copyFrom)
{
	*this = copyFrom;
}

//copy the list sent into ourself, deleting all data that existed
const TileEditOperation& TileEditOperation::operator=(const TileEditOperation &rhs)
{
	ClearSelection();
	m_vecUpLeft = rhs.m_vecUpLeft;
	m_vecDownRight = rhs.m_vecDownRight;
	m_bNeedsFullBoundsCheck = rhs.m_bNeedsFullBoundsCheck;
	m_forceLayerOfNextPaste = rhs.m_forceLayerOfNextPaste;
	m_bIgnoreParallaxOnNextPaste = rhs.m_bIgnoreParallaxOnNextPaste;
	selectedTile_list::const_iterator itor = rhs.m_selectedTileList.begin();

	SelectedTile *pSelectedTile = NULL;

	for (; itor != rhs.m_selectedTileList.end(); (itor++))
	{
		pSelectedTile = new SelectedTile;
		(*pSelectedTile) = *(*itor);
		m_selectedTileList.push_back(pSelectedTile);
	}
	return *this;
}


TileEditOperation::~TileEditOperation()
{
	ClearSelection();
}

void TileEditOperation::SetNeedsFullBoundsCheck(bool bNew)
{
	m_bNeedsFullBoundsCheck = bNew;
}
void TileEditOperation::ClearSelection()
{
	selectedTile_list::iterator itor = m_selectedTileList.begin();

	while (itor != m_selectedTileList.end())
	{
		delete (*itor);
		itor++;
	}

	m_selectedTileList.clear();
	
	m_bNeedsFullBoundsCheck = false;
	m_vecUpLeft = CL_Vector2(FLT_MAX, FLT_MAX);
	m_vecDownRight = CL_Vector2(-FLT_MAX,-FLT_MAX);
	m_forceLayerOfNextPaste = -1; //disable it
	m_bIgnoreParallaxOnNextPaste = false;
}

CL_Vector2 TileEditOperation::GetUpperLeftPos()
{
	RecomputeBoundsIfNeeded();
	return m_vecUpLeft;
}
CL_Vector2 TileEditOperation::GetLowerRightPos()
{
	RecomputeBoundsIfNeeded();
	return m_vecDownRight;
}


void TileEditOperation::RecomputeBoundsIfNeeded()
{
	if (!m_bNeedsFullBoundsCheck) return;
	m_bNeedsFullBoundsCheck = false;
	m_vecUpLeft = CL_Vector2(FLT_MAX, FLT_MAX);
	m_vecDownRight = CL_Vector2(-FLT_MAX,-FLT_MAX);
	LogMsg("Recomputing full bounds..");
	selectedTile_list::iterator itor = m_selectedTileList.begin();

	while (itor != m_selectedTileList.end())
	{
		AddWorldCoordToBoundsByTile((*itor)->m_pTile);
		itor++;
	}
}

//it makes a clone of the tile sent
void TileEditOperation::AddTileToSelection(int operation, bool bPerformDupeCheck, Tile *pTile)
{
//you know, we better do a quick check.  If this tile already exists in the list, let's delete it, so it
	//acts like a toggle.
	
	if (bPerformDupeCheck)
	{
	
	selectedTile_list::iterator itor = FindTileByLocation(m_selectedTileList, pTile);

	if (itor != m_selectedTileList.end())
	{
		//a dupe was found. 

		switch (operation)
		{
		case C_OPERATION_TOGGLE:
		case C_OPERATION_SUBTRACT:
			//we just want to delete it
		
			delete (*itor);
			m_selectedTileList.remove(*itor);
			SetNeedsFullBoundsCheck(true);
			return;

		case C_OPERATION_ADD:
			//keep it
			return;

		default:

			assert(!"error!");
		}
	}
	}

	if (operation == C_OPERATION_SUBTRACT)
	{
		//we don't want to actually add it
		SetNeedsFullBoundsCheck(true);
		return;
	}
	
	if (!pTile)
	{
		assert(!"Don't send this blanks yet");
		//get this ourself based on the world coords we've got
		//pTileList = GetWorld->GetScreen(screenID)->GetTileLi(ptTile);
	}
	SelectedTile *pSelTile = new SelectedTile;
	pSelTile->m_pTile = pTile->CreateClone();
	
	m_selectedTileList.push_back(pSelTile);
	AddWorldCoordToBoundsByTile(pTile);
}

void TileEditOperation::AddWorldCoordToBoundsByTile(Tile *pTile)
{

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		TileEntity *pEntTile = (TileEntity*) pTile;

		//special handling to deal with the complicated offsets a sprite can have

		CL_Rectf r(pTile->GetPos().x, pTile->GetPos().y, pTile->GetPos().x + pEntTile->GetEntity()->GetSizeX(), pTile->GetPos().y + pEntTile->GetEntity()->GetSizeY());
		CL_Origin origin;
		int x,y;
		pEntTile->GetEntity()->GetAlignment(origin,x,y);
		CL_Pointf offset = calc_origin(origin, r.get_size());
		r -= offset;
		
		AddWorldCoordToBounds(CL_Vector2(r.left, r.top)); //this rounds it off to the tile we're on
		AddWorldCoordToBounds(CL_Vector2(r.right, r.bottom));
		return;
	}

	//default handling
	AddWorldCoordToBounds(pTile->GetPos()); //this rounds it off to the tile we're on
	AddWorldCoordToBounds(pTile->GetPos() + pTile->GetBoundsSize());
}	


void TileEditOperation::AddTileByPoint(const CL_Vector2 &vecDragStart, int operation, const vector<unsigned int> &layerIDVec)
{
	Tile *pTile = NULL;
	bool bPerformDupeCheck = false;
	if (!IsEmpty()) bPerformDupeCheck = true;

	CL_Rect recArea(vecDragStart.x, vecDragStart.y, vecDragStart.x + 0.1f, vecDragStart.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	GetWorldCache->AddTilesByRect(recArea, &tileList, layerIDVec);

	//now we need to sort them
	tileList.sort(compareTileByLayer);

	if (tileList.rbegin() != tileList.rend())
	{
		pTile = (*tileList.rbegin())->CreateClone();
		if (pTile)
		{
			AddTileToSelection(operation, bPerformDupeCheck, pTile);
		}
		SAFE_DELETE(pTile);
	}

}


void TileEditOperation::AddTilesByWorldRect(const CL_Vector2 &vecDragStart, const CL_Vector2 &vecDragStop, int operation, const vector<unsigned int> &layerIDVec)
{
	Tile *pTile = NULL;

	bool bPerformDupeCheck = false;
	if (!IsEmpty()) bPerformDupeCheck = true;

	CL_Rect recArea(vecDragStart.x, vecDragStart.y, vecDragStop.x, vecDragStop.y);
	
	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;
	
	GetWorldCache->AddTilesByRect(recArea, &tileList, layerIDVec);

	tile_list::iterator itor = tileList.begin();
	while (itor != tileList.end())
	{
		pTile = (*itor)->CreateClone();
		if (pTile)
		{
			AddTileToSelection(operation, bPerformDupeCheck, pTile);
		}
		itor++;
	}

}

selectedTile_list::iterator TileEditOperation::FindTileByLocation(selectedTile_list &tileList, Tile *pTile)
{
	selectedTile_list::iterator itor = tileList.begin();

	while (itor != tileList.end())
	{
		if ((*itor)->m_pTile->GetPos() == pTile->GetPos())
		{
			//found it
			return itor;
		}
		itor++;
	}
	return itor;
}


CL_Vector2 TileEditOperation::GetSelectionSizeInWorldUnits()
{
	assert(GetUpperLeftPos().x != FLT_MAX && "Maybe we should send back 0 if empty");
	return GetLowerRightPos()-GetUpperLeftPos();
}
void TileEditOperation::AddWorldCoordToBounds(const CL_Vector2 &vecWorld)
{
	//look at each tile we're storing and compute the offset needed to the center of the group

		m_vecUpLeft.x = min(m_vecUpLeft.x, vecWorld.x);
		m_vecUpLeft.y = min(m_vecUpLeft.y, vecWorld.y);

		m_vecDownRight.x = max(m_vecDownRight.x, vecWorld.x);
		m_vecDownRight.y = max(m_vecDownRight.y, vecWorld.y);

		
}

//if pUndoOut isn't null, an undo is placed there of whatever is done
void TileEditOperation::PasteToWorld(CL_Vector2 vecWorld, int pasteOptions, TileEditOperation *pUndoOut)
{
	
	LayerManager &layerMan = GetWorld->GetLayerManager();
	if (layerMan.GetEditActiveList().size() == 0)	
	{
		CL_MessageBox::info("Warning", "Nothing will be pasted because no edit layer is active", GetApp()->GetGUI());
		return;
	}
	assert(!IsEmpty());
	selectedTile_list::const_iterator itor = m_selectedTileList.begin();
	CL_Vector2 vecDestTileWorld;
	bool bOverWroteAnotherTile;
	Screen *pScreen = NULL;
	CL_Vector2 vecOffset;

	switch (pasteOptions)
	{
	case PASTE_CENTERED:
		vecOffset = GetUpperLeftPos() + (GetSelectionSizeInWorldUnits()/2);
		break;
	case PASTE_UPPER_LEFT:
		vecOffset = GetUpperLeftPos();
		break;
	default:
		assert(!"ARGHGHGH");
	}
	while (itor != m_selectedTileList.end())
	{
		vecDestTileWorld = vecWorld - vecOffset;
		vecDestTileWorld += (*itor)->m_pTile->GetPos();

		unsigned int layerToUse = (*itor)->m_pTile->GetLayer();

		unsigned int originalLayer = layerToUse;
		//if the tile we want to paste to doesn't exist in the active layer list, let's use the next best one
		if (!layerMan.GetLayerInfo(layerToUse).IsEditActive())
		{
			//uh oh, find the next closest one
			layerToUse = layerMan.GetEditActiveList().at(0);
		}

		if (m_forceLayerOfNextPaste != -1)
		{
			//hack to allow changing layers of a selection yet still keep the undo on the right
			//layer.  
			layerToUse = m_forceLayerOfNextPaste;
		}
		if (GetGameLogic->GetParallaxActive() && !m_bIgnoreParallaxOnNextPaste)
		{
			Layer &layer = GetWorld->GetLayerManager().GetLayerInfo(layerToUse);
			if (layer.GetScrollMod().x != 0 || layer.GetScrollMod().y != 0)
			{
				//as a help to the guy pasting, let's modify its coords to compensate for the parallax scroll display
				vecDestTileWorld = GetWorldCache->ModifiedWorldToWorld(vecDestTileWorld, layer.GetScrollMod());
			}
		}

		//kill whatever was here
		Screen *pScreen = GetWorld->GetScreen(vecDestTileWorld);
		Tile *pOldTile = pScreen->GetTileByPosition(vecDestTileWorld, originalLayer);

		if (pOldTile)
		{
			bOverWroteAnotherTile = true;	
			//let's add this tile to the undo and kill it
			Tile *pClone = pOldTile->CreateClone();
			pClone->SetLayer(originalLayer);
			if (pUndoOut) pUndoOut->AddTileToSelection(C_OPERATION_ADD, false, pClone);
			SAFE_DELETE(pClone);
			pScreen->RemoveTileByPosition(vecDestTileWorld, originalLayer);
			pScreen->GetParentWorldChunk()->SetDataChanged(true);
			pScreen->GetParentWorldChunk()->SetNeedsThumbnailRefresh(true);

		} else bOverWroteAnotherTile = false;
	

		if ((*itor)->m_pTile->GetType() == C_TILE_TYPE_BLANK)
		{
	
		} else
		{
			if (!bOverWroteAnotherTile && pUndoOut)
			{
				//first save a command to delete the tile we place
				Tile *pBlank = new Tile;
				pBlank->SetLayer(layerToUse);
				pBlank->SetPos(vecDestTileWorld);
				pUndoOut->AddTileToSelection(C_OPERATION_ADD, false, pBlank);
				SAFE_DELETE(pBlank);
			}

			Tile *pTile = (*itor)->m_pTile->CreateClone();
			pTile->SetLayer(layerToUse);
			//move it to the new position and layer if required
			pTile->SetPos(vecDestTileWorld);
			GetWorld->AddTile(pTile);
		}
		itor++;
	}

	m_forceLayerOfNextPaste = -1; //disable it again in case it was enabled
	m_bIgnoreParallaxOnNextPaste = false;
}

void TileEditOperation::FillSelection(Tile *pTile)
{
	selectedTile_list::const_iterator itor = m_selectedTileList.begin();
	CL_Vector2 vecTileWorld;
	Screen *pScreen = NULL;
	Tile *pTileTmp;

	while (itor != m_selectedTileList.end())
	{
		pScreen = GetWorld->GetScreen((*itor)->m_pTile->GetPos());
		pScreen->RemoveTileBySimilarType((*itor)->m_pTile);
		
		//add the new tile as long as it isn't a blank
		if (pTile->GetType() != C_TILE_TYPE_BLANK)
		{
			pTileTmp = pTile->CreateClone();
			pTileTmp->SetPos((*itor)->m_pTile->GetPos());
			pTileTmp->SetLayer((*itor)->m_pTile->GetLayer());
		}

		pScreen->GetParentWorldChunk()->SetDataChanged(true);
		pScreen->GetParentWorldChunk()->SetNeedsThumbnailRefresh(true);
		
		
		itor++;
	}
}

void TileEditOperation::CopyTilePropertiesToSelection(Tile *pSrcTile, unsigned int flags)
{
	selectedTile_list::iterator itor = m_selectedTileList.begin();
	Tile *pDestTile;

	//tile_list::iterator tileItor;
	while (itor != m_selectedTileList.end())
	{
			pDestTile = (*itor)->m_pTile;
			
			//is the is the kind of tile we should paste these properties to?
			if (flags & eBitFlipX) pDestTile->SetBit(Tile::e_flippedX, pSrcTile->GetBit(Tile::e_flippedX));
			if (flags & eBitFlipY) pDestTile->SetBit(Tile::e_flippedY, pSrcTile->GetBit(Tile::e_flippedY));
			if (flags & eBitColor) pDestTile->SetColor(pSrcTile->GetColor());
			
	itor++;
	}
}


void TileEditOperation::SetLayerOfSelection(unsigned int layer)
{
	selectedTile_list::iterator itor = m_selectedTileList.begin();
	Tile *pDestTile;

	//tile_list::iterator tileItor;
	while (itor != m_selectedTileList.end())
	{
		pDestTile = (*itor)->m_pTile;

		pDestTile->SetLayer(layer);
		itor++;
	}
}

void TileEditOperation::ApplyOffset(CL_Vector2 vOffset)
{
	selectedTile_list::iterator itor = m_selectedTileList.begin();
	Tile *pTile;

	while (itor != m_selectedTileList.end())
	{
		pTile = (*itor)->m_pTile;

		pTile->SetPos(pTile->GetPos() + vOffset);
		itor++;
	}

//and finally, adjust our bounding boxes

	m_vecUpLeft += vOffset;
	m_vecDownRight += vOffset;

}