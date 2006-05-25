
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 5:1:2006   11:00
*/


#ifndef TileEditOperation_HEADER_INCLUDED // include guard
#define TileEditOperation_HEADER_INCLUDED  // include guard

#include "World.h"


class SelectedTile;

typedef std::list<SelectedTile*> selectedTile_list;

class SelectedTile
{
public:
	SelectedTile()
	{
		m_pTile = NULL;
	}
	~SelectedTile()
	{
		SAFE_DELETE(m_pTile);
	}
	
	const SelectedTile& operator=(const SelectedTile &rhs);
	SelectedTile(const SelectedTile &copyFrom);

	Tile *m_pTile;

private:

	
};

//this class holds currently selected tiles and copy/undo buffer of tile operations
class TileEditOperation
{
public:
	TileEditOperation();
	~TileEditOperation();


	const TileEditOperation& operator=(const TileEditOperation &rhs);
	TileEditOperation(const TileEditOperation &copyFrom);


	enum
	{
		C_OPERATION_TOGGLE,
		C_OPERATION_ADD,
		C_OPERATION_SUBTRACT
	};

	enum
	{
		PASTE_UPPER_LEFT,
		PASTE_CENTERED,

	};

	void PasteToWorld(CL_Vector2 vecWorld, int pasteOptions, TileEditOperation *pUndoOut);
	void ClearSelection();
	void AddTileToSelection(int operation, bool bPerformDupeCheck, Tile *pTile);
	selectedTile_list::iterator FindTileByLocation(selectedTile_list &tileList, Tile *pTile);
	void AddTilesByWorldRect(const CL_Vector2 &vecDragStart, const CL_Vector2 &vecDragStop, int operation, const vector<unsigned int> &layerIDVec);
	void AddTileByPoint(const CL_Vector2 &vecDragStart, int operation, const vector<unsigned int> &layerIDVec);
	CL_Vector2 GetSelectionSizeInWorldUnits();
	bool IsEmpty() {return m_selectedTileList.size() == 0;}

	void FillSelection(Tile *pTile); //using the selected positions, fills worldmap with tile sent in
	CL_Vector2 GetUpperLeftPos();
	CL_Vector2 GetLowerRightPos();
	void CopyTilePropertiesToSelection(Tile *pSrcTile, unsigned int partsToCopy);
	void ApplyOffset(CL_Vector2 vOffset); //moves all tiles it contains by this offset
	void SetLayerOfSelection(unsigned int layer);
	void SetForceLayerOfNextPaste(int layer) {m_forceLayerOfNextPaste = layer;}
	void SetIgnoreParallaxOnNextPaste() {m_bIgnoreParallaxOnNextPaste = true;}

	enum
	{
		eBitFlipX = D_BIT_0,
		eBitFlipY = D_BIT_1,
	};

	selectedTile_list m_selectedTileList;

protected:

	void AddWorldCoordToBoundsByTile(Tile *pTile);
	void AddWorldCoordToBounds(const CL_Vector2 &vecWorld);
	void SetNeedsFullBoundsCheck(bool bNew);
	void RecomputeBoundsIfNeeded();

 CL_Vector2 m_vecUpLeft, m_vecDownRight;
 bool m_bNeedsFullBoundsCheck;
 int m_forceLayerOfNextPaste; //i need an int because I want -1 to signal not active
bool m_bIgnoreParallaxOnNextPaste;
};

typedef std::deque<TileEditOperation> operation_deque;


#endif                  // include guard
