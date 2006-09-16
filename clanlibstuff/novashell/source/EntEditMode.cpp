#include "AppPrecomp.h"
#include "GameLogic.h"
#include "EntEditMode.h"
#include "EntEditor.h"
#include "EntWorldCache.h"
#include "EntCollisionEditor.h"
#include "HashedResource.h"
#include "TileEntity.h"
#include "MovingEntity.h"
#include "MessageManager.h"
#include "VisualProfileManager.h"
#include "EntVisualProfileEditor.h"

#define C_SCALE_MOD_AMOUNT 0.01f

TileEditOperation g_EntEditModeCopyBuffer; //ctrl-c puts it here

EntEditMode::EntEditMode(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pMenu = NULL;
	m_pOriginalEditEnt = NULL;
	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntEditMode::onButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntEditMode::onButtonUp);
	m_slots.connect (CL_Keyboard::sig_key_down(), this, &EntEditMode::onButtonDown);
	m_slots.connect (CL_Keyboard::sig_key_up(), this, &EntEditMode::onButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntEditMode::OnMouseMove);
	m_slots.connect(GetGameLogic->GetMyWorldManager()->sig_map_changed, this, &EntEditMode::OnMapChange);

	m_pTileWeAreEdittingCollisionOn = NULL;
	m_dragSnap = 1.0f;
	m_pWindow = NULL;
	m_pLabelMain = NULL;
	m_pLabelSelection = NULL;
	m_pCheckBoxSnap = NULL;
	m_pInputBoxSnapSize = NULL;
	
	m_pWindowBaseTile = NULL;
	m_pListBaseTile = NULL;

	m_dragInProgress = false;
	m_vecDragStart = m_vecDragStop = CL_Vector2(0,0);
	m_pEntCollisionEditor = NULL;
	m_bDialogIsOpen = false;
	m_pPropertiesInputScript= NULL;
	m_pPropertiesListData = NULL;
	
	SetName("edit mode");
	Init();
}

EntEditMode::~EntEditMode()
{
	Kill();
}

void EntEditMode::OnMouseMove(const CL_InputEvent &key)
{
	CL_Vector2 mouseWorldPos =GetWorldCache->ScreenToWorld(CL_Vector2(key.mouse_pos.x,key.mouse_pos.y));
	char stTemp[256];
	sprintf(stTemp, "Mouse Pos: X: %.1f Y: %.1f", mouseWorldPos.x, mouseWorldPos.y);
	m_pLabelMain->set_text(stTemp);

	if (m_dragInProgress)
	{
		//mouse is being dragged
		m_vecDragStop = mouseWorldPos;

		if (CL_Keyboard::get_keycode(CL_KEY_SPACE))
		{
			CL_Vector2 oldWorldPos = GetWorldCache->ScreenToWorld(CL_Vector2(m_vecLastMousePos.x,m_vecLastMousePos.y));
			//let's move our start by the amount they dragged with space bar down, old photshop trick
			m_vecDragStart -= (oldWorldPos-m_vecDragStop);
		}
	}

	m_vecLastMousePos = key.mouse_pos;
}

void EntEditMode::OnSelectSimilar()
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("To select similar, you must first select one or more tiles to work from.");
		return;
	}
	
	if (m_selectedTileList.m_selectedTileList.size() > 25)
	{
		if (!ConfirmMessage("Warning: Slow!","Are you sure you meant to find similar?  With this many source tiles it might be very slow." ))
		{
			return;
		}
	}
	
	TileEditOperation tempList = m_selectedTileList;
	
	//go through an select similar of each of these

	selectedTile_list::iterator itor = tempList.m_selectedTileList.begin();

	CL_Rectf r = GetCamera->GetViewRectWorld();
	
	while (itor != tempList.m_selectedTileList.end())
	{
		int operation = TileEditOperation::C_OPERATION_ADD;
		m_selectedTileList.AddTilesByWorldRectIfSimilar(CL_Vector2(r.left,r.top), CL_Vector2(r.right,r.bottom), operation, GetWorld->GetLayerManager().GetEditActiveList(), (*itor)->m_pTile);

			itor++;
	}
	
}

void EntEditMode::OnDeleteBadTiles()
{
	tile_list tlist;

	BlitMessage("Checking for tiles with resource errors...");
	
	CL_Rectf r = GetCamera->GetViewRectWorld();
	GetWorldCache->AddTilesByRect(CL_Rect(r.left, r.top, r.right, r.bottom), &tlist, GetWorld->GetLayerManager().GetAllList());

	LogMsg("Found %d tiles to look through.", tlist.size());

	//delete any that have missing art

	TilePic *pTilePic;

	int tilesRemoved = 0;
	tile_list::iterator itor;
	for (itor = tlist.begin(); itor != tlist.end(); itor++)
	{
		
		if ((*itor)->GetType() == C_TILE_TYPE_PIC)
		{
			//here is a tile pic.  but does it have a problem?

			pTilePic = (TilePic*) (*itor);

			if (!GetHashedResourceManager->GetResourceByHashedID(pTilePic->m_resourceID))
			{
				//error here.  Remove this tile
				pTilePic->GetParentScreen()->RemoveTileByPointer(pTilePic);
				tilesRemoved++;
			}

		}
	}

	CL_MessageBox::info("Bad tile search results", "Removed " + CL_String::from_int(tilesRemoved) + " tiles.", GetApp()->GetGUI());
}

void EntEditMode::OnReplace()
{
	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("Can't replace: Nothing selected, so nothing to replace.", GetApp()->GetGUI());
		return;
	}

	if (g_EntEditModeCopyBuffer.IsEmpty())
	{
		CL_MessageBox::info("Can't replace:  Nothing is in the copy buffer. Select the new tile, Ctrl-C, then select what you want to replace.", GetApp()->GetGUI());
		return;
	}

	//make a copy of what is selected
	TileEditOperation tempList = m_selectedTileList;

	//delete selection
	Tile blankTile; 
	AddToUndo(&m_selectedTileList);
	m_selectedTileList.FillSelection(&blankTile);

	//now create a new tile operation that will do a massive paste

	m_selectedTileList.ClearSelection();
	//look at each tile that we delete individually

	selectedTile_list::iterator itor;
	
	MovingEntity *pEnt = NULL;
	Tile *pTile = NULL;
	CL_Vector2 destPos;

	
	//check out the first tile available for pasting to get some info about it that may help us line it up better later
	Tile *pPastTile = (*g_EntEditModeCopyBuffer.m_selectedTileList.begin())->m_pTile;
	
	CL_Vector2 vPasteOffset = CL_Vector2::ZERO;

	MovingEntity *pPasteEnt = NULL;

	if (pPastTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		//entities are always centered, not upper left like tilepics.
		pPasteEnt = ((TileEntity*)pPastTile)->GetEntity();
		vPasteOffset = CL_Vector2(pPasteEnt->GetWorldRect().left, pPasteEnt->GetWorldRect().top) - pPasteEnt->GetPos();
	}
	for (itor = tempList.m_selectedTileList.begin(); itor != tempList.m_selectedTileList.end(); itor++)
	{
		g_EntEditModeCopyBuffer.SetIgnoreParallaxOnNextPaste();
		pTile = (*itor)->m_pTile;

		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			pEnt = ((TileEntity*)pTile)->GetEntity();
		} else
		{
			pEnt = NULL;
		}
		

		destPos = pTile->GetPos();


		OnPaste(g_EntEditModeCopyBuffer, destPos+vPasteOffset);
	}

}

void EntEditMode::ScaleSelection(CL_Vector2 vMod, bool bBigMovement)
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("Can't scale, no selection.");
		return;
	}

	if (bBigMovement)
	{
		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect

		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect

		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect
	}
	
	TileEditOperation temp = m_selectedTileList; //copy whatever is highlighted
	OnDelete();
		temp.ApplyScaleMod(vMod);
		temp.SetIgnoreParallaxOnNextPaste();
	OnPaste(temp, temp.GetUpperLeftPos());
	m_selectedTileList = temp;
}

