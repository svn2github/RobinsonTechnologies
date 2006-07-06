#include "AppPrecomp.h"
#include "main.h"
#include "TextManager.h"
#include "MovingEntity.h"

TextManager g_textManager;

TextObject::TextObject(TextManager *pTextManager)
{
	m_pTextManager = pTextManager;
	m_pEntity = NULL;
	m_vecDisplacement = CL_Vector2::ZERO;
}

void TextObject::InitCustom(const string &text, MovingEntity * pEnt, const CL_Vector2 &vecPos,
							const CL_Vector2 &vecMovement, const CL_Color &col, int timetoShowMS,
							int fontID)
{
	m_text = text;
	m_timeCreated = GetApp()->GetGameTick();
	m_timeToShowMS = timetoShowMS;
	m_pEntity = pEnt;
	m_worldPos = vecPos;
	m_vecMovement = vecMovement;
	m_color = col;
	
	SetMode(CUSTOM);

	//looks like we're going to draw, let's setup where in advance
	m_boundingRect = CL_Rect(0,0,1024,1024);
	m_fontID = fontID;

	CL_Font *pFont = GetApp()->GetFont(m_fontID);
	//get the real size of what we have to draw
	m_boundingRect = pFont->bounding_rect(m_boundingRect, m_text);

	if (pEnt)
	{
		//kill the text when this ent dies or is off the screen
		m_slots.connect(pEnt->sig_delete, this, &TextObject::EntDeleted);
	}
}

void TextObject::Init(const string &text, MovingEntity * pEnt, int fontID)
{
	assert(pEnt);
	m_text = text;
	m_timeCreated = GetApp()->GetGameTick();
	m_timeToShowMS = text.size()* 80;
	SetMode(DIALOG);

   //TODO: Figure out where the max macro is?
  if (m_timeToShowMS < 3300) m_timeToShowMS = 3300;

	m_pEntity = pEnt;

	TextObject *pLastText = m_pTextManager->GetLastTextForEntity(pEnt);

	if (pLastText)
	{
		m_bColorOdd = !pLastText->GetColorOdd();
	} else
	{
		m_bColorOdd = false;
	}

	m_color = m_pEntity->GetDefaultTextColor();
	
	if (m_bColorOdd)
	{
		//odd number, let's color it slightly differently
		short r,g,b;

		r = m_color.get_red() - 30;
		g = m_color.get_green() - 30;
		b = m_color.get_blue() - 30;

		//force them to be within range
		r = min(r, 255); r = max(0, r);
		g = min(g, 255); g = max(0, g);
		b = min(b, 255); b = max(0, b);

		m_color = CL_Color(r,g,b);
	} 

	//looks like we're going to draw, let's setup where in advance
	m_boundingRect = CL_Rect(0,0,400,400);
	m_fontID = fontID;

	CL_Font *pFont = GetApp()->GetFont(fontID);
	//get the real size of what we have to draw
	m_boundingRect = pFont->bounding_rect(m_boundingRect, m_text);

	m_slots.connect(pEnt->sig_delete, this, &TextObject::EntDeleted);
}

void TextObject::EntDeleted(int ID)
{
	m_pEntity = NULL;
}


CL_Point TextObject::CalculateTextAvoidenceOffset()
{
	//do a slow scan through everybody to figure out where text already is from this guy

	CL_Point ptOffset = CL_Point(0,0);
	
	textObject_list::iterator itor;
	for (itor = m_pTextManager->GetTextList().begin(); itor != m_pTextManager->GetTextList().end();)
	{
		if (itor->GetEntity() == m_pEntity && itor->GetMode() == DIALOG)
		{
			//it's our same guy!  Wait, is it actually this instance?
			if ( &(*itor) == this)
			{
				//yep, we're done here.
				break;
			} else
			{
				//apply what we can learn
				ptOffset.y -= itor->GetTextRect().get_height(); //extra spacer
			}
		}
		itor++;
	}

	return ptOffset;
}

//return false to delete this object
bool TextObject::UpdateDialog()
{
	if (!m_pEntity) return false;

	m_bVisible = true;
	m_rect = m_boundingRect;
	
	//now we need to position it

	CL_Vector2 entPos = GetWorldCache->WorldToScreen(m_pEntity->GetPos());
	
	m_pos = CL_Point(entPos.x- (m_rect.right/2)  , entPos.y - (m_rect.bottom));

	m_pos.y -= ( (m_pEntity->GetSizeY()/2) * GetCamera->GetScale().x);

	//clip to screen
	m_pos.x = max(0, m_pos.x);
	m_pos.y = max(0, m_pos.y);
	m_pos.x = min(GetScreenX-m_rect.get_width(), m_pos.x);
	m_pos.y = min(GetScreenY-m_rect.get_height(), m_pos.y);

	m_pos += CalculateTextAvoidenceOffset();

	//apply to rect

	m_rect.apply_alignment(origin_top_left, -m_pos.x, -m_pos.y);
	//LogMsg(PrintRect(m_rect).c_str());
	return true;
}

