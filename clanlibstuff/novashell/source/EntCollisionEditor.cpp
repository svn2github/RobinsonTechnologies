#include "AppPrecomp.h"
#include "Screen.h"
#include "EntCollisionEditor.h"
#include "Tile.h"
#include "GameLogic.h"
#include "physics/Body.h"
#include "MaterialManager.h"
#include "MovingEntity.h"

#define C_COL_VERT_SIZE 6.0f

EntCollisionEditor::EntCollisionEditor(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_bDataChanged = false;
	m_pWindow = NULL;
	m_pLabel = NULL;
	m_pCollisionData = NULL;
	m_pRadioGroup = NULL;
	m_pListType = NULL;
	m_pCheckBoxSnap = NULL;
	m_pCheckBoxClipToImage = NULL;
	
	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntCollisionEditor::OnButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntCollisionEditor::OnButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntCollisionEditor::OnMouseMove);
	
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &EntCollisionEditor::OnButtonDown);
	m_mode = e_modeAddCreate;
	m_pButtonOK = m_pButtonCancel = NULL;
	ClearSelection();
	m_recalculateOffsetsWhenDone = true;

}

EntCollisionEditor::~EntCollisionEditor()
{
	Kill();
}

void EntCollisionEditor::Kill()
{
	SAFE_DELETE(m_pButtonOK);
	SAFE_DELETE(m_pButtonCancel);
	SAFE_DELETE(m_pRadioGroup); 
	SAFE_DELETE(m_pLabel);
	SAFE_DELETE(m_pWindow);
}

void EntCollisionEditor::OnMouseMove(const CL_InputEvent &key)
{
	CL_Vector2 vecScreenOld = CL_Vector2(m_vecLastMousePos.x, m_vecLastMousePos.y);
	CL_Vector2 vecScreenNew = CL_Vector2(key.mouse_pos.x, key.mouse_pos.y);
	CL_Vector2 vecWorldDisp = GetWorldCache->ScreenToWorld(vecScreenNew) - GetWorldCache->ScreenToWorld(vecScreenOld);
	
		if (m_mode == e_modeAdjust)
	{
		if (m_dragVertID != C_INVALID_VERT)
		{
			//move it
			if (m_pActiveLine)
			{
				if (m_pSelectedVert)
				{
					//force it within range
					CL_Vector2 localPos = m_dragStartPos += vecWorldDisp;

					ApplySnap(localPos);

					if (m_pCheckBoxClipToImage->is_checked())
					{

						localPos.x = max(localPos.x, m_rectArea.left);
						localPos.x = min(localPos.x, m_rectArea.right);

						localPos.y = max(localPos.y, m_rectArea.top);
						localPos.y = min(localPos.y, m_rectArea.bottom);
					}
					
						
					*m_pSelectedVert = localPos;
				}
			}

		}
	}
	m_vecLastMousePos = key.mouse_pos;
}

void EntCollisionEditor::ApplySnap(CL_Vector2 &vec)
{
	if (m_pCheckBoxSnap->is_checked())
	{
		vec.x = int(vec.x);
		vec.y = int(vec.y);
	}
}

void EntCollisionEditor::OnButtonUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
		case CL_MOUSE_LEFT:
			if (m_mode == e_modeAdjust)
			{
				//snap them
				CL_Vector2 vecPos =GetWorldCache->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
				AdjustVert(vecPos);
			}
			break;
	}

	ClearSelection();

}

void EntCollisionEditor::AddCreateVert(CL_Vector2 vecPos)
{
	CL_Vector2 localPos = vecPos - m_pos;
	ApplySnap(localPos);

	if (m_pCheckBoxClipToImage->is_checked())
	{

		localPos.x = max(localPos.x, m_rectArea.left);
		localPos.x = min(localPos.x, m_rectArea.right);

		localPos.y = max(localPos.y, m_rectArea.top);
		localPos.y = min(localPos.y, m_rectArea.bottom);

	}

	if (m_rectArea.is_inside(CL_Pointf(localPos.x, localPos.y)))
	{
		//it's a click in a valid position
		if (m_pActiveLine)
		{
			//line is available to add this plot point to
			m_pActiveLine->GetPointList()->push_back(localPos);
		}
	}

}

void EntCollisionEditor::ClearSelection()
{
	m_pSelectedVert = NULL;
	m_dragVertID = C_INVALID_VERT;
}