void EntEditMode::ScaleUpSelected()
{
	ScaleSelection(CL_Vector2(1 + C_SCALE_MOD_AMOUNT, 1 + C_SCALE_MOD_AMOUNT), CL_Keyboard::get_keycode(CL_KEY_SHIFT));
}

void EntEditMode::ScaleDownSelected()
{
	ScaleSelection(CL_Vector2(1 - C_SCALE_MOD_AMOUNT, 1 -C_SCALE_MOD_AMOUNT), CL_Keyboard::get_keycode(CL_KEY_SHIFT));
}

void EntEditMode::Init()
{
	Kill();

	//setup GUI
	m_pWindow = new CL_Window(CL_Rect(0, 0, 200, 150), "Tile Edit Floating Palette", CL_Window::close_button, GetApp()->GetGUI());
	
	m_pWindow->set_position(0, C_EDITOR_MAIN_MENU_BAR_HEIGHT);
	m_slotClose = m_pWindow->sig_close_button_clicked().connect_virtual(this, &EntEditMode::OnClose);

	//setup our own tip display
	m_pLabelMain = new CL_Label(CL_Point(2,52),"", m_pWindow->get_client_area());
	
	m_pLabelSelection = new CL_Label(CL_Point(2,66),"", m_pWindow->get_client_area());

	//set main dialog tips
	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (!pEditor) throw CL_Error("Unable to locate editor entity");
	pEditor->SetTipLabel(
		"Tile Edit Mode - Use the mouse wheel or +/- to zoom in/out.  Hold the middle mouse button and drag to move camera.  Hold and drag or click with the left mouse button to make a selection.\n" \
		"\n"\
		"Hold Shift,Alt, or Ctrl while clicking to add/remove/toggle from selection.  Use arrow keys to nudge selection one pixel.  (think photoshop controls)\n"\
		"");

	m_pWindow->set_event_passing(false);

	m_pMenu = new CL_Menu(m_pWindow->get_client_area());
	CL_MenuNode *pItem;
	m_pMenu->set_event_passing(false);

	pItem = m_pMenu->create_item("Edit/Undo (Ctrl+Z)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnUndo);
	m_pMenuUndo = pItem; //remember this for later

	pItem = m_pMenu->create_item("Edit/Cut (Ctrl+X)");
	m_slots.connect(pItem->sig_clicked(),this, &EntEditMode::OnCut);

	pItem = m_pMenu->create_item("Edit/Copy (Ctrl+C)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnCopy);

	pItem = m_pMenu->create_item("Edit/Delete (DELETE)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDelete);


	pItem = m_pMenu->create_item("Edit/ ");
	pItem = m_pMenu->create_item("Edit/(Ctrl-V or right mouse button to paste selection)");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Get image (puts in copy buffer)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::BuildBaseTilePageBox);

	pItem = m_pMenu->create_item("Utilities/Get default Entity (puts in copy buffer)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::BuildDefaultEntity);

	
	pItem = m_pMenu->create_item("Utilities/Select similar to current selection (limited by view and active edit layers) (Ctrl-R)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnSelectSimilar);

	pItem = m_pMenu->create_item("Utilities/Replace each selected with what is in the copy buffer");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnReplace);

	pItem = m_pMenu->create_item("Utilities/Remove tiles with missing graphics (limited by view)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDeleteBadTiles);

	pItem = m_pMenu->create_item("Utilities/Create tile from tile (Ctrl-C while dragging a selection rectangle (Hold space to adjust drag position)");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Tip: You can create a tile from the active selection size (without dragging) with Shift-Ctrl-C");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Note: Tile from tile only works when everything is on the same bitmap");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Modify Selected/Edit properties (Ctrl-E or Ctrl+Right Mouse Button)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnProperties);

	pItem = m_pMenu->create_item("Modify Selected/Edit collision data (Toggle) (Ctrl-D)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDefaultTileHardness);

	pItem = m_pMenu->create_item("Modify Selected/Edit visual profile  (Ctrl-Shift-E) (if applicable)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnEditVisualProfile);

	pItem = m_pMenu->create_item("Modify Selected/Scale Down ([)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::ScaleDownSelected);

	pItem = m_pMenu->create_item("Modify Selected/Scale Up (])");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::ScaleUpSelected);

	pItem = m_pMenu->create_item("Modify Selected/ ");
	pItem = m_pMenu->create_item("Modify Selected/(Tip: Hold shift to scale by a larger amount)");
	pItem->enable(false);

	CL_Point offset = CL_Point(2,30);
	m_pCheckBoxSnap = new CL_CheckBox(offset, "Grid Snap - Size", m_pWindow->get_client_area());
	m_slots.connect(m_pCheckBoxSnap->sig_clicked(), this, &EntEditMode::SnapCheckBoxedClicked);

	m_pCheckBoxSnap->set_focusable(false);
	//make the coordinate input box
	
	CL_Point tmp = offset+CL_Point(m_pCheckBoxSnap->get_width(), -2);
	CL_Rect recSize(tmp, CL_Size(50,16));
	
	m_pInputBoxSnapSize = new CL_InputBox(CL_String::from_int(m_snapSize), m_pWindow->get_client_area());
	
    m_pInputBoxSnapSize->set_position(recSize);
	
	//m_slots.connect(m_pInputBoxSnapSize->sig_lost_focus(), this, &EntEditMode::SnapSizeChanged);
	m_slots.connect(m_pInputBoxSnapSize->sig_return_pressed(), this, &EntEditMode::SnapSizeChanged);
	m_slots.connect(m_pInputBoxSnapSize->sig_changed(), this, &EntEditMode::OnSnapSizeChanged);
	
	//m_pInputBoxSnapSize->set_position()
	offset.y += m_pCheckBoxSnap->get_height();
	GetSettingsFromWorld();
	UpdateMenuStatus();
}

void EntEditMode::OnSnapSizeChanged(const string &st)
{
	SnapSizeChanged();
}

void EntEditMode::BuildDefaultEntity()
{
	MovingEntity *pEnt = new MovingEntity;
	TileEntity *pTile = new TileEntity;
	pTile->SetEntity(pEnt);
	pEnt->Init();

	g_EntEditModeCopyBuffer.ClearSelection();
	g_EntEditModeCopyBuffer.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
		false, pTile);
}

void EntEditMode::OnExternalDialogClose(int entID)
{
	m_bDialogIsOpen = false;
	LogMsg("Closing external dialog");
}
void EntEditMode::OnEditVisualProfile()
{
	
	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("Select an entity first.", GetApp()->GetGUI());
		return;
	}

	if (m_selectedTileList.m_selectedTileList.size() > 1)
	{
		CL_MessageBox::info("Choose only ONE entity to edit its visual profile.", GetApp()->GetGUI());
		return;
	}


	Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;

	//sure, we have the copy in the selection buffer, but this is only a copy.  Let's go grab a pointer
	//to the real instance to use directly.
	
	pTile = GetWorld->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());

	if (!pTile)
	{
		CL_MessageBox::info("Error finding entity.", GetApp()->GetGUI());
		return;
	}
	
	if (pTile->GetType() != C_TILE_TYPE_ENTITY)
	{
		CL_MessageBox::info("This option can only be used on entities that have a visual profile attached by script.", GetApp()->GetGUI());
		return;
	}

	MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();

	if (!pEnt->GetVisualProfile())
	{
		CL_MessageBox::info("No visual profile has been assigned to this entity yet.  This must be done in its script.", GetApp()->GetGUI());
		return;
	}


	LogMsg("Editing visual profile %s", pEnt->GetVisualProfile()->GetName().c_str());
	EntVisualProfileEditor *pEditor = (EntVisualProfileEditor*) GetMyEntityMgr->Add(new EntVisualProfileEditor);

	if (!pEditor || !pEditor->Init(pEnt))
	{
		LogError("Error initializing visual profile editor");
		return;
	}

	m_bDialogIsOpen = true; //this cripples the EntEditMode functionality so we don't
	//interfere with the new dialog we just opened

	//However, we do want to know when it closes so we can change it back

	m_slots.connect(pEditor->sig_delete, this, &EntEditMode::OnExternalDialogClose);
}

void EntEditMode::SnapSizeChanged()
{
	if (m_pCheckBoxSnap->is_checked())
	{
		m_pInputBoxSnapSize->enable(true);
		int snap = CL_String::to_int(m_pInputBoxSnapSize->get_text());

		if (snap < 1) return;

		snap = max(1, snap);
		snap = min(snap, 4048);
		GetWorld->SetDefaultTileSize(snap);
		GetWorld->SetSnapEnabled(true);
		m_pInputBoxSnapSize->set_text(CL_String::from_int(snap));
	} else
	{
		m_pInputBoxSnapSize->enable(false);
		GetWorld->SetSnapEnabled(false);
	}

	m_snapSize = GetWorld->GetDefaultTileSize();
}

void EntEditMode::SnapCheckBoxedClicked()
{
	SnapSizeChanged();
}

void EntEditMode::Kill()
{
	if (m_pEntCollisionEditor)
	{
		//close this thing too
		m_pEntCollisionEditor->SetDeleteFlag(true);
		m_pEntCollisionEditor = NULL;
	}
	
	SAFE_DELETE(m_pCheckBoxSnap);
	SAFE_DELETE(m_pInputBoxSnapSize);
	SAFE_DELETE(m_pListBaseTile);
	SAFE_DELETE(m_pWindowBaseTile);
	SAFE_DELETE(m_pMenu);
	SAFE_DELETE(m_pWindow);
}

void EntEditMode::OnSelectBaseTilePage()
{
	int resourceID = FileNameToID(m_pListBaseTile->get_current_text().c_str());

	//break apart this pic into chunks
	int snap = 0;
	if (m_pCheckBoxSnap->is_checked()) snap = m_snapSize;
	GetHashedResourceManager->PutGraphicIntoTileBuffer(resourceID, g_EntEditModeCopyBuffer, snap);
}

void EntEditMode::BuildBaseTilePageBox()
{
	SAFE_DELETE(m_pListBaseTile);
	SAFE_DELETE(m_pWindowBaseTile);

	int width = 230;
	int height =  (GetHashedResourceManager->GetHashedResourceMap()->size()*12) +40;
	if (height > 500) height = 500;

	CL_Rect rectSize = CL_Rect(GetScreenX-width, C_EDITOR_MAIN_MENU_BAR_HEIGHT, 0, 0);
	rectSize.set_size(CL_Size(width,height));

	m_pWindowBaseTile = new CL_Window(rectSize, "Will use snap settings to split into buff" , CL_Window::close_button, GetApp()->GetGUI()->get_client_area());
	m_pWindowBaseTile->set_event_passing(false);

	CL_Rect r = m_pWindowBaseTile->get_children_rect();
	r.set_size(r.get_size() - CL_Size(5,28));
	m_pListBaseTile = new CL_ListBox(r, m_pWindowBaseTile->get_client_area());

	//add all maps

	HashedResourceMap * pMap = GetHashedResourceManager->GetHashedResourceMap();
	HashedResourceMap::iterator itor = pMap->begin();

	while(itor != pMap->end())
	{
		m_pListBaseTile->insert_item(GetStrippedFilename((*itor).second->m_strFilename));
		itor++;
	}

	m_slotSelectedBaseTilePage = m_pListBaseTile->sig_selection_changed().connect(this, &EntEditMode::OnSelectBaseTilePage);
}

//helps us know when it's a bad time to process hotkeys
bool EntEditMode::IsDialogOpen()
{
	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (pEditor)
	{
		if (pEditor->IsDialogOpen()) return true;
	}
	if (m_bDialogIsOpen) return true;

	return false;
}

void EntEditMode::ClearSelection()
{
	m_selectedTileList.ClearSelection();
}

void EntEditMode::OnClose(CL_SlotParent_v0 &parent_handler)
{
	SetDeleteFlag(true);
}

void EntEditMode::onButtonUp(const CL_InputEvent &key)
{
	if (m_pEntCollisionEditor) 
	{
	  switch (key.id)
		{
		case CL_KEY_D:
			m_pEntCollisionEditor->OnOk();
			break;
		}

		return; //now is a bad time for most stuff
	}

	if (IsDialogOpen()) 
	{
		//we must be doing something in a dialog, don't want to delete entities accidently	
		return;
	}

	switch(key.id)
	{
	
	case CL_KEY_D:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnDefaultTileHardness();
		}
		break;
	
	case CL_MOUSE_LEFT:
		{
			//highlight selected tile(s)
			
			if (!m_dragInProgress) break; //not a real drag was happening

			//scan every tile within this range
			m_vecDragStop =  GetWorldCache->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL) &&
				!CL_Keyboard::get_keycode(CL_KEY_SHIFT) &&
#ifdef __APPLE__
				!CL_Keyboard::get_keycode(CL_KEY_COMMAND) &&
#endif

				!CL_Keyboard::get_keycode(CL_KEY_MENU)
				)
			{
				ClearSelection();
			}
			m_dragInProgress = false;

			int operation = TileEditOperation::C_OPERATION_TOGGLE;
			if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
			{
				operation = TileEditOperation::C_OPERATION_ADD;
			}
			if (CL_Keyboard::get_keycode(CL_KEY_MENU))
			{
				operation = TileEditOperation::C_OPERATION_SUBTRACT;
			}

			if ( (m_vecDragStop- m_vecDragStart).length() < 1)
			{
				//they clicked one thing, without really dragging.  Treat it special
				m_selectedTileList.AddTileByPoint(m_vecDragStart, operation, GetWorld->GetLayerManager().GetEditActiveList());
			} else
			{
			
				m_selectedTileList.AddTilesByWorldRect(m_vecDragStart, m_vecDragStop, operation, GetWorld->GetLayerManager().GetEditActiveList());
			}
		}

		break;
	}
}

