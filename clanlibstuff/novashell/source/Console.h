//  ***************************************************************
//  Console - Creation date: 05/09/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Console_h__
#define Console_h__


class ConsoleItem
{
public:
	
	ConsoleItem()
	{
		m_color = CL_Color(255,255,255,255);
	}
	string m_text;
	CL_Color m_color;
};

typedef deque <ConsoleItem> console_cont;

class Console
{
public:
	Console();
	virtual ~Console();

	void Add(const string line);
	void AddError(const string line);

	bool GetOnScreen() {return m_bOnScreen;}
	void SetOnScreen(bool bNew) {m_bOnScreen = bNew;}
	void Render();
	void OnKeyDown(const CL_InputEvent &key);
	void Init();

protected:

	void AddGeneric(ConsoleItem &item);
	void CopyToTextBuffer();
	void RenderGUIOverlay();


	console_cont m_lineVec;
	bool m_bOnScreen; //if true, we're displaying it

	CL_SlotContainer m_slots;
};

extern Console g_Console;

#endif // Console_h__