void EntCollisionEditor::DeleteVert(CL_Vector2 vecPos)
{
	CL_Vector2 localPos = vecPos - m_pos;

	CL_Vector2 *m_pSelectedVert;
	unsigned int vertID;
	
	CL_Vector2 vScale = GetCamera->GetScale();

	GetIndexOfVertAtThisPos(*m_pActiveLine->GetPointList(), localPos, &m_pSelectedVert, vertID, C_COL_VERT_SIZE/vScale.x);

	if (m_pSelectedVert)
	{
		//delete this vert they clicked
		m_pActiveLine->GetPointList()->erase(m_pActiveLine->GetPointList()->begin() + vertID);
		ClearSelection();
	}
	
}

bool EntCollisionEditor::GetIndexOfVertAtThisPos(point_list &pList, CL_Vector2 &vLocalPos, CL_Vector2 ** ppVertOut, unsigned int &vertIDOut, float fPadding)
{

	CL_Rectf r(vLocalPos.x-fPadding, vLocalPos.y-fPadding, vLocalPos.x+fPadding, vLocalPos.y+fPadding );

	vertIDOut = C_INVALID_VERT;
	*ppVertOut = NULL;
	for (unsigned int i=0; i < pList.size(); i++)
	{

		if (r.is_inside(*(CL_Pointf*)&pList[i]))
		{
			//found it
			*ppVertOut = &pList[i];
			vertIDOut = i;
			return true;
		}
	}

	return false;
}

void EntCollisionEditor::AdjustVert(CL_Vector2 vecPos)
{
	
	CL_Vector2 localPos = vecPos - m_pos;
	point_list &pointList = *m_pActiveLine->GetPointList(); 
	CL_Vector2 vScale = GetCamera->GetScale();
	
	GetIndexOfVertAtThisPos(pointList, localPos, &m_pSelectedVert, m_dragVertID, C_COL_VERT_SIZE/vScale.x);
	if (m_pSelectedVert)
	{
		m_dragStartPos = *m_pSelectedVert;
	}
}

void EntCollisionEditor::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_DELETE:

		if (m_dragVertID != C_INVALID_VERT)
		{
			m_pActiveLine->GetPointList()->erase(m_pActiveLine->GetPointList()->begin() + m_dragVertID);
			m_dragVertID = C_INVALID_VERT;
		}
		break;
	
	case CL_MOUSE_LEFT:
		{
	  	 
			CL_Vector2 vecPos =GetWorldCache->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			switch (m_mode)
			{
			case e_modeAddCreate:
				AddCreateVert(vecPos);
				break;

			case e_modeAdjust:
				AdjustVert(vecPos);
				break;

			/*
			case e_modeDelete:
				DeleteVert(vecPos);
				break;
				*/

			default: ;

			}
		}
		break;

	case CL_MOUSE_RIGHT:
		
			//close the shape?
			if (m_pActiveLine)
			{
				int pointCount = m_pActiveLine->GetPointList()->size();
				if (pointCount > 1)
				{
					
					if (m_pActiveLine->GetPointList()->at(0) != m_pActiveLine->GetPointList()->at(pointCount-1))
					{
						//close the poly
						m_pActiveLine->GetPointList()->push_back(*m_pActiveLine->GetPointList()->begin());
						
					
					} else
					{
						//it already seems to be closed
					}
					
				}
			}
			break;
	}
}

void EntCollisionEditor::OnRadioButtonChange(CL_RadioButton *pSelectedButton)
{
	ClearSelection();

	for (int i=0; i < e_modeCount; i++)
	{
		if (pSelectedButton == m_pRadioArray[i])
		{
			m_mode = i;
		}
	}

	switch (m_mode)
	{
	case e_modeAddCreate:
		m_pLabel->set_text("Left click to add points, click on another vertex to attach\nit.");
		break;
	case e_modeAdjust:
		m_pLabel->set_text("Click and drag verts to move, tap DELETE while dragging\nto remove a vertex.");
		break;
	}
}

void EntCollisionEditor::OnOk()
{
	//accept changes
	
	m_bDataChanged = true;
	*m_pCollisionData = m_col; //copy it back
	
	if (m_recalculateOffsetsWhenDone)
	m_pCollisionData->RecalculateOffsets();
	m_pCollisionData->SetDataChanged(true);

	LogMsg("Final collision edit data:");
	LogMsg( (string("Offset is ") + PrintVector(m_pCollisionData->GetLineList()->begin()->GetOffset())).c_str());

	m_pCollisionData->GetLineList()->begin()->ComputeConvexHull();
	m_pCollisionData->GetLineList()->begin()->PrintPoints();

	SetDeleteFlag(true);

}

void EntCollisionEditor::OnCancel()
{
	SetDeleteFlag(true);
}