void EntEditMode::SelectByLayer(const vector<unsigned int> &layerVec)
{
	m_selectedTileList.ClearSelection();
	int operation = TileEditOperation::C_OPERATION_ADD;

	CL_Rectf r = GetCamera->GetViewRectWorld();
	m_selectedTileList.AddTilesByWorldRect(CL_Vector2(r.left,r.top), CL_Vector2(r.right,r.bottom), operation, layerVec);

}
void EntEditMode::OnCopy()
{
	//copy whatever is highlighted
	g_EntEditModeCopyBuffer = m_selectedTileList;
}

void EntEditMode::OnPaste(TileEditOperation &editOperation, CL_Vector2 vecWorld)
{
   //OPTIMIZE can avoid a copy operation of the undo buffer

	if (editOperation.IsEmpty()) return;
	TileEditOperation undo;
	editOperation.PasteToWorld(vecWorld, TileEditOperation::PASTE_UPPER_LEFT, &undo);
	AddToUndo(&undo);
}

void EntEditMode::UpdateMenuStatus()
{
	//grey out undo button if it has no data
	m_pMenuUndo->enable(m_undo.size() != 0);
}
void EntEditMode::AddToUndo( TileEditOperation *pTileOperation)
{
	if (pTileOperation->IsEmpty()) return;

	if (m_undo.size() >= C_TILE_EDIT_UNDO_LEVEL)
	{
		m_undo.pop_back();
	}

	TileEditOperation to;
	m_undo.push_front(to);
	m_undo.front() = *pTileOperation;
	UpdateMenuStatus();
}

void EntEditMode::OnUndo()
{
	//LogMsg("Undo size is %u", m_undo.size());

	if (m_undo.size() == 0) return;
    m_undo.front().SetIgnoreParallaxOnNextPaste();
	m_undo.front().PasteToWorld(m_undo.front().GetUpperLeftPos(), TileEditOperation::PASTE_UPPER_LEFT, NULL);
	m_undo.pop_front();
	UpdateMenuStatus();
}

void EntEditMode::OnDelete()
{
	if (m_selectedTileList.IsEmpty()) return;
	//delete selection
	Tile blankTile; 
	AddToUndo(&m_selectedTileList);
	m_selectedTileList.FillSelection(&blankTile);
	ClearSelection();
}

