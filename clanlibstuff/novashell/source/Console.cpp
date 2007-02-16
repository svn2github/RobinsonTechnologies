#include "AppPrecomp.h"
#include "Console.h"
#include "main.h"
#include "AppUtils.h"
#include "ClanLib/Core/System/clipboard.h"

Console g_Console;
#define C_MAX_LOG_LINES 200
#define C_PAGE_SCROLL_LINES 50



Console::Console()
{ 
	m_bOnScreen = false; 
	m_curViewIndexOffset = 0;
}


void Console::Init()
{
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &Console::OnKeyDown);

}
Console::~Console()
{

}

void Console::CopyToTextBuffer()
{

	string final;

	console_cont::iterator itor;

	for (itor = m_lineVec.begin(); itor != m_lineVec.end(); itor++)
	{
		
		vector<string> lines = CL_String::tokenize(itor->m_text, "\n", false);
		for (unsigned int i=0; i < lines.size(); i++)
		{
			final += lines[i]+"\r\n";
		}
	}

	CL_Clipboard::set_text(final);
}

void Console::OnKeyDown(const CL_InputEvent &key)
{
	switch (key.id)
	{
		case CL_KEY_TILDE: //aka GRAVE or backtick
		m_bOnScreen = !m_bOnScreen;
		//LogMsg("Toggling console display");
		m_curViewIndexOffset = 0; //reset this
		break;


		case CL_KEY_C:
			if (m_bOnScreen && CL_Keyboard::get_keycode(CL_KEY_CONTROL))

			{
				//let's put this in the text buffer
				CopyToTextBuffer();
				LogMsg("Log copied to system text buffer.");
			}
			break;

#ifdef _WIN32
		case CL_KEY_PRIOR:
#else
	case CL_KEY_PAGE_UP:
#endif
			m_curViewIndexOffset += C_PAGE_SCROLL_LINES;
			break;

	
#ifdef _WIN32
	case CL_KEY_NEXT:
#else
	case CL_KEY_PAGE_DOWN:
		LogMsg("Page down");
#endif
		
		m_curViewIndexOffset -= C_PAGE_SCROLL_LINES;
		
		if (m_curViewIndexOffset < 0) m_curViewIndexOffset = 0;
			break;
	}
}

void Console::AddGeneric(ConsoleItem &item)
{
	m_lineVec.push_back(item);

	while (m_lineVec.size() > C_MAX_LOG_LINES)
	{
		m_lineVec.pop_front();
	}
}

void Console::Add(const string line)
{
	ConsoleItem item;
	item.m_text = line;
	AddGeneric(item);
}

void Console::AddError(const string line)
{
	ConsoleItem item;
	item.m_text = line;
	item.m_color = CL_Color(255,150,150,255);
	AddGeneric(item);
  
	SetOnScreen(true);
}

void Console::RenderGUIOverlay()
{

	//draw bar at the top of the screen
	CL_Rect r(0,0,GetScreenX, 15);
	CL_Display::fill_rect(r, CL_Color(200,0,0,255));

	//draw the text over it
	ResetFont(GetApp()->GetFont(C_FONT_GRAY));
	GetApp()->GetFont(C_FONT_GRAY)->set_alignment(origin_center);
	GetApp()->GetFont(C_FONT_GRAY)->draw(GetScreenX/2,7, "System Log - Press ` (backtick) to close, Ctrl-C into system clipboard, Page-Up/Page-Down to scroll");
}

void Console::Render()
{
	if (!m_bOnScreen) return;
	//figure out overall size
	CL_Rect r(0,15, GetScreenX, GetScreenY);
	CL_Display::fill_rect(r, CL_Color(0,0,0,140));

	CL_Font *pFont = GetApp()->GetFont(C_FONT_GRAY);
	
	if (!pFont)
	{
		//serious error, was unable to init a base font
		throw CL_Error("Serious error: Font not initialized, check the log text file.  Did your script run GetGameLogic:InitGameGUI?");
	}
	
	ResetFont(pFont);

//cycle through them
	console_cont::reverse_iterator itor;

	int posy = r.bottom-15;
	int posx = 10;
	
	int linesToSkip = m_curViewIndexOffset;

	for (itor = m_lineVec.rbegin(); itor != m_lineVec.rend(); itor++)
	{
		pFont->set_color(itor->m_color);

		vector<string> lines = CL_String::tokenize(itor->m_text, "\n", false);
		for (unsigned int i=0; i < lines.size(); i++)
		{
			//DrawWithShadow(posx, posy, lines[i], itor->m_color);
	
			if (linesToSkip > 0)
			{
				//this is bad way to implement a scrollback buffer because it's going to get slower the farther you scroll back.  Uh..
				linesToSkip--;
			} else
			{
				pFont->draw(posx, posy, lines[i]);
				posy -= pFont->get_height();

			}
	
		}

		if (posy < 15) 
		{
			//can't fit anymore on the screen
			break;
		}
	}

	//pFont->set_alpha(1);
	
	RenderGUIOverlay();

}