void EntCollisionEditor::OnChangeLineType()
{

	if (m_pActiveLine && m_pListType->get_current_item() != -1)
	{
		m_pActiveLine->SetType(m_pListType->get_current_item());
	}
}

void EntCollisionEditor::OnClear()
{
	if (ConfirmMessage("Clear active line", "Are you sure you want to clear all points on the active line?"))
	{
		if (m_pActiveLine)
		{
			m_pActiveLine->GetPointList()->clear();
			m_mode = e_modeAddCreate;
			m_pRadioGroup->set_checked(m_pRadioArray[m_mode]);
		}
	}
}

void EntCollisionEditor::Init(CL_Vector2 vPos, CL_Rect vEditBounds, CollisionData *pCollisionData, bool calcOffsets)
{
	m_pCollisionData =  pCollisionData; //remember location of the original and hope it doesn't change
	m_col = *pCollisionData; //make a local copy

	m_rectArea = vEditBounds;
	
	if (!calcOffsets)
	{
			m_recalculateOffsetsWhenDone = false;
	} else
	{
		m_col.RemoveOffsets();

	}
	m_pos = vPos;

	//create our GUI window at the bottom of the screen
	CL_Point ptSize(400,150);
	m_pWindow = new CL_Window(CL_Rect(0, 0, ptSize.x, ptSize.y), "Collision Edit Palette", 0, GetApp()->GetGUI());
	m_pWindow->set_position(200, GetScreenY-ptSize.y);

	int buttonOffsetX = 10;
	m_pButtonOK = new CL_Button(CL_Point(buttonOffsetX,ptSize.y-50), "Save Changes", m_pWindow->get_client_area());
	m_pButtonCancel = new CL_Button(CL_Point(buttonOffsetX+100,ptSize.y-50), "Cancel", m_pWindow->get_client_area());
	CL_Button *pButtonClear = new CL_Button(CL_Point(buttonOffsetX+200,ptSize.y-50), "Clear", m_pWindow->get_client_area());

	m_slots.connect(m_pButtonOK->sig_clicked(), this, &EntCollisionEditor::OnOk);
	m_slots.connect(m_pButtonCancel->sig_clicked(), this, &EntCollisionEditor::OnCancel);
	m_slots.connect(pButtonClear->sig_clicked(), this, &EntCollisionEditor::OnClear);

	int heightOfBut = 16;

	m_pRadioArray[e_modeAddCreate] = new CL_RadioButton(CL_Point(2,2), "Add Verts", m_pWindow->get_client_area());
	m_pRadioArray[e_modeAdjust] = new CL_RadioButton(CL_Point(2,2+ heightOfBut), "Adjust Verts", m_pWindow->get_client_area());
	//m_pRadioArray[e_modeDelete] = new CL_RadioButton(CL_Point(2,2 + (heightOfBut*2)), "Delete", m_pWindow->get_client_area());
	
	m_pRadioGroup = new CL_RadioGroup();
	
	for (int i=0; i < e_modeCount; i++)
	{
		m_pRadioGroup->add(m_pRadioArray[i], true);
	}

	m_slots.connect(m_pRadioGroup->sig_selection_changed(), this, &EntCollisionEditor::OnRadioButtonChange);

	CL_Rect r = m_pWindow->get_children_rect();
	//r.set_size(r.get_size() - CL_Size(5,28));

	CL_Size sz(100,120);
	int rightEdgeOffset = 10;
	int bottomEdgeOffset = 30;

	r.left = r.get_width()-sz.width;
	r.right -= rightEdgeOffset;

	r.top = r.get_height() - sz.height;
	r.bottom -= bottomEdgeOffset;

	m_pListType = new CL_ListBox(r, m_pWindow->get_client_area());
	
	//populate with our materials that were set from lua
	for (int i=0; i < g_materialManager.GetCount(); i++)
	{
		m_pListType->insert_item(g_materialManager.GetMaterial(i)->GetName());
	}

	//setup our own tip display
	m_pLabel = new CL_Label(CL_Point(100,1),"", m_pWindow->get_client_area());

	new CL_Label(CL_Point(ptSize.x-190,50),"Material Type:", m_pWindow->get_client_area());

	m_pWindow->set_event_passing(false);

	if (m_col.GetLineList()->size() == 0)
	{
		m_mode = e_modeAddCreate;
		//add one to get them started
		PointList pl;
		m_col.GetLineList()->push_back(pl);
	} else
	{
		m_mode = e_modeAdjust;
	}

	m_pActiveLine = &(*m_col.GetLineList()->begin());
	
	m_pListType->set_selected(m_pActiveLine->GetType(), true);
	m_pRadioGroup->set_checked(m_pRadioArray[m_mode]); //this will call OnRadioButtonChanged and set the help text too

	m_slots.connect( m_pListType->sig_selection_changed(), this, &EntCollisionEditor::OnChangeLineType);
	CL_Point offset = CL_Point(2,40);
	m_pCheckBoxSnap = new CL_CheckBox(offset, "Snap", m_pWindow->get_client_area());
	m_pCheckBoxSnap->set_checked(true);
	m_pCheckBoxSnap->set_focusable(false);

	offset.y += 15;
	m_pCheckBoxClipToImage = new CL_CheckBox(offset, "Clip to image", m_pWindow->get_client_area());
	m_pCheckBoxClipToImage ->set_checked(true);
	m_pCheckBoxClipToImage ->set_focusable(false);

}