void EntEditMode::OnCut()
{
	OnCopy();
	OnDelete();
}

void EntEditMode::CutSubTile(CL_Rect recCut)
{
	LogMsg("Cutting %d, %d, %d, %d (%d, %d)", recCut.left, recCut.top, recCut.right, recCut.bottom,
		recCut.get_width(), recCut.get_height());

	//which bitmap are we trying to cut from?
	tile_list tList;
	GetWorldCache->AddTilesByRect(recCut, &tList, GetWorld->GetLayerManager().GetEditActiveList());
	if (tList.size() == 0)
	{
		CL_MessageBox::info("SubTile Create failed, you need to do it over an existing tile on this layer.", GetApp()->GetGUI());
		return;
	}

	//guess at the most suitable one
	tile_list::iterator itor = tList.begin();
	Tile *pTile = *tList.begin(); //default
	
	while (itor != tList.end())
	{
		
		if (pTile->GetType() != C_TILE_TYPE_PIC)
		{
			//well, this can't be right
			pTile = (*itor);
		}

		if ( (*itor)->GetBoundsSize().length() > pTile->GetBoundsSize().length())
		{
			//this is bigger, it's probably what they wanted
			pTile = (*itor);
		}
		itor++;
	}

	if (pTile->GetType() != C_TILE_TYPE_PIC)
	{
		CL_MessageBox::info("Don't know how to make a subtile out of this tile type.  Try a normal pic.", GetApp()->GetGUI());
		return;
	}
	//now that we have it, we need to create a new subtile from it, if possible
	if (
		(pTile->GetWorldRect().calc_union(recCut).get_width() != recCut.get_width())
		|| (pTile->GetWorldRect().calc_union(recCut).get_height() != recCut.get_height())

		)
	{
		LogMsg("Warning: Subtile selection too big for tile it looks like...");
	}

	CL_Rect localRect = recCut;
	localRect.apply_alignment(origin_top_left, pTile->GetPos().x, pTile->GetPos().y);

	LogMsg("We're going to cut the new tile at %d,%d,%d,%d", localRect.left, localRect.top, localRect.right, localRect.bottom);

	GetHashedResourceManager->PutSubGraphicIntoTileBuffer((TilePic*)pTile, g_EntEditModeCopyBuffer, localRect);
	m_dragInProgress = false;
}

void EntEditMode::OffsetSelectedItems(CL_Vector2 vOffset, bool bBigMovement)
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("Can't nudge, no selection.");
		return;
	}

	if (bBigMovement) vOffset *= 10;
	TileEditOperation temp = m_selectedTileList; //copy whatever is highlighted
	OnDelete();
	temp.ApplyOffset(vOffset);
	temp.SetIgnoreParallaxOnNextPaste();
	OnPaste(temp, temp.GetUpperLeftPos());
	m_selectedTileList = temp;
}

void EntEditMode::HandleMessageString(const string &msg)
{
	vector<string> words = CL_String::tokenize(msg, "|");
	
	if (words[0] == "offset_selected")
	{
		assert(words.size() == 4);
		OffsetSelectedItems(CL_Vector2(CL_String::to_float(words[1]),CL_String::to_float(words[2])), 
			CL_String::to_bool(words[3]) );

	} else
	{
		LogMsg("Don't know how to handle message %s", msg.c_str());
	}

}

void EntEditMode::onButtonDown(const CL_InputEvent &key)
{
	if (m_pEntCollisionEditor) return; //now is a bad time

	if (IsDialogOpen()) 
	{
		//we must be doing something in a dialog, don't want to delete entities accidently	
		return;
	}
	
	#define VK_CLOSE_BRACKET 0xDD
	#define VK_OPEN_BRACKET 0xDB

	switch(key.id)
	{
	case VK_OPEN_BRACKET:
		ScaleDownSelected();
		break;
	
	case VK_CLOSE_BRACKET:
		ScaleUpSelected();
		break;

	//nudging
	case CL_KEY_RIGHT:
		ScheduleSystem(1, ID(), ("offset_selected|1|0|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_LEFT:
		ScheduleSystem(1, ID(), ("offset_selected|-1|0|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_UP:
		ScheduleSystem(1, ID(), ("offset_selected|0|-1|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_DOWN:
		ScheduleSystem(1, ID(), ("offset_selected|0|1|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_MOUSE_LEFT:

		if (!m_dragInProgress)
		{
			m_vecDragStart = GetWorldCache->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			m_vecDragStop = m_vecDragStart;
			m_dragInProgress = true;
		}
		break;
	case CL_KEY_R:

		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnSelectSimilar();
		}
		break;

	case CL_KEY_C:

		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			//two modes of copy, one to cut out subtiles..
			if (m_dragInProgress)
			{
				m_vecDragStart = GetWorld->SnapWorldCoords(m_vecDragStart, m_dragSnap);
				m_vecDragStop = GetWorld->SnapWorldCoords(m_vecDragStop, m_dragSnap);
				CL_Rect rec( int(m_vecDragStart.x), int(m_vecDragStart.y), int(m_vecDragStop.x), int(m_vecDragStop.y));
				rec.normalize();
				CutSubTile(rec);
			} else if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
			{
				
				if (m_selectedTileList.IsEmpty())
				{
					CL_MessageBox::info("Shift-Ctrl-C makes a subtile from the current selection size.  But you don't have a selection!", GetApp()->GetGUI());

					return;
				}
				//cut subtile from current selection, if possible..
				 CL_Rect rec(int(m_selectedTileList.GetUpperLeftPos().x), int(m_selectedTileList.GetUpperLeftPos().y),
					 int(m_selectedTileList.GetLowerRightPos().x), int(m_selectedTileList.GetLowerRightPos().y));
				rec.normalize();
				CutSubTile(rec);
			} else
			{
				OnCopy();
			}
		}

		break;

	case CL_KEY_X:
		if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL)) return;
		OnCut();
		break;

    case CL_KEY_DELETE:
			OnDelete();
		break;

	case CL_KEY_E:
		if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL)) return;
		
		if (!CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			OnProperties();
		} else
		{
			OnEditVisualProfile();
		}

		break;

	case CL_KEY_Z:
		if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL)) return;
		OnUndo();
		break;

	case CL_KEY_V:
			
	if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
	{
		CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());
		CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(vecMouseClickPos);
		OnPaste(g_EntEditModeCopyBuffer, vecWorld);
	}

	break;
	
	case CL_MOUSE_RIGHT:
		{
			CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());
			CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(vecMouseClickPos);

			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				//control right click opens a special menu instead of pasting
			    BuildTilePropertiesMenu(&vecMouseClickPos, &vecWorld, &m_selectedTileList);
				return;
			}

			OnPaste(g_EntEditModeCopyBuffer, vecWorld);
		}
	
		break;

	default: ;
	}

}

void EntEditMode::Update(float step)
{
}

CL_Vector2 EntEditMode::ConvertMouseToCenteredSelectionUpLeft(CL_Vector2 vecMouse)
{
	vecMouse = GetWorldCache->ScreenToWorld(vecMouse);
	
	if (g_EntEditModeCopyBuffer.IsEmpty())
	{
		//this is no selection, so let's just convert to what they are highlighting
		return GetWorld->SnapWorldCoords(vecMouse, 1);
		return vecMouse;
	}

	vecMouse -= g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits()/2;
	if (m_pCheckBoxSnap->is_checked())
	{
		vecMouse = GetWorld->SnapWorldCoords(vecMouse, m_snapSize);
	} else
	{
		vecMouse = GetWorld->SnapWorldCoords(vecMouse, 1);

	}
	//vecMouse += CL_Vector2(GetWorld->GetDefaultTileSize()/2, GetWorld->GetDefaultTileSize()/2);
	return vecMouse;
}

