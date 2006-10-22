
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 17:1:2006   14:50
*/


#ifndef PointList_HEADER_INCLUDED // include guard
#define PointList_HEADER_INCLUDED  // include guard

enum
{
	C_POINT_LIST_HARD = 0
};

typedef std::vector<CL_Vector2> point_list;

class CBody;

class PointList
{
public:

    PointList();
    ~PointList();

	int GetType() {return m_type;}
	void SetType(int myType);
	point_list * GetPointList() {return &m_points;}
	void CalculateOffsets();
	CL_Vector2 & GetOffset() {return m_vecOffset;}
	void SetOffset(CL_Vector2 &v) {m_vecOffset = v;}
	void RemoveOffsets();
	void ApplyScale(const CL_Vector2 &vScale);
	void PrintPoints(); //debug thing

	CBody & GetAsBody(const CL_Vector2 &vPos, CBody *pCustomBody = NULL);
	const CL_Rectf & GetRect();
	bool HasData() {return m_points.size() > 0;}
	bool ComputeConvexHull(); //any illegal verts are moved to legal positions, which creates dupes...
	void RemoveDuplicateVerts(); //cut out any vert that has another vert sitting exactly on it
	bool GetLineIntersection(const CL_Vector2 &a, const CL_Vector2 &b);

protected:

private:
	bool BuildBoundingRect(); //returns false if no data is built
	bool m_bNeedsToRecalculateRect;
	CL_Rectf m_boundingRect; //cache this info out for speed
	int m_type;
	CL_Vector2 m_vecOffset; //offset from our Tile's upper left to the upper left vert
	point_list m_points; //needs at least 2 to make a line
};

#endif                  // include guard
