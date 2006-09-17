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


#include "SparseGraph.h"
#include "GraphEdgeTypes.h"
#include "GraphNodeTypes.h"

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
	void DumpStatistics();
	MovingEntity * ConvertWorldNodeToOwnerEntity(int nodeID, bool bLoadWorldIfRequired);

	void Save();
	void Load();

protected:
	
	void LinkTwoNodes(TagObject *pTagSrc, TagObject *pTag);
	void LinkToConnectedWarpsOnSameMap(TagObject *pTagSrc);
	bool DoNodesConnect(World *pMap, int a, int b);

	enum
	{
		//some enums used when loading/saving chunk	
		CHUNK_NODE,
		CHUNK_LINK,
		CHUNK_DONE
	};

	void Kill();
	void LinkMap(World *pMap);
	void StripUnrequiredNodesFromPath(MacroPathInfo &m);
	void Serialize(CL_FileHelper &helper);

	//these are not that fast, use with caution.
	int ConvertMapNodeToWorldNode(World *pMap, int mapNode);
	int ConvertWorldNodeToMapNode(int nodeID);
	
	//this map's accompanying navigation graph
	NavGraph*                          m_pNavGraph;  

private:
};

extern WorldNavManager g_worldNavManager;
#endif // WorldNavManager_h__