void EntEditMode::DrawActiveBrush(CL_GraphicContext *pGC)
{
  if (g_EntEditModeCopyBuffer.IsEmpty()) return;
  CL_Vector2 vecMouse = ConvertMouseToCenteredSelectionUpLeft(CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y()));
  CL_Vector2 vecBottomRight = (vecMouse + (g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits()));

  SelectedTile *pSelTile = (*g_EntEditModeCopyBuffer.m_selectedTileList.begin());
  DrawRectFromWorldCoordinates(vecMouse, vecBottomRight, CL_Color(255,0,0,150), pGC);
}


void EntEditMode::OnProperties()
{
	BuildTilePropertiesMenu(NULL, NULL, &m_selectedTileList);
}

void EntEditMode::OnCollisionDataEditEnd(int id)
{
	
	if (!m_pEntCollisionEditor) return;
	bool bDataChanged = m_pEntCollisionEditor->GetDataChanged();
	
	//last minute thing to update the tiles that might be modified
	m_pEntCollisionEditor = NULL; //back to normal mode

	//need to reinit all script based items possibly
	if (m_pOriginalEditEnt)
	{
		m_pOriginalEditEnt->SetPos(m_collisionEditOldTilePos - m_pOriginalEditEnt->GetVisualOffset());
		m_pTileWeAreEdittingCollisionOn = m_pOriginalEditEnt->GetTile(); //fix a probably bad pointer, from moving it
		
		//also fix our selection box

		m_selectedTileList.ClearSelection();
		m_selectedTileList.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
			false, m_pTileWeAreEdittingCollisionOn);
		//before
	}

	if (bDataChanged)
	{

		if (m_pTileWeAreEdittingCollisionOn->GetType() == C_TILE_TYPE_ENTITY)
		{
			TileEntity *pEnt = (TileEntity*)m_pTileWeAreEdittingCollisionOn;
			if (pEnt->GetEntity())
			{

				pEnt->GetEntity()->Kill(); //force it to save out its collision info to disk, so when
				//we reinit all similar tiles, they will use the changed version
				GetWorld->ReInitEntities();
			} else
			{
				assert(!"Huh?");
			}
		}

		if (m_pTileWeAreEdittingCollisionOn->GetType() == C_TILE_TYPE_PIC)
		{
			TilePic *pTilePic = (TilePic*)m_pTileWeAreEdittingCollisionOn;

			pTilePic->SaveToMasterCollision();
			GetWorld->ReInitCollisionOnTilePics();


		}
	}
}

void EntEditMode::OnDefaultTileHardness()
{
	
	if (m_pEntCollisionEditor)
	{
		m_pEntCollisionEditor->OnOk();
		return;
	}

	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("No tiles selected.  Left click on a tile first.", GetApp()->GetGUI());
		return;
	}

	if (m_selectedTileList.m_selectedTileList.size() > 1)
	{
		CL_MessageBox::info("Collision data editing can only be done on single tiles, just select one.", GetApp()->GetGUI());
		return;
	}

	Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;
	pTile  =  GetWorld->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());
	
	if (!pTile)
	{
		CL_MessageBox::info("Highlighted entity not found, don't try this on something that is moving.", GetApp()->GetGUI());
		return;
	}
 
	m_pOriginalEditEnt = 	NULL;

	bool bIsEnt = pTile->GetType()==C_TILE_TYPE_ENTITY;
	if (bIsEnt)
	{
		m_pOriginalEditEnt = ((TileEntity*)pTile)->GetEntity();
	}
	
	bool bUsesCustomCollisionData = bIsEnt &&  ((TileEntity*)pTile)->GetEntity()->UsingCustomCollisionData();

	//well, now we have the tile that was in our selection buffer.  But that isn't really good enough
	
	if (!pTile->GetCollisionData())
	{
		//it's an entity but no collision data has been assigned
		CL_MessageBox::info("This tile/entity doesn't have collision data enabled in it's script.", GetApp()->GetGUI());
		return;
	}

	m_pEntCollisionEditor = (EntCollisionEditor*) GetMyEntityMgr->Add(new EntCollisionEditor);
	m_slots.connect(m_pEntCollisionEditor->sig_delete, this, &EntEditMode::OnCollisionDataEditEnd);

	CL_Rect rec(pTile->GetWorldRect());
	
	CL_Vector2 vEditPos = pTile->GetPos();

	if (bIsEnt)
	{
		rec.apply_alignment(origin_top_left,m_pOriginalEditEnt->GetVisualOffset().x, m_pOriginalEditEnt->GetVisualOffset().y);
		vEditPos += m_pOriginalEditEnt->GetVisualOffset();
		//total hack to fake the visual offset while we're drawing it
		
		m_pOriginalEditEnt->SetPos(m_pOriginalEditEnt->GetPos() + m_pOriginalEditEnt->GetVisualOffset());
		m_collisionEditOldTilePos = m_pOriginalEditEnt->GetPos();
	
	} else
	{

	}

	rec.apply_alignment(origin_top_left, vEditPos.x, vEditPos.y);

	//the collision shape MUST be have point 0,0 inside of it, so if we want a shape to start at offset
	//50,50 in an image, it breaks.  To fix, we automatically handle creating an offset from the real 
	//position.  But for entities, this isn't needed as the shape is always going to be centered around
	//0,0

//	bool useCollisionOffsets = !bIsEnt;
	bool useCollisionOffsets = true;

	pTile->GetCollisionData()->RemoveOffsets();

	m_pTileWeAreEdittingCollisionOn = pTile; //messy way of remembering this in the
	//callback that is hit when editing is done

	m_pEntCollisionEditor->Init(vEditPos, rec, pTile->GetCollisionData(), useCollisionOffsets);
}

void EntEditMode::Render(void *pTarget)
{
	//uh... there's gotta be a better way, but I don't want to pollute the EntBase class
	//with Clanlib specific things
	CL_GraphicContext *pGC = (CL_GraphicContext*)pTarget;

	selectedTile_list::iterator itor = m_selectedTileList.m_selectedTileList.begin();

	//draw the drag rect
	m_pLabelSelection->set_text("");

	if (!m_selectedTileList.IsEmpty())
	{
		string s;
		char stTemp[256];
		sprintf(stTemp, "Sel Pos: X: %.1f Y: %.1f\n(%.1f, %.1f)", m_selectedTileList.GetUpperLeftPos().x, m_selectedTileList.GetUpperLeftPos().y,
			m_selectedTileList.GetSelectionSizeInWorldUnits().x, m_selectedTileList.GetSelectionSizeInWorldUnits().y);
	
		s = stTemp;

		if (m_selectedTileList.m_selectedTileList.size() == 1)
		{
			unsigned int tileLayer = m_selectedTileList.m_selectedTileList.back()->m_pTile->GetLayer();
			LayerManager &layerMan = GetWorld->GetLayerManager();

			Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;
			
			if (tileLayer >= layerMan.GetLayerCount())
			{
				s += ( "\nLayer: Invalid (ID "+CL_String::from_int(tileLayer)+")");
			} else
			{
				s += ( "\nLayer: "+ layerMan.GetLayerInfo(tileLayer).GetName());
			}

			if (pTile->GetType() == C_TILE_TYPE_ENTITY)
			{
				//it's pointless to show the entity ID unless we are working from the real source tile instead of the copy
				Tile *pSrcTile  =  GetWorld->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());

				if (pSrcTile)
				{
					MovingEntity *pEnt = ((TileEntity*)pSrcTile)->GetEntity();
					s += "\nID: " + CL_String::from_int(pEnt->ID());
					if (!pEnt->GetName().empty())
					{
						s += " Tag: "+pEnt->GetName();
					}
				}
			}
			
		}
		m_pLabelSelection->set_text(s);

	}
	
	while (itor != m_selectedTileList.m_selectedTileList.end())
	{
		DrawSelectedTileBorder((*itor)->m_pTile, pGC);
		itor++;
	}

	if (m_pEntCollisionEditor) return; //don't draw this extra GUI stuff right now
	if (m_dragInProgress)
	{
		m_pLabelSelection->set_text(CL_String::format("Size: %1, %2", int(GetWorld->SnapWorldCoords(m_vecDragStop, m_dragSnap).x - GetWorld->SnapWorldCoords(m_vecDragStart, m_dragSnap).x),
			int(GetWorld->SnapWorldCoords(m_vecDragStop, m_dragSnap).y - GetWorld->SnapWorldCoords(m_vecDragStart, m_dragSnap).y)));
		//draw a rect on the screen
		DrawRectFromWorldCoordinates(m_vecDragStart, m_vecDragStop, CL_Color(255,255,0,200), pGC);
	} else
	{
		//draw the brush, if available
		DrawActiveBrush(pGC);
	}

	//for testing
}

