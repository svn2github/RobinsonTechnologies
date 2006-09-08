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
	m_pNavGraph->AddNode(GraphNode(pTag->m_graphNodeID, pTag->m_hashID));
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

	if (!pTagSrc->m_pWorld->IsInitted())
	{
		GetWorldManager->LoadWorld(pTagSrc->m_pWorld->GetDirPath(), false);
		pTagSrc->m_pWorld->PreloadMap();
	}
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
	//m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_pWorld->GetMasterNavMapID(), pTag->m_graphNodeID, cost));

	//also we need to link to its real target
	pTarget = g_TagManager.GetFromString(pTag->m_warpTarget);
	if (pTarget)
	{
		if (pTarget->m_graphNodeID == invalid_node_index)
		{
			LogError("Warp %s cannot warp to %s because it's not also a warp.  (it needs GetTagManager:RegisterAsWarp in its script)",
				pTag->m_tagName.c_str(), pTarget->m_tagName.c_str());
		} else
		{
			m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_graphNodeID, pTarget->m_graphNodeID, cost));

			//this may be a bad idea, but assume everything is two way for now?
//			m_pNavGraph->AddEdge(NavGraph::EdgeType(pTarget->m_graphNodeID, pTag->m_graphNodeID,cost));
		}
	} else
	{
		LogError("Warp %s Couldn't find warp target %s.", pTag->m_tagName.c_str(), pTag->m_warpTarget.c_str());
		//return;
	}


	//and a two way link to any other warp nodes it connects with

	 LinkToConnectedWarpsOnSameMap(pTag);
	
}

void WorldNavManager::LinkMap(World *pMap)
{
	
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
		//LogMsg("Linking navmap for %s. (its hub is node %d)", (*itor)->m_world.GetName().c_str(), (*itor)->m_world.GetMasterNavMapID());
		itor++;
	}

}

//certain kinds of map nodes like warps have extra data so we can figure out what their node ID is on the map nav graph (the
//map nav graph is the thing that helps us figure out where doors lead to quickly)

int WorldNavManager::ConvertMapNodeToWorldNode(World *pMap, int mapNode)
{

	NavGraphNodeExtra &n = pMap->GetNavGraph()->GetGraph().GetNode( mapNode);
	MovingEntity *pNodeOwnerEnt = (MovingEntity*)EntityMgr->GetEntityFromID(n.GetEntID());
	assert(pNodeOwnerEnt && "didn't you know this should always exist?  Maybe the original node wasn't a named entity");
	TagObject *pTag = GetTagManager->GetFromHash(pNodeOwnerEnt->GetNameHash());
	if (!pTag)
	{
			LogError("ConvertMapNodeToWorldNode: %s missing tagcache data.  It should be a named entity", pNodeOwnerEnt->GetName().c_str());
		return invalid_node_index;
	}

	assert(pTag->m_graphNodeID != invalid_node_index && "Huh?");
	return pTag->m_graphNodeID;

};

MovingEntity * WorldNavManager::ConvertWorldNodeToOwnerEntity(int nodeID)
{
	GraphNode &n =m_pNavGraph->GetNode(nodeID);

	TagObject *pTag = g_TagManager.GetFromHash(n.GetTagHashID());
	if (!pTag)
	{
		
		return NULL;
	} 

	return (MovingEntity*)EntityMgr->GetEntityFromID(pTag->m_entID);
}

