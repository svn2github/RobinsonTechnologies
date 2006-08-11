
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 4:1:2006   13:45
*/


#ifndef EntEditMode_HEADER_INCLUDED // include guard
#define EntEditMode_HEADER_INCLUDED  // include guard

#include "main.h"
#include "Screen.h"
#include "TileEditOperation.h"

#define C_TILE_EDIT_UNDO_LEVEL 20

class EntCollisionEditor;
class DataObject;
class DataManager;

class EntEditMode: public BaseGameEntity
{
public:

	virtual void Render(void *pTarget);
	virtual void Update(float step);
	void SelectByLayer(const vector<unsigned int> &layerVec);
	void Init();

	EntEditMode();
    virtual ~EntEditMode();

protected:
	
	void ClearSelection();
    void Kill();
	void onButtonDown(const CL_InputEvent &key);
	void onButtonUp(const CL_InputEvent &key);
	void DrawActiveBrush(CL_GraphicContext *pGC);
	void OnDelete();
	void OnCopy();
	void OnCut();
	void OnPaste(TileEditOperation &editOperation, CL_Vector2 vecWorld);
	void OnUndo();
	void AddToUndo(TileEditOperation *pTileOperation);
	void UpdateMenuStatus();
	CL_Vector2 ConvertMouseToCenteredSelectionUpLeft(CL_Vector2 vecMouse);
	void OnClose(CL_SlotParent_v0 &parent_handler);
	void OnSelectBaseTilePage();
	void BuildBaseTilePageBox();
	void BuildTilePropertiesMenu(CL_Vector2 *pVecMouseClickPos, CL_Vector2 *pVecWorld, TileEditOperation *pTileList);
	void OnProperties();
	void SnapCheckBoxedClicked();
	void SnapSizeChanged();
	void OnMouseMove(const CL_InputEvent &key);
	void CutSubTile(CL_Rect recCut);
	void OnDefaultTileHardness();
	void OnCollisionDataEditEnd(int id);
	void OnSnapSizeChanged(const string &st);
	void BuildDefaultEntity();
	bool IsDialogOpen();
	void OffsetSelectedItems(CL_Vector2 vOffset, bool bBigMovement);
	virtual void HandleMessageString(const string &msg);
	void OnMapChange();
	void GetSettingsFromWorld();
	void ScaleUpSelected();
	void ScaleDownSelected();
	void ScaleSelection(CL_Vector2 vMod, bool bBigMovement);
	void OnSelectSimilar();
	void OnReplace();
	void OnDeleteBadTiles();

	CL_Slot m_slotClose;
	
	CL_SlotContainer m_slots;

	//for the GUI parts
	CL_Window *m_pWindow;
	CL_Label *m_pLabelSelection;
	CL_Label *m_pLabelMain;
	CL_CheckBox *m_pCheckBoxSnap;
	CL_InputBox *m_pInputBoxSnapSize; //what tile size we should emulate

	CL_Vector2 m_vecDragStart, m_vecDragStop;
	TileEditOperation m_selectedTileList;
	bool m_dragInProgress;

	operation_deque m_undo; //undo buffer
	CL_Menu *m_pMenu;
	CL_MenuNode *m_pMenuUndo;
	
	CL_Slot m_slotSelectedBaseTilePage;
	CL_Slot m_slotOpenBaseTileWindow;

	CL_ListBox *m_pListBaseTile;
	CL_Window * m_pWindowBaseTile;
	int m_snapSize;
	CL_Point m_vecLastMousePos;
	EntCollisionEditor * m_pEntCollisionEditor;
	Tile *m_pTileWeAreEdittingCollisionOn;
	CL_Vector2 m_collisionEditOldTilePos;
	MovingEntity *m_pOriginalEditEnt;
	bool m_bDialogIsOpen; //if true, we know not to respond to DELETE key presses to delete items and things

	float m_dragSnap;

	//for the properties dialog

	void OnPropertiesOK();
	void OnPropertiesOpenScript();
	void OnPropertiesEditScript();
	void OnPropertiesAddData();
	void OnPropertiesRemoveData();
	void OnPropertiesConvert();
	string PropertyMakeItemString(DataObject &o);
	void OnPropertiesEditData(const CL_InputEvent &input);
	void PropertiesSetDataManagerFromListBox(DataManager *pData, CL_ListBox &listBox);

	enum
	{
		C_GUI_CANCEL,
		C_GUI_OK,
		C_GUI_CONVERT

	};
	int m_guiResponse;
	CL_InputBox *m_pPropertiesInputScript;
	CL_ListBox *m_pPropertiesListData;

};


void DrawSelectedTileBorder(Tile *pTile, CL_GraphicContext *pGC);

extern TileEditOperation g_EntEditModeCopyBuffer; //needed so we can clear it before shutdown, otherwise
//weird things can happen because many subsystems are gone

#endif                  // include guard
