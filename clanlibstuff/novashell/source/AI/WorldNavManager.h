//  ***************************************************************
//  WorldNavManager - Creation date: 09/07/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef WorldNavManager_h__
#define WorldNavManager_h__


#include "AI/SparseGraph.h"
#include "AI/GraphEdgeTypes.h"
#include "AI/GraphNodeTypes.h"

class World;
class TagObject;
class MovingEntity;

class MicroPathChunk
{
public:
	MicroPathChunk(int nodeID):
	m_worldNodeID(nodeID)
	{

	}
	int m_worldNodeID;
	
};
class MacroPathInfo
{
public:
	bool IsValid() {return !m_path.empty();}

	list<int> m_path;

private:

};

class WorldNavManager
{
public:

	typedef SimpleNode         GraphNode;
	typedef SparseGraph<GraphNode, NavGraphEdge>      NavGraph;

	WorldNavManager();
	virtual ~WorldNavManager();

	void Init();
	NavGraph&                          GetGraph()const{return *m_pNavGraph;}
	void AddNode(TagObject *pTag);
	void RemoveNode(TagObject *pTag);
	void LinkNode(TagObject *pTag);
	void LinkEverything();
	MacroPathInfo FindPathToMapAndPos(MovingEntity *pEnt, World *pDestMap, CL_Vector2 vDest);
	void LinkToConnectedWarpsOnSameMap(TagObject *pTagSrc);
	bool DoNodesConnect(World *pMap, int a, int b);
	MovingEntity * ConvertWorldNodeToOwnerEntity(int nodeID);
	void DumpStatistics();

protected:
	
	void Kill();
	void LinkMap(World *pMap);
	int ConvertMapNodeToWorldNode(World *pMap, int mapNode);

	//this map's accompanying navigation graph
	NavGraph*                          m_pNavGraph;  

private:
};

extern WorldNavManager g_worldNavManager;
#endif // WorldNavManager_h__