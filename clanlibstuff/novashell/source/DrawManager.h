//  ***************************************************************
//  DrawManager - Creation date: 06/30/2008
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2008 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef DrawManager_h__
#define DrawManager_h__

class DrawEvent
{
public:
	
	DrawEvent()
	{
		m_type =TYPE_UNKNOWN;	
	}
	
	enum eEventType
	{
		TYPE_UNKNOWN,
		TYPE_DRAW_LINE
	};

	void Render( CL_GraphicContext *pGC);

	CL_Vector2 m_a,m_b;
	CL_Color m_color;
	int m_drawStyle;
	eEventType m_type;
};

class DrawEventList
{
public:
	
	void Render( CL_GraphicContext *pGC);

	DrawEventList(int layerID)
	{
		m_layerID = layerID;
	}

	DrawEvent * GetNewEvent();
	int m_layerID;

private:
	list <DrawEvent> m_events;
};

class DrawManager
{
public:

	enum eDrawStyle
	{
		C_DRAW_SCREEN_COORDS,
		C_DRAW_WORLD_COORDS
	};

	DrawManager();
	virtual ~DrawManager();

	//I don't use eDrawStyle directly because I don't want to confuse luabind
	void DrawLine(CL_Vector2 a, CL_Vector2 b, CL_Color col, int layerID, int drawStyle );
	void RenderUpToLayerID(int layerID, CL_GraphicContext *pGC);
	void Update(float delta);

private:

	DrawEventList * GetDrawEventList(int layerID); //will create it if needed
	list<DrawEventList> m_eventList;
	int m_lastRenderedLayerID; //reset every draw
};

extern DrawManager g_drawManager;

#endif // DrawManager_h__