void EntCollisionEditor::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext*) pTarget;
	CL_Color col(255,0,0,255);
	RenderVectorCollisionData(m_pos, m_col, pGC, true, &col, NULL);
}

void EntCollisionEditor::Update(float step)
{
}

void DrawCenteredBox(const CL_Vector2 &a, int size, CL_Color col, CL_GraphicContext *pGC)
{
	CL_Rect rec;
	rec.left = a.x - C_COL_VERT_SIZE;
	rec.top = a.y - C_COL_VERT_SIZE;

	rec.right = a.x + C_COL_VERT_SIZE;
	rec.bottom = a.y + C_COL_VERT_SIZE;
	pGC->draw_rect(rec, col);

}

void RenderVectorPointList(const CL_Vector2 &vecPos, PointList &pl, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride, CBody *pCustomBody)
{
	
	CL_Vector2 a,b, firstVert;
	CL_Color col;

	col.color = g_materialManager.GetMaterial(pl.GetType())->GetColor();
	
	if (pColorOveride) col = *pColorOveride;

	unsigned int totalVerts = pl.GetPointList()->size();
	if ( totalVerts == 0)
	{
		return;
	}

	if (!bRenderVertBoxes && pCustomBody)
	{
		//it's connected to a physics system, it's better if we render it a different way, so we can see rotation
		CBody *pBody = &pl.GetAsBody(CL_Vector2(0,0), pCustomBody);
		
		RenderVertexListRotated(vecPos, (CL_Vector2*)pBody->GetVertArray(), pBody->GetVertCount(), col, pGC,
			RadiansToDegrees(pBody->GetOrientation()));
		return;
	}

	a = pl.GetPointList()->at(0);
	a += vecPos;
	
	
	a = GetWorldCache->WorldToScreen(a);
    firstVert = a;

	if (bRenderVertBoxes)
	{
		DrawCenteredBox(a, C_COL_VERT_SIZE, col, pGC);
	}

	if (totalVerts == 1) return;

	unsigned int i=1;
	while(i < totalVerts)
	{
		b = pl.GetPointList()->at(i);
		b += vecPos;


		b = GetWorldCache->WorldToScreen(b);

		if (bRenderVertBoxes)
		{
			DrawCenteredBox(b, C_COL_VERT_SIZE, col, pGC);
		}

		pGC->draw_line(a.x,a.y,b.x,b.y, col);
		a = b;
		i++;
	}
	
	//draw the last line to connect the last vert with the first
	
	a = firstVert;
	pGC->draw_line(b.x,b.y,a.x,a.y, col);

}

void RenderVectorCollisionData(const CL_Vector2 &vecPos, CollisionData &col, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride, CBody *pCustomBody)
{
	line_list *pLineList = col.GetLineList();
	line_list::iterator itor = pLineList->begin();

	while (itor != pLineList->end())
	{
		RenderVectorPointList(vecPos + (*itor).GetOffset(), (*itor), pGC, bRenderVertBoxes, pColorOveride, pCustomBody);
		itor++;
	}
}

void RenderTileListCollisionData(tile_list &tileList, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride)
{
	tile_list::iterator itor = tileList.begin();

	CollisionData col, *pCol;

	while (itor != tileList.end())
	{
			if ((*itor)->UsesTileProperties())
		{
			//we need a customized version
			CreateCollisionDataWithTileProperties((*itor), col);
			RenderVectorCollisionData((*itor)->GetPos(), col, pGC, bRenderVertBoxes, pColorOveride, (*itor)->GetCustomBody());
		} else
		{
			pCol = (*itor)->GetCollisionData();
			if (pCol)
			RenderVectorCollisionData((*itor)->GetPos(), *pCol, pGC, bRenderVertBoxes, pColorOveride, (*itor)->GetCustomBody());
		}
	
		itor++;
	}
}