#include "AppPrecomp.h"
#include "GameLogic.h"
#include "EntVisualProfileEditor.h"
#include "VisualProfileManager.h"

EntVisualProfileEditor::EntVisualProfileEditor(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pWindow = NULL;
	m_pListAnims = NULL;
	m_pEnt = NULL;

	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntVisualProfileEditor::OnButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntVisualProfileEditor::OnButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntVisualProfileEditor::OnMouseMove);
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &EntVisualProfileEditor::OnButtonDown);

	SetName("visualeditor");
	LogMsg("Creating visual profile editor");
}

EntVisualProfileEditor::~EntVisualProfileEditor()
{
	SAFE_DELETE(m_pWindow);
}

bool EntVisualProfileEditor::IsDialogOpen()
{
	//if yes, keyboard and mouse editting controls will be killed so we don't zoom or do things while editing text
	return false;
}

void EntVisualProfileEditor::OnEditorClosed(int entID)
{
	OnClose();
}

void EntVisualProfileEditor::OnEntityDeleted(int entID)
{
	//well, the entity we were editing was just deleted, this could happen when the game is shutdown.
	//Don't bother saving..
	SetDeleteFlag(true);

}

void EntVisualProfileEditor::OnClose()
{
	//save and close

	if (GetDeleteFlag()) return; //this was called more than once?

	if (m_pEnt->GetVisualProfile())
	{
		m_pEnt->GetVisualProfile()->GetParentVisualResource()->Save();
	}

	
	SetDeleteFlag(true);

	//now, we don't really have to do this, and we shouldn't if it's slow, but it's less confusing if we reinit all possible sprites that could
	//have been affected.

	GetActiveMap->ReInitEntities();
		

}

int SetListBoxByString(CL_ListBox &lbox, const string &st)
{

	vector<CL_ListItem*> &items = lbox.get_items();

	for (unsigned int i=0; i < items.size(); i++)
	{
		if (items[i]->str == st)
		{
			//found match
			lbox.set_current_item(i);
			return i;
		}
	}
	return -1; //error
}

void EntVisualProfileEditor::OnChangeAnim()
{
	m_pEnt->SetAnimByName(m_pListAnims->get_current_text());
}

bool EntVisualProfileEditor::Init(MovingEntity *pEnt)
{
	assert(!m_pWindow && "We don't support initting things twice");

	m_pEnt = pEnt;

	//build the GUI window

	CL_Point ptSize(400,150);
	m_pWindow = new CL_Window(CL_Rect(0, 0, ptSize.x, ptSize.y), "Visual Profile Palette", 0, GetApp()->GetGUI());
	m_pWindow->set_position(200, GetScreenY-ptSize.y);
	m_pWindow->set_event_passing(false);

	int buttonOffsetX = 10;
	int curY = 10;
	CL_Button *pButton = new CL_Button(CL_Point(buttonOffsetX,ptSize.y-50), "Close", m_pWindow->get_client_area());

	new CL_Label(CL_Point(buttonOffsetX, curY), "Use the arrow keys to adjust the offset of the selected animation.  Hold", m_pWindow->get_client_area()); curY += 16;
	new CL_Label(CL_Point(buttonOffsetX, curY), "shift to move in larger increments.", m_pWindow->get_client_area()); curY += 16;
	new CL_Label(CL_Point(buttonOffsetX, curY), "", m_pWindow->get_client_area()); curY += 16;
	new CL_Label(CL_Point(buttonOffsetX, curY), "Use Control+Up/Down to quickly change\nanimations.", m_pWindow->get_client_area()); curY += 16;

	m_slots.connect(pButton->sig_clicked(), this, &EntVisualProfileEditor::OnClose);

	//while we're at it, if they close the edit mode window, let's also close ourself
	m_slots.connect(EntityMgr->GetEntityByName("edit mode")->sig_delete, this, &EntVisualProfileEditor::OnEditorClosed);

	//what if the entity itself is destroy for some reason?  handle that too
	m_slots.connect(m_pEnt->sig_delete, this, &EntVisualProfileEditor::OnEntityDeleted);


	//set the list of anims that can be clicked

	vector<string> vecAnims = m_pEnt->GetVisualProfile()->GetListOfActiveAnims();

	CL_Rect r = m_pWindow->get_children_rect();

	CL_Size sz(150,120);
	int rightEdgeOffset = 10;
	int bottomEdgeOffset = 30;
	r.left = r.get_width()-sz.width;
	r.right -= rightEdgeOffset;
	r.top = r.get_height() - sz.height;
	r.bottom -= bottomEdgeOffset;

	m_pListAnims = new CL_ListBox(r, m_pWindow->get_client_area());

	for (unsigned int i=0; i < vecAnims.size(); i++)
	{
		m_pListAnims->insert_item(vecAnims[i].c_str());
	}

	m_slots.connect( m_pListAnims->sig_selection_changed(), this, &EntVisualProfileEditor::OnChangeAnim);

	SetListBoxByString(*m_pListAnims, m_pEnt->GetVisualProfile()->AnimIDToText(m_pEnt->GetAnimID()));

	return true; //success
}

void EntVisualProfileEditor::OnMouseMove(const CL_InputEvent &key)
{
	
}


void EntVisualProfileEditor::OnButtonUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		
		break;
	}
}

void EntVisualProfileEditor::ModifyActiveAnim(CL_Point pt)
{
	CL_Origin o;
	int x,y;

	int mult = 1;

	if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
	{
		mult = 5; //move much more if shift is pressed
	}

	CL_Sprite *pSprite = m_pEnt->GetVisualProfile()->GetSpriteByAnimID(m_pEnt->GetAnimID());


	if (!pSprite)
	{
		LogError("No active sprite right now?  Can't adjust it");
		return;
	}

	pSprite->get_alignment(o, x,y);

	//apply modification
	x += pt.x * mult;
	y += pt.y * mult;

	//put it back in
	pSprite->set_alignment(o,x,y);

	//update the visuals so we can see the changes
	m_pEnt->ForceSpriteUpdate(); //otherwise it would ignore the change, thinking it already has it active
	m_pEnt->SetSpriteData(pSprite);

}

void EntVisualProfileEditor::MoveAnimSelection(int offset)
{
	int selected = m_pListAnims->get_current_item();
	selected = altmod(selected + offset, m_pListAnims->get_count());
	m_pListAnims->set_current_item(selected);
	m_pListAnims->set_top_item(selected);
}

void EntVisualProfileEditor::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_RIGHT:
		ModifyActiveAnim(CL_Point(-1,0));
		break;

	case CL_KEY_LEFT:
		ModifyActiveAnim(CL_Point(1,0));
		break;

	case CL_KEY_UP:
		
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			MoveAnimSelection(-1);
			return;
		}
		
		ModifyActiveAnim(CL_Point(0,1));
		break;

	case CL_KEY_DOWN:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			MoveAnimSelection(1);
			return;
		}
		ModifyActiveAnim(CL_Point(0,-1));
		break;

	case CL_KEY_DELETE:
		break;

	case CL_MOUSE_LEFT:
		break;

	case CL_MOUSE_RIGHT:
		break;
	}
}


void EntVisualProfileEditor::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext*) pTarget;

	DrawBullsEyeWorld(m_pEnt->GetPos(), CL_Color(255,255,0,200), 10, pGC); //draw his center point

}