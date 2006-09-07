#include "AppPrecomp.h"
#include "NavGraphManager.h"
#include "AppUtils.h"
#include "GameLogic.h"
#include "AI/GraphAlgorithms.h"
#include "AI/AStarHeuristicPolicies.h"

NavGraphManager::NavGraphManager(World *pParent)
{
	m_pWorld = pParent;
	m_pNavGraph = NULL;
	m_pNavGraph = new NavGraph(false);
}

void NavGraphManager::Kill()
{
	SAFE_DELETE(m_pNavGraph);
}

void NavGraphManager::ExamineNodesForLinking(Tile *pA, Tile *pB)
{
	assert(pA->GetGraphNodeID() != invalid_node_index);
	assert(pB->GetGraphNodeID() != invalid_node_index);
	
	assert(pA != pB && "Why the heck would you link to yourself?");
	//if these two things don't have a wall between them, they should be linked.

	//let's just assume they don't

	CL_Vector2 a = pA->GetPos()+ (pA->GetBoundsSize()/2);
	CL_Vector2 b = pB->GetPos()+ (pB->GetBoundsSize()/2);

	float dist = Vec2DDistanceSq(a, b);

	if (!m_pWorld->GetMyWorldCache()->IsPathObstructed(a, b, 10, NULL, true))
	{
		m_pNavGraph->AddEdge(NavGraph::EdgeType(pA->GetGraphNodeID(), pB->GetGraphNodeID(), dist));
		//link the other way too
		m_pNavGraph->AddEdge(NavGraph::EdgeType(pB->GetGraphNodeID(), pA->GetGraphNodeID(), dist));
	}

}

void NavGraphManager::AddNeighborLinks(Tile *pTile)
{

	//first get all surounding nodes that we might want to connect to

	CL_Vector2 pos = pTile->GetPos() + (pTile->GetBoundsSize()/2);

	float range = GetNodeMaxLinkDistance();

	CL_Rect recArea(CL_Point(pos.x-range, pos.y-range), CL_Size(range*2, range*2));

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	m_pWorld->GetMyWorldCache()->AddTilesByRect(recArea, &tileList, m_pWorld->GetLayerManager().GetCollisionList());

	GraphNode* pN;
	int nodeID;

	tile_list::iterator itor = tileList.begin();

	for (; itor != tileList.end(); itor++)
	{
		
		if (pTile == (*itor))
		{
			//don't want to link to ourself!
			continue;
		}
		nodeID = (*itor)->GetGraphNodeID();

		if (nodeID != invalid_node_index)
		{
			pN = &m_pNavGraph->GetNode(nodeID);
			ExamineNodesForLinking(pTile, (*itor));
		}
	}

}

void NavGraphManager::AddTileNode(Tile *pTile)
{
	
	pTile->SetGraphNodeID(m_pNavGraph->GetNextFreeNodeIndex());
	m_pNavGraph->AddNode(GraphNode(pTile->GetGraphNodeID(), pTile->GetPos() + (pTile->GetBoundsSize()/2) ));

	AddNeighborLinks(pTile);

}

void NavGraphManager::RemoveTileNode(Tile *pTile)
{
	assert(pTile->GetGraphNodeID() != invalid_node_index);

	m_pNavGraph->RemoveNode(pTile->GetGraphNodeID());
}

NavGraphManager::~NavGraphManager()
{
	Kill();
}

void NavGraphManager::Render(bool bDrawNodeIDs, CL_GraphicContext *pGC)
{
	if (!m_pNavGraph) return;
	ResetFont(GetApp()->GetFont(C_FONT_GRAY));

	//draw the nodes 
	NavGraph::ConstNodeIterator NodeItr(*m_pNavGraph);
	for (const NavGraph::NodeType* pN=NodeItr.begin();
		!NodeItr.end();
		pN=NodeItr.next())
	{
		CL_Vector2 a = GetWorldCache->WorldToScreen(pN->Pos());
	
		DrawCenteredBoxWorld(pN->Pos(), 4, CL_Color::white, pGC);

		if (bDrawNodeIDs)
		{
			
			GetApp()->GetFont(C_FONT_GRAY)->set_color(CL_Color(0,0,0));
			GetApp()->GetFont(C_FONT_GRAY)->draw(a.x-2, a.y-16, CL_String::from_int(pN->Index()));
			GetApp()->GetFont(C_FONT_GRAY)->set_color(CL_Color(255,255,30));
			GetApp()->GetFont(C_FONT_GRAY)->draw(a.x-3, a.y-17, CL_String::from_int(pN->Index()));

			//gdi->TextColor(200,200,200);
			//gdi->TextAtPos((int)pN->Pos().x+5, (int)pN->Pos().y-5, ttos(pN->Index()));
		}

		NavGraph::ConstEdgeIterator EdgeItr(*m_pNavGraph, pN->Index());
		for (const NavGraph::EdgeType* pE=EdgeItr.begin();
			pE && !EdgeItr.end();
			pE=EdgeItr.next())
		{
				CL_Vector2 b = GetWorldCache->WorldToScreen( m_pNavGraph->GetNode(pE->To()).Pos() );

			pGC->draw_line(a.x, a.y, b.x, b.y, CL_Color(255,255,255,100));
		}
	}

}