void DrawSelectedTileBorder(Tile *pTile, CL_GraphicContext *pGC)
{
	//let's draw a border around this tile
	CL_Rectf worldRect(pTile->GetWorldRect());
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);
	
	DrawRectFromWorldCoordinates(vecStart, vecStop, CL_Color(255,255,255,180), pGC);
}

void EntEditMode::OnPropertiesOK()
{
	m_guiResponse = C_GUI_OK;
}

void EntEditMode::OnPropertiesEditScript()
{
	string file = GetGameLogic->GetScriptRootDir()+"/"+m_pPropertiesInputScript->get_text();
	g_VFManager.LocateFile(file);
	OpenScriptForEditing(CL_Directory::get_current() + "/" +file);
}

void EntEditMode::OnPropertiesOpenScript()
{
	//this is awfully messy, because I'm calculating paths in a strange way to get them based off of the media/scripts
	//dir.  

	assert(m_pPropertiesInputScript);

	string fname = m_pPropertiesInputScript->get_text();
	string original_dir = CL_Directory::get_current();
	string dir = original_dir + "/" + GetGameLogic->GetScriptRootDir();
	string path = dir;
	
	string fileNameWithoutPath = fname;
	int offset;

	if ( (offset=fname.find_last_of("/\\")) != -1)
	{
		fileNameWithoutPath = fname.substr(offset+1, (fname.size()-offset)-1);
	} else
	{
		fileNameWithoutPath = fname;
	}

	if ( (offset=fname.find_last_of("/\\")) != -1)
	{
		path += ("/"+fname.substr(0, offset));
	}

	CL_Directory::change_to(path);
	CL_FileDialog dlg("Open LUA Script", fileNameWithoutPath, "*.lua", GetApp()->GetGUI());
	
	//dlg.set_behavior(CL_FileDialog::quit_file_selected);
	dlg.run();

	CL_Directory::change_to(original_dir);
	
	if (dlg.get_file().empty())
	{
		//guess thet didn't choose anything
		return;
	}

	//extract what they chose with the relevent path
	
	fname = dlg.get_path() +"/" + dlg.get_file();

	if (fname.size() < dir.size())
	{
		//imprecise way to figure out if they went to a wrong directory and avoids an STL crash.  really, I
		//need to convert the slashes to forward slashes in both strings and check if dir is in fname.
		
		LogMsg("Can't open this file, outside of our media dir.");
		return;
	}

	//remove the part we don't want
	fname = fname.substr(dir.size(), (fname.size()-dir.size()));
	

	//let's just convert all the slashes
	StringReplace("\\", "/", fname);

	//remove slash at the front if applicable
	if (fname[0] == '/')
	{
		fname = fname.substr(1, fname.size()-1);
	}

	m_pPropertiesInputScript->set_text(fname);
}
void EntEditMode::OnPropertiesRemoveData()
{
	//kill all selected items
	
	for (int i=0; i < m_pPropertiesListData->get_count(); i++)
	{
		CL_ListItem *pItem = m_pPropertiesListData->get_item(i);

		if (pItem->selected)
		{
			m_pPropertiesListData->remove_item(i);
			i = -1;
		}
	}
}

/*
bool SetDialogTabIdByButtonName(CL_InputDialog &dlg, string buttonName, int tabID)
{
	//run through the children components
	
	std::list<CL_Component*>::const_iterator itor = dlg.get_children().begin();

	while (itor != dlg.get_children().end())
	{
		 CL_Component * pComp = (*itor);

		 if (pComp->get_tab_id() == -1)
		 {
			 //uh, do extra validation to make sure it's the ok button later...
			 pComp->set_tab_id(tabID);
			 return true;
		 }
		 
		itor++;
	}
	
	return false; //couldn't find it
}
*/

void CreateEditDataDialog(DataObject &o)
{
   CL_InputDialog dlg("Edit Data Dialog", "Ok", "Cancel", "",GetApp()->GetGUI());
   dlg.set_event_passing(false);

   CL_InputBox *pName = dlg.add_input_box("Name", o.m_key, 600);
   pName->set_tab_id(0);

   CL_InputBox *pValue = dlg.add_input_box("Value", o.Get(), 600);
   pValue->set_tab_id(1);
   dlg.get_button(0)->set_tab_id(2);
   dlg.get_button(1)->set_tab_id(3);

   dlg.set_focus();
   pName->set_focus();
   dlg.run();
   
   if (dlg.get_result_button() == 0)
   {
      if (pName->get_text().size() > 0)
	  {
	   o.Set(pName->get_text(), pValue->get_text());
	  } else
	  {
		LogMsg("Can't have a key with no name, ignoring input.");
	  }
   }

}

void EntEditMode::OnPropertiesAddData()
{
	DataObject o;
	CreateEditDataDialog(o);
	if (o.m_key.size() > 0)
	m_pPropertiesListData->insert_item(PropertyMakeItemString(o));
}

string EntEditMode::PropertyMakeItemString(DataObject &o)
{
	return ("Name: |" + o.m_key+"|            Value: |"+o.Get()+"|");
}

void EntEditMode::OnPropertiesEditData(const CL_InputEvent &input)
{
	int index = m_pPropertiesListData->get_item(input.mouse_pos);
	if (index == -1) return;

	CL_ListItem *pItem = m_pPropertiesListData->get_item(index);

	vector<string> words = CL_String::tokenize(pItem->str, "|", false);
	string name = words[1].c_str();
	string value = words[3].c_str();

	DataObject o;
	o.Set(name, value);

	CreateEditDataDialog(o);

	//update the listbox
	m_pPropertiesListData->change_item(PropertyMakeItemString(o), index);
	
}

void EntEditMode::PropertiesSetDataManagerFromListBox(DataManager *pData, CL_ListBox &listBox)
{
	pData->Clear();

	string name;
	string value; 

	//cycle through and add each key/value pair
	for (int i=0; i < m_pPropertiesListData->get_count(); i++)
	{
		CL_ListItem *pItem = m_pPropertiesListData->get_item(i);

		vector<string> words = CL_String::tokenize(pItem->str, "|", false);
		name = words[1].c_str();
		value = words[3].c_str();

		//OPTIMIZE:  Later, we should examine and if we find a # value add it as a # instead of a string,
		pData->Set(name, value); //it will create it if it doesn't exist
	}
}


void EntEditMode::OnPropertiesConvert()
{
	m_guiResponse = C_GUI_CONVERT;
}
void EntEditMode::BuildTilePropertiesMenu(CL_Vector2 *pVecMouseClickPos, CL_Vector2 *pVecWorld, TileEditOperation *pTileList)
{
	//change options on one or many tiles

	if (pTileList->IsEmpty())
	{
		CL_MessageBox::info("No tiles selected.  Left click on a tile first.", GetApp()->GetGUI());
		return;
	}
	int tilesSelected = pTileList->m_selectedTileList.size();

	if ( tilesSelected == 0 || ! (*pTileList->m_selectedTileList.begin())->m_pTile)
	{
		CL_MessageBox::info("No actual tile layer data exist in the current selection.", GetApp()->GetGUI());
		return;
	}

	selectedTile_list::iterator itor = pTileList->m_selectedTileList.begin();

	if (pTileList->m_selectedTileList.size() == 0)
	{
		LogMsg("No tile selected error");
		return;
	}
	Tile *pTile = (*pTileList->m_selectedTileList.begin())->m_pTile->CreateClone();

	string st;
	st = "Tile/Entity Properties ("+ CL_String::to(tilesSelected) + " selected)";

	// Creating InputDialog

	MovingEntity *pEnt = NULL;

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		//note, we're getting the pointer to the REAL tile, not the copy we had made.  We do our
		//final operations to the real entity and tile because we don't have a nice way to propogate
		//changes to the whole list.  we're only allowing these changes to be made to single-selections
		//as a result

		pEnt = ((TileEntity*)(*pTileList->m_selectedTileList.begin())->m_pTile)->GetEntity();
		assert(pEnt);
	}

