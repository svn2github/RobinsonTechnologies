#include "AppPrecomp.h"
#include "PointList.h"
#include "physics/Body.h"
#include "MaterialManager.h"
#include "AppUtils.h"


//for SimpleHull2D function:
//Copyright 2002, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// Assume that a class is already given for the object:
//    Point with coordinates {float x, y;}
//===================================================================


// isLeft(): test if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
inline float
isLeft( CL_Vector2 P0, CL_Vector2 P1, CL_Vector2 P2 )
{
	return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
}


// simpleHull_2D():
//    Input:  V[] = polyline array of 2D vertex points 
//            n   = the number of points in V[]
//    Output: H[] = output convex hull array of vertices (max is n)
//    Return: h   = the number of points in H[]
int simpleHull_2D( CL_Vector2* V, int n, CL_Vector2* H )
{
	// initialize a deque D[] from bottom to top so that the
	// 1st three vertices of V[] are a counterclockwise triangle
	CL_Vector2* D = new CL_Vector2[2*n+1];
	int bot = n-2, top = bot+3;   // initial bottom and top deque indices
	D[bot] = D[top] = V[2];       // 3rd vertex is at both bot and top
	if (isLeft(V[0], V[1], V[2]) > 0) {
		D[bot+1] = V[0];
		D[bot+2] = V[1];          // ccw vertices are: 2,0,1,2
	}
	else {
		D[bot+1] = V[1];
		D[bot+2] = V[0];          // ccw vertices are: 2,1,0,2
	}

	// compute the hull on the deque D[]
	for (int i=3; i < n; i++) {   // process the rest of vertices
		// test if next vertex is inside the deque hull
		if ((isLeft(D[bot], D[bot+1], V[i]) > 0) &&
			(isLeft(D[top-1], D[top], V[i]) > 0) )
			continue;         // skip an interior vertex

		// incrementally add an exterior vertex to the deque hull
		// get the rightmost tangent at the deque bot
		while (isLeft(D[bot], D[bot+1], V[i]) <= 0)
			++bot;                // remove bot of deque
		D[--bot] = V[i];          // insert V[i] at bot of deque

		// get the leftmost tangent at the deque top
		while (isLeft(D[top-1], D[top], V[i]) <= 0)
			--top;                // pop top of deque
		D[++top] = V[i];          // push V[i] onto top of deque
	}

	// transcribe deque D[] to the output hull array H[]
	int h;        // hull vertex counter
	for (h=0; h <= (top-bot); h++)
		H[h] = D[bot + h];

	delete D;
	return h-1;
}

void PointList::RemoveDuplicateVerts()
{
	point_list::iterator itor = m_points.begin();

	while (itor != m_points.end())
	{

		point_list::iterator itorFound = std::find(itor+1, m_points.end(), *itor);
	
		if (itorFound != m_points.end())
		{
			//uh oh, it has a dupe.  Erase this one
			itor = m_points.erase(itor);
			continue;
		}

		itor++;
	}
}

void PointList::ApplyScale(const CL_Vector2 &vScale)
{
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i].x *= vScale.x;
		m_points[i].y *= vScale.y;
	}

	m_bNeedsToRecalculateRect = true;

}

bool PointList::ComputeConvexHull()
{
	m_bNeedsToRecalculateRect = true;

	if (m_points.size() < 2)
	{
		LogMsg("Not enough points, aborting computing convex hull.");
		return false;
	}
	point_list ptsVec;
	ptsVec.resize(m_points.size()+1, CL_Vector2(0,0));
	
	simpleHull_2D(&m_points[0], m_points.size(), &ptsVec[0]);
	
	//copy the new points over
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] = ptsVec[i];
	}

	RemoveDuplicateVerts();
	return true;
}


CBody g_Body; //a global for now, until I put in the real definitions

PointList::PointList()
{
	m_bNeedsToRecalculateRect = true;
	m_type = C_POINT_LIST_HARD;	
    m_vecOffset = CL_Vector2(0,0);
}

PointList::~PointList()
{
}

const CL_Rectf & PointList::GetRect()
{
	if (m_bNeedsToRecalculateRect)
	{
		BuildBoundingRect();
	}

	return m_boundingRect;
}

void PointList::SetType(int myType)
{
	m_type = myType;
	if (m_type >= g_materialManager.GetCount())
	{
		LogMsg("Warning: Material index %d not initted, changing it to 0", m_type);
		m_type = 0;
	}
}

bool PointList::BuildBoundingRect()
{
	if (!HasData()) 
	{
		m_boundingRect.left = m_boundingRect.right = m_boundingRect.top = m_boundingRect.bottom = 0;	
		return false;
	}

	m_bNeedsToRecalculateRect = false;

	m_boundingRect.left = FLT_MAX;
	m_boundingRect.top = FLT_MAX;
	m_boundingRect.bottom = -FLT_MAX;
	m_boundingRect.right = -FLT_MAX;

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_boundingRect.left = min(m_boundingRect.left, m_points[i].x + m_vecOffset.x);
		m_boundingRect.right = max(m_boundingRect.right, m_points[i].x + m_vecOffset.x);

		m_boundingRect.top = min(m_boundingRect.top, m_points[i].y + m_vecOffset.y);
		m_boundingRect.bottom = max(m_boundingRect.bottom, m_points[i].y + m_vecOffset.y);
	}
	return true;
}

void PointList::RemoveOffsets()
{
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] += m_vecOffset;
	}
	m_vecOffset = CL_Vector2(0,0);
	
	m_bNeedsToRecalculateRect = true;
}

void PointList::CalculateOffsets()
{
	RemoveOffsets();
	if (m_points.size() == 0) return;

	m_bNeedsToRecalculateRect = true; //force recalulation to happen now	

	m_vecOffset = CL_Vector2(GetRect().get_width()/2, GetRect().get_height()/2);

	for (unsigned int i=0; i < m_points.size(); i++)
	{
	m_points[i] -= m_vecOffset;
	}

	/*
	
	m_vecOffset = CL_Vector2(200000,20000);
	//run through the array and find the center point
	CL_Vector2 vTotal = CL_Vector2(0,0);
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		vTotal += m_points[i];
	}

	m_vecOffset = vTotal / m_points.size(); //find middle point

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] -= m_vecOffset;
	}

	m_bNeedsToRecalculateRect = true;
	*/
}

void PointList::PrintPoints()
{
	LogMsg("Offset is %.2f, %.2f", m_vecOffset.x, m_vecOffset.y);

	string s;

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		s = "Vert #" + CL_String::from_int(i) + ": "+ PrintVector(m_points[i]);
		LogMsg(s.c_str());
	}

}

CBody & PointList::GetAsBody(const CL_Vector2 &vPos, CBody *pCustomBody)
{
	// assert(m_points.size() > 0);
 	 if (pCustomBody)
	 {
		 pCustomBody->SetVertArray(pCustomBody->GetPosition(), (Vector*)&m_points[0], m_points.size());
		 pCustomBody->SetMaterial(g_materialManager.GetMaterial(m_type));
		 return *pCustomBody;
	 }

	 g_Body.SetMaterial(g_materialManager.GetMaterial(m_type));
	 g_Body.SetVertArray(*(Vector*)& (vPos+m_vecOffset), (Vector*)&m_points[0], m_points.size());
	 return g_Body;
}
