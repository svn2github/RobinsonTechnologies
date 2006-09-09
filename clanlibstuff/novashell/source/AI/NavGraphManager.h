//  ***************************************************************
//  NavGraphManager - Creation date: 08/30/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef NavGraphManager_h__
#define NavGraphManager_h__

#include "AI/SparseGraph.h"
#include "AI/GraphEdgeTypes.h"
#include "AI/GraphNodeTypes.h"

class World;
class Tile;
class MovingEntity;

class NavGraphManager
{
public:

	typedef NavGraphNodeExtra         GraphNode;
	typedef SparseGraph<GraphNode, NavGraphEdge>      NavGraph;

	NavGraphManager(World *pParent);
	virtual ~NavGraphManager();
	void Kill();
	void Render(bool bDrawNodeIDs, CL_GraphicContext *pGC);

	NavGraph&                          GetGraph()const{return *m_pNavGraph;}

	void AddTileNode(Tile *pTile);
	void RemoveTileNode(Tile *pTile);
	float GetNodeMaxLinkDistance() {return 200;}
	World * GetParent() {return m_pWorld;}
	int GetClosestSpecialNode(MovingEntity *pEnt, World *pMap, const CL_Vector2 pos, int nodeType);
	bool DoNodesConnect(int a, int b);

protected:
	

private:

	void AddNeighborLinks(Tile *pTile);
	void ExamineNodesForLinking(Tile *pA, Tile *pB); //see if two specific tiles should be linked, if so, it links them

	//this map's accompanying navigation graph
	NavGraph*                          m_pNavGraph;  
	World *m_pWorld;
	
};

#endif // NavGraphManager_h__