//make GUI here
	m_bDialogIsOpen = true; //don't let other operations happen until we're done with this
	
	CL_Point ptSize(370,450);
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), st, 0, GetApp()->GetGUI());
	window.set_event_passing(false);
	m_guiResponse = C_GUI_CANCEL; //default
	window.set_position(300, GetScreenY- (ptSize.y+100));

	CL_SlotContainer slots;

	//add some buttons
	int buttonOffsetX = 10;
	int SecondOffsetX = 100;
	int ThirdOffsetX = 190;
	int offsetY = 10;
	CL_Button buttonCancel (CL_Point(buttonOffsetX+100,ptSize.y-50), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+200,ptSize.y-50), "Ok", window.get_client_area());
	
	CL_CheckBox flipX (CL_Point(buttonOffsetX,offsetY), "Flip X", window.get_client_area());
	flipX.set_checked(pTile->GetBit(Tile::e_flippedX));

	CL_Label labelName(CL_Point(SecondOffsetX, offsetY), "If not blank, entity is locatable by name:", window.get_client_area());
	offsetY+= 20;

	CL_Label labelName2(CL_Point(SecondOffsetX, offsetY), "Name:", window.get_client_area());
	CL_Rect rPos(0,0,100,16);
	rPos.apply_alignment(origin_top_left, - (labelName2.get_width()+labelName2.get_client_x()+5), -(offsetY));
	CL_InputBox inputName(rPos, "", window.get_client_area());


	CL_Button buttonConvert (CL_Point(inputName.get_client_x()+inputName.get_width()+5,offsetY), "Convert to entity", window.get_client_area());


	CL_CheckBox flipY (CL_Point(buttonOffsetX,offsetY),"Flip Y", window.get_client_area());
	offsetY+= 20;
	flipY.set_checked(pTile->GetBit(Tile::e_flippedY));
	
	CL_CheckBox castShadow (CL_Point(buttonOffsetX,offsetY),"Cast Shadow", window.get_client_area());
	castShadow.set_checked(pTile->GetBit(Tile::e_castShadow));

	CL_CheckBox sortShadow (CL_Point(SecondOffsetX,offsetY),"Sort Shadow", window.get_client_area());
	sortShadow.set_checked(pTile->GetBit(Tile::e_sortShadow));

	CL_CheckBox pathNode(CL_Point(ThirdOffsetX,offsetY),"Path Node", window.get_client_area());
	pathNode.set_checked(pTile->GetBit(Tile::e_pathNode));

	offsetY+= 20;
	
	CL_Label labelEntity (CL_Point(buttonOffsetX, offsetY), "LUA Script:", window.get_client_area());

	rPos = CL_Rect(0,0,200,16);
	rPos.apply_alignment(origin_top_left, - (labelEntity.get_width()+buttonOffsetX+5), -(offsetY));
	CL_InputBox inputEntity(rPos, "", window.get_client_area());

	m_pPropertiesInputScript = &inputEntity; //need this for the file open function, messy, I know

	//add a little file open button
	CL_Button buttonOpenScript (CL_Point(inputEntity.get_client_x()+inputEntity.get_width(), offsetY), "File...", window.get_client_area());

	//add edit button
	CL_Button buttonEditScript (CL_Point(buttonOpenScript.get_client_x()+buttonOpenScript.get_width(), offsetY), "Edit", window.get_client_area());
	offsetY+= 20;

	CL_Label labelData (CL_Point(40, offsetY), "Custom data attached to the object:", window.get_client_area());
	offsetY+= 20;

	CL_Point ptOffset = CL_Point(buttonOffsetX, offsetY);
	rPos = CL_Rect(0,0,ptSize.x - (buttonOffsetX*2),100);
	rPos.apply_alignment(origin_top_left, -ptOffset.x, -ptOffset.y);

	CL_ListBox listData(rPos, window.get_client_area());
	//listData.set_multi_selection(true);
	m_pPropertiesListData = &listData;
	offsetY+= 4 + listData.get_height();

	//add a little file open button
	CL_Button buttonAddData (CL_Point(buttonOffsetX, offsetY), "Add", window.get_client_area());
	CL_Button buttonRemoveData (CL_Point(buttonOffsetX+50, offsetY), "Remove Selected", window.get_client_area());
	offsetY += 20;
	//put in a layer select list too
	CL_Label labelLayer(CL_Point(buttonOffsetX, offsetY), "Changing layer affects all selected", window.get_client_area());

	offsetY += 20;
	rPos = CL_Rect(0,0, 85, 150);
	ptOffset = CL_Point(buttonOffsetX, offsetY);
	rPos.apply_alignment(origin_top_left, -ptOffset.x, -ptOffset.y);

	CL_ListBox layerList(rPos, window.get_client_area());
	LayerManager &layerMan = GetWorld->GetLayerManager();

	vector<unsigned int> layerVec;
	layerMan.PopulateIDVectorWithAllLayers(layerVec);
	//sort it
	std::sort(layerVec.begin(), layerVec.end(), compareLayerBySort);
	int originalLayer;

	unsigned int layers = layerMan.GetLayerCount();
	for (unsigned int i=0; i < layers; i++)
	{
		int index = layerList.insert_item(layerMan.GetLayerInfo(layerVec[i]).GetName());
		layerList.get_item(i)->user_data = layerVec[i];
		if (pTile->GetLayer() == i)
		{
			originalLayer = i; //so we can tell if it changed later
			layerList.set_selected(i, true);
		}
	}

	 CL_Label labelName3(CL_Point(SecondOffsetX, offsetY), "Base Color:", window.get_client_area());
	 rPos = CL_Rect(0,0,110,16);
	 rPos.apply_alignment(origin_top_left, - (labelName3.get_width()+labelName3.get_client_x()+5), -(offsetY));
	 
	 string originalColor = ColorToString(pTile->GetColor());
	 CL_InputBox inputColor(rPos, originalColor, window.get_client_area());

	 offsetY += 20;

	 CL_Label labelName4(CL_Point(SecondOffsetX, offsetY), "Scale X/Y:", window.get_client_area());
	 rPos = CL_Rect(0,0,110,16);
	 rPos.apply_alignment(origin_top_left, - (labelName4.get_width()+labelName3.get_client_x()+5), -(offsetY));

	 string originalScale =  CL_String::from_float(pTile->GetScale().x) + " " + CL_String::from_float(pTile->GetScale().y);
	 CL_InputBox inputScale(rPos, originalScale, window.get_client_area());



if (pTile->GetType() == C_TILE_TYPE_PIC && tilesSelected == 1)
{
	buttonConvert.enable(true);
} else
{
	buttonConvert.enable(false);
}

