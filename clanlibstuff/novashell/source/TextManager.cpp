#include "AppPrecomp.h"
#include "main.h"
#include "TextManager.h"
#include "MovingEntity.h"

TextManager g_textManager;

TextObject::TextObject(TextManager *pTextManager)
{
	m_pTextManager = pTextManager;
	m_pEntity = NULL;
}

void TextObject::Init(const string &text, MovingEntity * pEnt)
{
	m_text = text;
	m_timeCreated = GetApp()->GetGameTick();
	m_timeToShowMS = text.size()* 60;
  
   //TODO: Figure out where the max macro is?
  if (m_timeToShowMS < 3000) m_timeToShowMS = 3000;

	m_pEntity = pEnt;

	TextObject *pLastText = m_pTextManager->GetLastTextForEntity(pEnt);

	if (pLastText)
	{
		m_bColorOdd = !pLastText->GetColorOdd();
	} else
	{
		m_bColorOdd = false;
	}
	
	if (m_pEntity)
	{
		m_color = m_pEntity->GetDefaultTextColor();
	} else
	{
		m_color = CL_Color(255,255,255,255);
	}
	
	if (m_bColorOdd)
	{
		//odd number, let's color is slightly differently
			m_color.set_red(min(m_color.get_red(), m_color.get_red()-10));
			m_color.set_green(min( m_color.get_green(), m_color.get_green()-40));
			m_color.set_blue( max(m_color.get_blue(), m_color.get_blue() + 20));
	} 

	//looks like we're going to draw, let's setup where in advance
	m_boundingRect = CL_Rect(0,0,400,400);
	CL_Font *pFont = GetApp()->GetFont(C_FONT_NORMAL);
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
		if (itor->GetEntity() == m_pEntity)
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
bool TextObject::Update()
{
	if (!m_pEntity) return false;
	
	int timeLeft = (m_timeCreated+m_timeToShowMS) - GetApp()->GetGameTick();
	
	if (timeLeft < 0)
	{
		//done showing
		return false;
	}
	
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

	//calculate the color/alpha

	int fadeOutTimeMS = 1000;
	if (timeLeft < fadeOutTimeMS)
	{
		m_alpha = ( float(timeLeft) / float(fadeOutTimeMS));
	} else
	{
		m_alpha = 1;
	}

	m_bVisible = true;
	return true; //don't delete yet
}

void TextObject::Render()
{
	if (!m_bVisible) return;

	CL_Font *pFont = GetApp()->GetFont(C_FONT_NORMAL);
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

void TextManager::Add(const string &text, const MovingEntity *pEnt)
{
	TextObject t(this);
	m_textList.push_back(t);

	m_textList.rbegin()->Init(text, const_cast<MovingEntity*>(pEnt));

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