//return false to delete this object
bool TextObject::UpdateCustom()
{
	m_bVisible = true;
	m_rect = m_boundingRect;

	//apply movement
	m_vecDisplacement += m_vecMovement;

	CL_Vector2 entPos = GetWorldCache->WorldToScreen(m_worldPos);
	m_pos = CL_Point(entPos.x- (m_rect.right/2)  , entPos.y - (m_rect.bottom));

	m_pos += CL_Point(m_vecDisplacement.x, m_vecDisplacement.y);
	m_rect.apply_alignment(origin_top_left, -m_pos.x, -m_pos.y);
	
	//LogMsg(PrintRect(m_rect).c_str());
	return true;
}

bool TextObject::Update()
{

	if (m_pEntity)
	{
		if (m_pEntity->GetTile()->GetParentScreen()->GetParentWorldChunk()->GetParentWorld() != GetWorld)
		{
			m_bVisible = false;
			return true;
		}
	}
	int timeLeft = (m_timeCreated+m_timeToShowMS) - GetApp()->GetGameTick();
	
	if (timeLeft < 0)
	{
		//done showing
		return false;
	}
	
	int fadeOutTimeMS = 1000;
	if (timeLeft < fadeOutTimeMS)
	{
		m_alpha = ( float(timeLeft) / float(fadeOutTimeMS));
	} else
	{
		m_alpha = 1;
	}


	switch (m_mode)
	{
	case DIALOG:
		return UpdateDialog();
		break;

	case CUSTOM:
		return UpdateCustom();
		break;

	default:
		assert(0);
	}

	return true; //don't delete yet
}

void TextObject::Render()
{
	if (!m_bVisible) return;


	//draw a semi transparent box around it so we can read the text easier

	CL_Display::fill_rect(m_rect, CL_Color(0,0,0,min(70, (m_alpha*180))));

	CL_Font *pFont = GetApp()->GetFont(m_fontID);
	pFont->set_color(m_color);
	pFont->set_alpha(m_alpha);
	pFont->draw(m_rect, m_text);
}

TextManager::TextManager()
{
	Reset();
}

TextManager::~TextManager()
{
}

void TextManager::Reset()
{
	m_textList.clear();
}

void TextManager::AddCustom(const string &text, const MovingEntity *pEnt, const CL_Vector2 &vecPos,
					  const CL_Vector2 &vecMovement, const CL_Color &col, int timeToShowMS, int fontID)
{
	TextObject t(this);
	m_textList.push_back(t);

	m_textList.rbegin()->InitCustom(text, const_cast<MovingEntity*>(pEnt),
		vecPos, vecMovement, col, timeToShowMS, fontID);

}
void TextManager::Add(const string &text, MovingEntity *pEnt)
{
	if (!pEnt)
	{
		LogError("NULL entity passed into TextManager::Add, ignoring it.");
		return;
	}

	if (!pEnt->GetTile()->GetParentScreen())
	{
		LogMsg("Warning: TextManager type things shouldn't go into the visual Init(), use GameInit()");
		return;
	}
	
	TextObject t(this);
	m_textList.push_back(t);

	m_textList.rbegin()->Init(text, const_cast<MovingEntity*>(pEnt), C_FONT_NORMAL);

}

int TextManager::GetCountOfTextActiveForEntity(const MovingEntity *pEnt)
{
	int count = 0;

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (itor->GetEntity() == pEnt) 
		{
			count++;
		}

		itor++;
	}

	return count;
}


TextObject * TextManager::GetLastTextForEntity(const MovingEntity *pEnt)
{

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (itor->GetEntity() == pEnt) 
		{
			return &(*itor);
		}

		itor++;
	}

	return NULL;  //none existed
}

bool compareTextObjectsBackwards(const TextObject &A, const TextObject &B) 
{
	return (A.GetTimeCreated() > B.GetTimeCreated());
}


void TextManager::Update(float step)
{
	//sort them based on time created
	m_textList.sort(compareTextObjectsBackwards);

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (!itor->Update())
		{
			//time to delete it
			itor = m_textList.erase(itor);
			continue;
		}

		itor++;
	}

}

void TextManager::Render()
{
	//render them out in reverse order

	textObject_list::reverse_iterator itor;
	for (itor = m_textList.rbegin(); itor != m_textList.rend();)
	{
		itor->Render();
		itor++;
	}

}