const char C_MULTIPLE_SELECT_TEXT[] = "<multiple selected>";

	if (pEnt)
	{
	
		if (tilesSelected > 1)
		{
			inputName.set_text(C_MULTIPLE_SELECT_TEXT);
			inputName.enable(false);
			inputEntity.set_text(C_MULTIPLE_SELECT_TEXT);
		
			listData.enable(false);
			buttonAddData.enable(false);
			buttonRemoveData.enable(false);

		
		} else
		{
			inputEntity.set_text(pEnt->GetMainScriptFileName());
			inputName.set_text(pEnt->GetName());
		}
	
	} else
	{
		inputEntity.set_text("<N/A, not an entity>");
		inputEntity.enable(false);
		buttonOpenScript.enable(false);
		buttonEditScript.enable(false);
		inputName.enable(false);
		listData.enable(false);
		buttonAddData.enable(false);
		buttonRemoveData.enable(false);
	}


	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);

	//on ok, run our ok function and then kill the window
	slots.connect(buttonOk.sig_clicked(), this, &EntEditMode::OnPropertiesOK);
	slots.connect(buttonOk.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);

	slots.connect(buttonConvert.sig_clicked(), this, &EntEditMode::OnPropertiesConvert);
	slots.connect(buttonConvert.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);


	slots.connect(buttonOpenScript.sig_clicked(), this, &EntEditMode::OnPropertiesOpenScript);
	slots.connect(buttonEditScript.sig_clicked(), this, &EntEditMode::OnPropertiesEditScript);

	slots.connect(buttonAddData.sig_clicked(), this, &EntEditMode::OnPropertiesAddData);
	slots.connect(buttonRemoveData.sig_clicked(), this, &EntEditMode::OnPropertiesRemoveData);

	slots.connect(listData.sig_mouse_dblclk(), this, &EntEditMode::OnPropertiesEditData);
	
	//populate the listbox

	if (pEnt)
	{
		dataList *dList = pEnt->GetData()->GetList();
		dataList::iterator ditor = dList->begin();

		string stTemp;

		while (ditor != dList->end())
		{
			listData.insert_item(PropertyMakeItemString(ditor->second));
			ditor++;
		}
	}

	listData.sort();
	
	window.run();	//loop in the menu until quit() is called by hitting a button
	
	//Process GUI here

	if (m_guiResponse == C_GUI_OK  )
	{
		unsigned int flags = 0;
		
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			if (inputEntity.get_text() != C_MULTIPLE_SELECT_TEXT)
			{
				((TileEntity*)pTile)->GetEntity()->SetMainScriptFileName(inputEntity.get_text());
				flags = flags | TileEditOperation::eBitScript;
			}
		}
		
		if (flipX.is_checked() != pTile->GetBit(Tile::e_flippedX))
		{
			pTile->SetBit(Tile::e_flippedX, flipX.is_checked());
			flags = flags | TileEditOperation::eBitFlipX;
		}

		if (flipY.is_checked() != pTile->GetBit(Tile::e_flippedY))
		{
			pTile->SetBit(Tile::e_flippedY, flipY.is_checked());
			flags = flags | TileEditOperation::eBitFlipY;
		}

		if (castShadow.is_checked() != pTile->GetBit(Tile::e_castShadow))
		{
			pTile->SetBit(Tile::e_castShadow, castShadow.is_checked());
			flags = flags | TileEditOperation::eBitCastShadow;
		}

		if (sortShadow.is_checked() != pTile->GetBit(Tile::e_sortShadow))
		{
			pTile->SetBit(Tile::e_sortShadow, sortShadow.is_checked());
			flags = flags | TileEditOperation::eBitSortShadow;
		}

		if (pathNode.is_checked() != pTile->GetBit(Tile::e_pathNode))
		{
			pTile->SetBit(Tile::e_pathNode, pathNode.is_checked());
			flags = flags | TileEditOperation::eBitPathNode;
		}

		if (inputColor.get_text() != originalColor)
		{
			pTile->SetColor(StringToColor(inputColor.get_text()));
			flags = flags | TileEditOperation::eBitColor;
		}
	
		if (inputScale.get_text() != originalScale)
		{
			CL_Vector2 vScale = StringToVector(inputScale.get_text());
			//ensure it's not 0, otherwise it will be invisible...
			if (vScale.x == 0) 
			{
				LogError("Bad scale input, setting X to 1 instead of 0.");
				vScale.x = 1;
			}

			if (vScale.y == 0) 
			{
				LogError("Bad scale input, setting Y to 1 instead of 0.");
				vScale.y = 1;
			}
			pTile->SetScale(vScale);
			
			flags = flags | TileEditOperation::eBitScale;
		}
		//run through and copy these properties to all the valid tiles in the selection
		pTileList->CopyTilePropertiesToSelection(pTile, flags);


		if (pEnt && pTileList->m_selectedTileList.size() == 1)
		{
			//it's a single ent, so extra property changes can be made too
			if (inputName.get_text() != pEnt->GetName())
			{
				pEnt->SetName(inputName.get_text());
			}
			PropertiesSetDataManagerFromListBox(pEnt->GetData(), listData);
		}

		int selectedLayer = layerList.get_current_item();
		if (selectedLayer == -1) selectedLayer = originalLayer;
	
		if ( selectedLayer != originalLayer)
		{
			pTileList->SetForceLayerOfNextPaste(selectedLayer);
			//LogMsg("Changing layer to %d", layerList.get_item(selectedLayer)->user_data);
		}
		
		pTileList->SetIgnoreParallaxOnNextPaste();
		
		//paste our selection, this helps by creating an undo for us
		OnPaste(*pTileList, pTileList->GetUpperLeftPos());

		if (selectedLayer != originalLayer)
		{
			//couldn't change it before, because the paste needed it to delete the old tiles			
			pTileList->SetLayerOfSelection(selectedLayer);
		}

		GetWorld->ReInitEntities();

		//update our current selection
		pTileList->UpdateSelectionFromWorld();

	} else if (m_guiResponse == C_GUI_CONVERT)
	{
		MovingEntity *pEnt = new MovingEntity;
		TileEntity *pNewTile = new TileEntity;
		pNewTile->SetEntity(pEnt);
		pEnt->SetPos(pTile->GetPos());
		pNewTile->SetBit(Tile::e_flippedX, pTile->GetBit(Tile::e_flippedX));
		pNewTile->SetBit(Tile::e_flippedY, pTile->GetBit(Tile::e_flippedY));
		pNewTile->SetBit(Tile::e_castShadow, pTile->GetBit(Tile::e_castShadow));
		pNewTile->SetBit(Tile::e_sortShadow, pTile->GetBit(Tile::e_sortShadow));
		pNewTile->SetBit(Tile::e_pathNode, pTile->GetBit(Tile::e_pathNode));
		pNewTile->SetLayer(pTile->GetLayer());
		pEnt->SetImageFromTilePic((TilePic*)pTile);
		pEnt->SetMainScriptFileName("");
		pEnt->Init();

		//now, we could be done, but we can be nice and try to adjust the collision to be centered
		//instead of how it was

		if (pEnt->GetCollisionData()->HasData())
		{
			PointList *pLine;

			CL_Rectf r = pEnt->GetBoundsRect();
			CL_Vector2 vOffset = CL_Vector2(-r.get_width()/2, -r.get_height()/2);
			CL_Vector2 vMoveOffset = CL_Vector2::ZERO;

			line_list::iterator listItor;

			//for each line...

			for (listItor = pEnt->GetCollisionData()->GetLineList()->begin(); 
				listItor != pEnt->GetCollisionData()->GetLineList()->end(); listItor++)
			{
				pLine = &(*listItor);
				
				vMoveOffset = pLine->GetOffset();
				
				pLine->RemoveOffsets();
				pLine->SetOffset(vOffset);
				pLine->CalculateOffsets();
		
				break; //actually, only process the first one
			}

			//move position to match where it was
			pEnt->SetPos(pEnt->GetPos()+vMoveOffset);
		}

		OnDelete(); //kill the old one
		
		m_selectedTileList.ClearSelection();
		m_selectedTileList.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
			false, pNewTile);
	
		m_selectedTileList.SetIgnoreParallaxOnNextPaste();
		//paste our selection, this helps by creating an undo for us
		OnPaste(m_selectedTileList, m_selectedTileList.GetUpperLeftPos());
		GetWorld->ReInitCollisionOnTilePics();

	}

	m_bDialogIsOpen = false;
	SAFE_DELETE(pTile);
	m_pPropertiesInputScript= NULL;
	m_pPropertiesListData = NULL;
}

void EntEditMode::GetSettingsFromWorld()
{
		
	if (GetWorld)	
	{
		m_pCheckBoxSnap->set_checked(GetWorld->GetSnapEnabled());
		m_snapSize = GetWorld->GetDefaultTileSize();
		m_pInputBoxSnapSize->set_text(CL_String::from_int(m_snapSize));
	}
}

void EntEditMode::OnMapChange()
{
	GetSettingsFromWorld();
}
