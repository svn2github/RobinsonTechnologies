#include "AppPrecomp.h"
#include "DrawManager.h"
#include "MapManager.h"

DrawManager g_drawManager;

#define C_VERY_LOW_NUMBER -999999;

DrawManager::DrawManager()
{
	m_lastRenderedLayerID = C_VERY_LOW_NUMBER;
}

DrawManager::~DrawManager()
{
}


DrawEventList * DrawManager::GetDrawEventList(int layerID)
{
	list<DrawEventList>::iterator itor = m_eventList.begin();

	while (itor != m_eventList.end())
	{
		if ( itor->m_layerID == layerID)
		{
			//this is it
			return &(*itor);
		}
		itor++;
	}

	//add it and return it
	m_eventList.push_back(DrawEventList(layerID));
	return &m_eventList.back();
}

void DrawManager::DrawLine( CL_Vector2 a, CL_Vector2 b, CL_Color col, int layerID, int drawStyle )
{
	DrawEvent *pEvent = GetDrawEventList(layerID)->GetNewEvent();

	pEvent->m_type = DrawEvent::TYPE_DRAW_LINE;
	
	if (drawStyle == C_DRAW_WORLD_COORDS)
	{
		pEvent->m_a = g_pMapManager->GetActiveMap()->GetMyMapCache()->WorldToScreen(a);
		pEvent->m_b = g_pMapManager->GetActiveMap()->GetMyMapCache()->WorldToScreen(b);

	} else
	{
		pEvent->m_a = a;
		pEvent->m_b = b;

	}
	
	pEvent->m_color = col;
	pEvent->m_drawStyle = drawStyle;
}

void DrawManager::Update(float delta)
{
	m_eventList.clear();
	m_lastRenderedLayerID = C_VERY_LOW_NUMBER;
}

void DrawManager::RenderUpToLayerID(int layerID, CL_GraphicContext *pGC)
{
	if (layerID < m_lastRenderedLayerID)
	{
		//reset it
		m_lastRenderedLayerID = C_VERY_LOW_NUMBER; //some very low number
	}
	
	list<DrawEventList>::iterator itor = m_eventList.begin();

	for (; itor != m_eventList.end(); )
	{
		if (itor->m_layerID > m_lastRenderedLayerID && itor->m_layerID <= layerID)
		{
			itor->Render(pGC);
			m_lastRenderedLayerID = itor->m_layerID;
			itor++;
		} else
		{
			//all done for now I guess
			return;
		}
	}
}

void DrawEventList::Render( CL_GraphicContext *pGC )
{
	list <DrawEvent>::iterator itor = m_events.begin();
	while (itor != m_events.end())
	{
		itor->Render(pGC);
		itor++;
	}
}

DrawEvent * DrawEventList::GetNewEvent()
{
	m_events.push_back(DrawEvent());
	return &m_events.back();
}

void DrawEvent::Render( CL_GraphicContext *pGC )
{
	switch (m_type)
	{
	case TYPE_DRAW_LINE:
		CL_Display::draw_line(m_a.x, m_a.y,  m_b.x, m_b.y, m_color);
		break;

	default:
		LogError("DrawManager error with type %d", m_type);
	}
}