void WorldNavManager::DumpStatistics()
{
	LogMsg("** World Nav Graph Info **");

	if (m_pNavGraph->isEmpty())
	{
		LogMsg("It's empty right now.")	;
		return;
	}

	NavGraph::ConstNodeIterator NodeItr(*m_pNavGraph);
	for (const NavGraph::NodeType* pN=NodeItr.begin();
		!NodeItr.end();
		pN=NodeItr.next())
	{

		TagObject *pTag = g_TagManager.GetFromHash(pN->GetTagHashID());

		string extra = "Bad Tag Hash";
		if (pTag)
		{
			extra =  pTag->m_tagName + " (" + CL_String::from_int(pTag->m_entID)+ ") ";
			if (pTag->m_graphNodeID != pN->Index())
			{
				extra += "ID Mismatch!";
			} else
			{
			
				MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(pTag->m_entID);
				if (pEnt)
				{
					if (pEnt->GetNameHash() == pTag->m_hashID)
					{
						extra += "Match OK";

					} else
					{
						extra += "Ent " + CL_String::from_int(pEnt->ID())+"'s hash wrong: " + CL_String::from_int(pEnt->GetNameHash());
					}
				} else
				{
					extra = "Ent ID " + CL_String::from_int(pTag->m_entID)+ " doesn't exist?";
				}
			}
		}
		LogMsg("Node %d - Hash: %u - %s", pN->Index(), pN->GetTagHashID(), extra.c_str());

		string temp = "    Connections: ";

		NavGraph::ConstEdgeIterator EdgeItr(*m_pNavGraph, pN->Index());
		for (const NavGraph::EdgeType* pE=EdgeItr.begin();
			pE && !EdgeItr.end();
			pE=EdgeItr.next())
		{
			temp += CL_String::from_int(m_pNavGraph->GetNode(pE->To()).Index()) + "  ";
		}
		LogMsg(temp.c_str());
	}

}

MacroPathInfo WorldNavManager::FindPathToMapAndPos(MovingEntity *pEnt, World *pDestMap, CL_Vector2 vDest)
{
	//first we need to locate a door, any door, it doesn't have to  be the closest, it just has to be reachable
	int startWarpID = pEnt->GetMap()->GetNavGraph()->GetClosestSpecialNode(pEnt, pEnt->GetMap(), pEnt->GetPos(), C_NODE_TYPE_WARP);

	MacroPathInfo m;

	if (startWarpID == invalid_node_index)
	{ 
		LogMsg("Can't reach door on starting map");
		return m;
	}


	//now we need to find a door on the goal map that definitely can connect to

	int destWarpID = pDestMap->GetNavGraph()->GetClosestSpecialNode(pEnt,pDestMap,  vDest, C_NODE_TYPE_WARP);

	if (destWarpID == invalid_node_index)
	{
		LogMsg("Can't reach warp on target map.");
		return m;
	}


	//the unique thing about nodes that are warps is we can get their worldnav node id from their mapnav id.  The worldnav is
	//a sparse graph that connects all the doors in the world
	
	int worldNavNodeStart = ConvertMapNodeToWorldNode(pEnt->GetMap(), startWarpID);

	if (worldNavNodeStart == invalid_node_index)
	{
		LogError("Unable to locate starting warp %d's path node.", startWarpID);
		return m;
	}
	int worldNavNodeEnd = ConvertMapNodeToWorldNode(pDestMap, destWarpID);

	if (worldNavNodeEnd == invalid_node_index)
	{
		LogError("Unable to locate destination warp %d's path node.", worldNavNodeEnd);
		return m;
	}

	LogMsg("Start worldNavID: %d  End: %d",worldNavNodeStart, worldNavNodeEnd);
	//first we need to locate a doable pathway

	typedef Graph_SearchDijkstras_TS<NavGraph, FindNodeIndex> DijSearch;
	Graph_SearchTimeSliced<NavGraph::EdgeType>*  pCurrentSearch =  new DijSearch(*m_pNavGraph,
		worldNavNodeStart, worldNavNodeEnd);

	int result;
	do {result = pCurrentSearch->CycleOnce();} while(result == search_incomplete);

	if (result == target_not_found)
	{
		return m; //they can query this object to see it failed
	}

	//we've located a way through that we know will work.

	m.m_path = pCurrentSearch->GetPathToTarget();

	//m.  pCurrentSearch->GetPathToTarget()

	return m;
}
