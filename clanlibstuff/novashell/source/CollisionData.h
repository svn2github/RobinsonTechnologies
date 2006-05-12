
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 23:1:2006   12:34
*/

 
#ifndef CollisionData_HEADER_INCLUDED // include guard
#define CollisionData_HEADER_INCLUDED  // include guard

using namespace std;

#include "PointList.h"
#include "Tile.h"


typedef std::list<PointList> line_list;

class CollisionData
{
public:

	CollisionData();
	CollisionData(const CL_Rect &rec) {m_rect = rec;};
    ~CollisionData();
	void Clear();
	CL_Rect & GetRect() {return m_rect;}
	void SetRect(const CL_Rect &rec) {m_rect = rec;}
	line_list * GetLineList() {return &m_lineList;}
	void Serialize(CL_FileHelper &helper);
	bool HasData();
	void RecalculateOffsets();
	void RemoveOffsets();
	void Load(const string &fileName);
	void SetDataChanged(bool bNew) {m_dataChanged = bNew;}

protected:

	void SaveIfNeeded();

private:
	CL_Rect m_rect;
	line_list m_lineList;
	string m_fileName; //if not empty, it means we were loaded from disk and should also
	//save if changed
	bool m_dataChanged;
};

void CreateCollisionDataWithTileProperties(Tile *pTile, CollisionData &colOut);


#endif                  // include guard
