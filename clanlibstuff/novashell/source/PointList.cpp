#include "AppPrecomp.h"
#include "PointList.h"
#include "physics/Body.h"
#include "MaterialManager.h"
#include "AppUtils.h"
#include "PhysicsManager.h"

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

	RemoveOffsets();
	
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i].x *= vScale.x;
		m_points[i].y *= vScale.y;
	}

	CalculateOffsets();

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

	if (m_points.size() > 3)
	{
		point_list ptsVec;
		ptsVec.resize(m_points.size()+1, CL_Vector2(0,0));
		simpleHull_2D(&m_points[0], m_points.size(), &ptsVec[0]);

		//copy the new points over
		for (unsigned int i=0; i < m_points.size(); i++)
		{
			m_points[i] = ptsVec[i];
		}
	}

	RemoveDuplicateVerts();
	return true;
}


Body g_Body; //a global for now, until I put in the real definitions

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
	assert(m_vecOffset == CL_Vector2(0,0));
return;
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] += m_vecOffset;
	}

	m_vecOffset = CL_Vector2(0,0);
	m_bNeedsToRecalculateRect = true;
}

void PointList::CalculateOffsets()
{
		return;
	if (m_points.size() == 0) return;

	m_bNeedsToRecalculateRect = true; //force recalulation to happen now	

	CL_Vector2 upperLeftBounds = CL_Vector2(100000,100000);

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		upperLeftBounds.x = min(upperLeftBounds.x, m_points[i].x);
		upperLeftBounds.y = min(upperLeftBounds.y, m_points[i].y);
	}

	m_vecOffset = upperLeftBounds + CL_Vector2(GetRect().get_width()/2, GetRect().get_height()/2);

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] -= m_vecOffset;
	}
	
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

/*
Body & PointList::GetAsBody(const CL_Vector2 &vPos, Body *pCustomBody)
{
	// assert(m_points.size() > 0);
 	 if (pCustomBody)
	 {
		 pCustomBody->SetVertArray(pCustomBody->m_position, (Vector*)&m_points[0], m_points.size());
		 pCustomBody->SetMaterial(g_materialManager.GetMaterial(m_type));
		 return *pCustomBody;
	 }

	 g_Body.SetMaterial(g_materialManager.GetMaterial(m_type));
	 g_Body.SetVertArray(*(Vector*)& (vPos+m_vecOffset), (Vector*)&m_points[0], m_points.size());
	 return g_Body;
}
*/

//stolen from jyk's snippet from GDNet: http://www.gamedev.net/community/forums/topic.asp?topic_id=304578

bool IntersectCircleSegment(const CL_Vector2& c,        // center
							float r,                            // radius
							const CL_Vector2& p1,     // segment start
							const CL_Vector2& p2)     // segment end
{
	CL_Vector2 dir = p2 - p1;
	CL_Vector2 diff = c - p1;
	float t = diff.dot(dir) / dir.dot(dir);
	if (t < 0.0f)
		t = 0.0f;
	if (t > 1.0f)
		t = 1.0f;
	CL_Vector2 closest = p1 + (dir*t);
	CL_Vector2 d = c - closest;
	float distsqr = d.dot(d);
	return distsqr <= r * r;
}

bool PointList::GetCircleIntersection(const CL_Vector2 &c, float radius)
{
	for (unsigned int i=0; i < GetPointList()->size();)
	{
		int endLineIndex;

		if (i == GetPointList()->size()-1)
		{
			endLineIndex=0;
		} else
		{
			endLineIndex = i+1;
		}

		if (IntersectCircleSegment(c, radius, GetPointList()->at(i), GetPointList()->at(endLineIndex)))
		{
			return true; 
		}
		
		i++;
	}

	return false;
}

void PointList::GetAsPolygonDef(b2PolygonDef *shapeDef )
{
	for (unsigned int i=0; i < GetPointList()->size(); i++)
	{
		shapeDef->vertices[i].x = m_points[i].x / C_PHYSICS_PIXEL_SIZE;
		shapeDef->vertices[i].y = m_points[i].y / C_PHYSICS_PIXEL_SIZE;
	}

	shapeDef->vertexCount = GetPointList()->size();

	shapeDef->restitution = g_materialManager.GetMaterial(m_type)->GetRestitution();
	shapeDef->friction = g_materialManager.GetMaterial(m_type)->GetFriction();
}

bool PointList::GetLineIntersection(const CL_Vector2 &a, const CL_Vector2 &b)
{
	float lineA[4], lineB[4];
	lineA[0] = a.x;
	lineA[1] = a.y;

	lineA[2] = b.x;
	lineA[3] = b.y;

		for (unsigned int i=0; i < GetPointList()->size();)
			{
	
					lineB[0] = GetPointList()->at(i).x;
					lineB[1] = GetPointList()->at(i).y;

					int endLineIndex;

					if (i == GetPointList()->size()-1)
					{
						endLineIndex=0;
					} else
					{
						endLineIndex = i+1;
					}

					lineB[2] = GetPointList()->at(endLineIndex).x;
					lineB[3] = GetPointList()->at(endLineIndex).y;

				
				//LogMsg("Line B is %.2f, %.2f - %.2f, %.2f", lineB[0],lineB[1],lineB[2],lineB[3] );
				if (CL_LineMath::intersects(lineA, lineB))
				{
					
					//LogMsg("Got intersection");
					//ptCol = CL_LineMath::get_intersection(lineA, lineB);
					return true;
				}
				i++;
			}
		

	return false;
}
