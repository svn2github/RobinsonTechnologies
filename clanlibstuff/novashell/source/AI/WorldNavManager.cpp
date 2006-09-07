#include "AppPrecomp.h"
#include "WorldNavManager.h"
#include "TagManager.h"
#include "WorldManager.h"
#include "GameLogic.h"
#include "TimeSlicedGraphAlgorithms.h"
#include "GraphAlgorithms.h"
#include "NavGraphManager.h"
#include "PathPlanner.h"

WorldNavManager g_worldNavManager;
WorldNavManager::WorldNavManager()
{
	m_pNavGraph = NULL;
	
}

WorldNavManager::~WorldNavManager()
{
	Kill();
}

void WorldNavManager::Kill()
{
	SAFE_DELETE(m_pNavGraph);
}

void WorldNavManager::Init()
{
	Kill();
	m_pNavGraph = new NavGraph(false);
}

void WorldNavManager::AddNode(TagObject *pTag)
{
	assert(pTag->m_graphNodeID == invalid_node_index);
	pTag->m_graphNodeID = m_pNavGraph->GetNextFreeNodeIndex();
	m_pNavGraph->AddNode(GraphNode(pTag->m_graphNodeID));
	pTag->m_pWorld->AddWarpTagHashID(pTag->m_hashID);
	
//AddNeighborLinks(pTile);
}

void WorldNavManager::RemoveNode(TagObject *pTag)
{
	assert(pTag->m_graphNodeID != invalid_node_index);
	m_pNavGraph->RemoveNode(pTag->m_graphNodeID);
	pTag->m_pWorld->RemoveWarpTagHashID(pTag->m_hashID);
	pTag->m_graphNodeID = invalid_node_index;
}

bool WorldNavManager::DoNodesConnect(World *pMap, int a, int b)
{
	//well, there must be some awesome shortcut to figure this out where you don't care about the path, but for now:
	
	assert(a != b && "They are the same node!");

		typedef Graph_SearchAStar<NavGraphManager::NavGraph, Heuristic_Euclid> AStar;

		AStar search(pMap->GetNavGraph()->GetGraph(), a, b);
		return search.IsPathValid();

}
void WorldNavManager::LinkToConnectedWarpsOnSameMap(TagObject *pTagSrc)
{
	tag_hash_list::iterator itor;
	TagObject *pTag;

	for (itor = pTagSrc->m_pWorld->GetWarpTagHashList().begin(); itor != pTagSrc->m_pWorld->GetWarpTagHashList().end(); itor++)
	{
		if (*itor == pTagSrc->m_hashID) continue; //don't want to process ourself

		pTag = g_TagManager.GetFromHash(*itor);
		if (pTag)
		{
			//do we actually touch this?
			if (DoNodesConnect(pTagSrc->m_pWorld, pTagSrc->m_graphNodeID, pTag->m_graphNodeID))
			{
				//OPTIMIZE:  Use sq version?
				LogMsg("Nodes connect!");
				int cost = Vec2DDistance(pTag->GetPos(), pTagSrc->GetPos());
				m_pNavGraph->AddEdge(NavGraph::EdgeType(pTagSrc->m_graphNodeID, pTag->m_graphNodeID, cost));
				m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_graphNodeID,pTagSrc->m_graphNodeID, cost));
			} else
			{
				LogMsg("Nodes don't connect");
			}
			

		} else
		{
			LogError("MapNav Hash doesn't exist?!");
		}
	}

}

void WorldNavManager::LinkNode(TagObject *pTag)
{
	
	TagObject *pTarget;
	const int cost = 100;

	//one way link FROM its maps hub to itself
	m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_pWorld->GetMasterNavMapID(), pTag->m_graphNodeID, cost));

	//also we need to link to its real target
	pTarget = g_TagManager.GetFromString(pTag->m_warpTarget);
	if (pTarget)
	{
		m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_graphNodeID, pTarget->m_pWorld->GetMasterNavMapID(), cost));
	} else
	{
		LogError("Warp %s Couldn't find warp target %s.", pTag->m_tagName.c_str(), pTag->m_warpTarget.c_str());
	}


	//and a two way link to any other warp nodes it connects with

	 LinkToConnectedWarpsOnSameMap(pTag);
	
}

void WorldNavManager::LinkMap(World *pMap)
{
	//we'll have each node link to one central "map" node.
	if (pMap->GetMasterNavMapID() == invalid_node_index)
	{
		pMap->SetMasterNavMapID(m_pNavGraph->GetNextFreeNodeIndex());
		m_pNavGraph->AddNode(GraphNode(pMap->GetMasterNavMapID()));
	}

	//scan through and link all nodes to its master node, and also the node it warps to
	tag_hash_list::iterator itor;
	TagObject *pTag;

	for (itor = pMap->GetWarpTagHashList().begin(); itor != pMap->GetWarpTagHashList().end(); itor++)
	{
		pTag = g_TagManager.GetFromHash(*itor);
		if (pTag)
		{
			
			LinkNode(pTag);//link to the map hub

		} else
		{
			LogError("MapNav Hash doesn't exist?!");
		}
	}
}

void WorldNavManager::LinkEverything()
{
	//go through each map and link them all

	world_info_list * pList = GetGameLogic->GetMyWorldManager()->GetWorldInfoList();
	world_info_list::iterator itor = pList->begin();

	while(itor != pList->end())
	{
		LinkMap(&(*itor)->m_world);
		LogMsg("Linking navmap for %s. (its hub is node %d)", (*itor)->m_world.GetName().c_str(), (*itor)->m_world.GetMasterNavMapID());
		
		itor++;
	}

}

MacroPathInfo WorldNavManager::FindPathToMapAndPos(MovingEntity *pEnt, World *pMap, CL_Vector2 vDest)
{
	LogMsg("Tripping through worlds");
	
	//first we need to locate a door, any door
	int doorNodeID = pEnt->GetMap()->GetNavGraph()->GetClosestSpecialNode(pEnt, pEnt->GetPos(), C_NODE_TYPE_WARP);

	MacroPathInfo m;

	if (doorNodeID == invalid_node_index)
	{
		LogMsg("Can't reach any doors.");
		return m;
	}

	LogMsg("CLosest door is %d.", doorNodeID);
	
	//first we need to locate a doable pathway

	typedef Graph_SearchDijkstras_TS<NavGraph, FindNodeIndex> DijSearch;
	Graph_SearchTimeSliced<NavGraph::EdgeType>*  pCurrentSearch =  new DijSearch(*m_pNavGraph,
		pEnt->GetMap()->GetMasterNavMapID(), pMap->GetMasterNavMapID());

	int result;
	do {result = pCurrentSearch->CycleOnce();} while(result == search_incomplete);

	if (result == target_not_found)
	{
		return m; //they can query this object to see it failed
	}

	//m.  pCurrentSearch->GetPathToTarget()

	return m;
}
