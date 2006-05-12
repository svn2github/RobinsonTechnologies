#include "AppPrecomp.h"
#include "Console.h"
#include "Main.h"
#include "AppUtils.h"

Console g_Console;
#define C_MAX_LOG_LINES 50

Console::Console()
{
	m_bOnScreen = false;
}

void Console::Init()
{
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &Console::OnKeyDown);

}
Console::~Console()
{

}

void Console::OnKeyDown(const CL_InputEvent &key)
{
	switch (key.id)
	{

	case 192: //VK_OEM_3, backtick for US keyboards
		m_bOnScreen = !m_bOnScreen;
		//LogMsg("Toggling console display");
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
	item.m_color = CL_Color(255,50,50,255);
	AddGeneric(item);
  
	SetOnScreen(true);
}

void Console::Render()
{
	if (!m_bOnScreen) return;
	//figure out overall size
	CL_Rect r(0,GetScreenY/6, GetScreenX, GetScreenY);
	CL_Display::fill_rect(r, CL_Color(255,255,255,235));

	CL_Font *pFont = GetApp()->GetFont(C_FONT_GRAY);
	ResetFont(pFont);

//cycle through them
	console_cont::reverse_iterator itor;

	int posy = r.bottom-15;
	int posx = 10;
	for (itor = m_lineVec.rbegin(); itor != m_lineVec.rend(); itor++)
	{
		pFont->set_color(itor->m_color);

		vector<string> lines = CL_String::tokenize(itor->m_text, "\n", false);
		for (unsigned int i=0; i < lines.size(); i++)
		{
			pFont->draw(posx, posy, lines[i]);
			posy -= pFont->get_height();
		}
	}

	//pFont->set_alpha(1);
	
}
