#include "AppPrecomp.h"
#include "Polygon.h"

// taken from 
// http://www.physicsforums.com/showthread.php?s=e251fddad79b926d003e2d4154799c14&t=25293&page=2&pp=15
float Poly::calculateMass(float density)
{
	if (m_count < 2)
		return 5.0f * density;

	float mass = 0.0f;
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
		mass +=  (float) fabs(m_vertices[i] ^ m_vertices[j]);
	
	mass *= density * 0.5f;
	return mass;
}

// taken from 
// http://www.physicsforums.com/showthread.php?s=e251fddad79b926d003e2d4154799c14&t=25293&page=2&pp=15
float Poly::calculateInertia()
{
	if (m_count == 1) 
		return 0.0f;

	float denom = 0.0f;
	float numer = 0.0f;
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
	{
		float a = (float) fabs(m_vertices[i] ^ m_vertices[j]);
		float b = (m_vertices[j]*m_vertices[j] + m_vertices[j]*m_vertices[i] + m_vertices[i]*m_vertices[i]);
		denom += (a * b);
		numer += a;
	}
	float inertia = (denom / numer) * (1.0f / 6.0f);;
	return inertia;
}


Poly::Poly()
{
	m_count = 0;
	m_vertices = NULL;
}


Poly::Poly(const Vector& min, const Vector& max)
{
	m_vertices[0] = Vector(min.x, min.y);
	m_vertices[1] = Vector(min.x, max.y);
	m_vertices[2] = Vector(max.x, max.y);
	m_vertices[3] = Vector(max.x, min.y);
	m_count = 4;
}


void Poly::SetVertInfo(Vector *pVerts, int count)
{
	m_vertices = pVerts;
	m_count = count;
}

Poly::Poly(int count, float radius)
{
	m_count = count;
	
	for(int i = 0; i < m_count; i ++)
	{
		float a = 2.0f * pi() * (i / (float) m_count);

		m_vertices[i] = Vector(cos(a), sin(a)) * radius;
	}
}

void Poly::transform(const Vector& position, float rotation)
{
	for(int i = 0; i < m_count; i ++)
		m_vertices[i].transform(position, rotation);
}

void Poly::translate(const Vector& delta)
{
	for(int i = 0; i < m_count; i ++)
		m_vertices[i] += delta;
}

void Poly::render(bool solid) const
{
	if(solid)
	{
		glBegin(GL_TRIANGLE_FAN);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}
	for(int i = 0; i < m_count; i ++)
	{
		glVertex2f(m_vertices[i].x, m_vertices[i].y);
	}
	glVertex2f(m_vertices[0].x, m_vertices[0].y);
	glEnd();
}

CollisionInfo Poly::collide(const Poly& poly, const Vector& delta) const
{
	CollisionInfo info;
	// reset info to some weird values
	info.m_overlapped = true;		 // we'll be regressing tests from there
	info.m_collided = true;
	info.m_mtdLengthSquared = -1.0f; // flags mtd as not being calculated yet
	info.m_tenter = 1.0f;			 // flags swept test as not being calculated yet
	info.m_tleave = 0.0f;			 // <--- ....

		
	// test separation axes of current polygon
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
	{
		Vector v0 = m_vertices[j];
		Vector v1 = m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge
		
		if(separatedByAxis(axis, poly, delta, info))
			return CollisionInfo();
	}

	// test separation axes of other polygon
	for(int j = poly.m_count-1, i = 0; i < poly.m_count; j = i, i ++)
	{
		Vector v0 = poly.m_vertices[j];
		Vector v1 = poly.m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge
		if(separatedByAxis(axis, poly, delta, info))
			return CollisionInfo();
	}

	assert(!(info.m_overlapped) || (info.m_mtdLengthSquared >= 0.0f));
	assert(!(info.m_collided)   || (info.m_tenter <= info.m_tleave));

	// sanity checks
	info.m_overlapped &= (info.m_mtdLengthSquared >= 0.0f);
	info.m_collided   &= (info.m_tenter <= info.m_tleave);	

	// normalise normals
	info.m_Nenter.normalise();
	info.m_Nleave.normalise();

	return info;
}

void Poly::calculateInterval(const Vector& axis, float& min, float& max) const
{
	min = max = (m_vertices[0] * axis);

	for(int i = 1; i < m_count; i ++)
	{
		float d = (m_vertices[i] * axis);
		if (d < min) 
			min = d; 
		else if (d > max) 
			max = d;
	}
}

bool Poly::separatedByAxis(const Vector& axis, const Poly& poly, const Vector& delta, CollisionInfo& info) const
{
	float mina, maxa;
	float minb, maxb;

	// calculate both polygon intervals along the axis we are testing
	calculateInterval(axis, mina, maxa);
	poly.calculateInterval(axis, minb, maxb);

	// calculate the two possible overlap ranges.
	// either we overlap on the left or right of the polygon.
	float d0 = (maxb - mina); // 'left' side
	float d1 = (minb - maxa); // 'right' side
	float v  = (axis * delta); // project delta on axis for swept tests

	bool sep_overlap = separatedByAxis_overlap(axis, d0, d1, info);
	bool sep_swept   = separatedByAxis_swept  (axis, d0, d1, v, info);
	
	// both tests didnt find any collision
	// return separated
	return (sep_overlap && sep_swept);
}

bool Poly::separatedByAxis_overlap(const Vector& axis, float d0, float d1, CollisionInfo& info) const
{
	if(!info.m_overlapped)
		return false;

	// intervals do not overlap. 
	// so no overlpa possible.
	if(d0 < 0.0f || d1 > 0.0f)
	{
		info.m_overlapped = false;
		return true;
	}
	
	// find out if we overlap on the 'right' or 'left' of the polygon.
	float overlap = (d0 < -d1)? d0 : d1;

	// the axis length squared
	float axis_length_squared = (axis * axis);

	assert(axis_length_squared > 0.00001f);

	// the mtd vector for that axis
	Vector sep = axis * (overlap / axis_length_squared);

	// the mtd vector length squared.
	float sep_length_squared = (sep * sep);

	// if that vector is smaller than our computed MTD (or the mtd hasn't been computed yet)
	// use that vector as our current mtd.
	if(sep_length_squared < info.m_mtdLengthSquared || (info.m_mtdLengthSquared < 0.0f))
	{
		info.m_mtdLengthSquared = sep_length_squared;
		info.m_mtd				= sep;
	}
	return false;
}

bool Poly::separatedByAxis_swept(const Vector& axis, float d0, float d1, float v, CollisionInfo& info) const
{
	if(!info.m_collided)
		return false;

	// projection too small. ignore test
	if(fabs(v) < 0.0000001f) return true;

	Vector N0 = axis;
	Vector N1 = -axis;
	float t0 = d0 / v;   // estimated time of collision to the 'left' side
	float t1 = d1 / v;  // estimated time of collision to the 'right' side

	// sort values on axis
	// so we have a valid swept interval
	if(t0 > t1) 
	{
		swapf(t0, t1);
		N0.swap(N1);
	}
	
	// swept interval outside [0, 1] boundaries. 
	// polygons are too far apart
	if(t0 > 1.0f || t1 < 0.0f)
	{
		info.m_collided = false;
		return true;
	}

	// the swept interval of the collison result hasn't been
	// performed yet.
	if(info.m_tenter > info.m_tleave)
	{
		info.m_tenter = t0;
		info.m_tleave = t1;
		info.m_Nenter = N0;
		info.m_Nleave = N1;
		// not separated
		return false;
	}
	// else, make sure our current interval is in 
	// range [info.m_tenter, info.m_tleave];
	else
	{
		// separated.
		if(t0 > info.m_tleave || t1 < info.m_tenter)
		{
			info.m_collided = false;
			return true;
		}

		// reduce the collison interval
		// to minima
		if (t0 > info.m_tenter)
		{
			info.m_tenter = t0;
			info.m_Nenter = N0;
		}
		if (t1 < info.m_tleave)
		{
			info.m_tleave = t1;
			info.m_Nleave = N1;
		}			
		// not separated
		return false;
	}
}

SupportPoints Poly::getSupports(const Vector& axis)
{
	SupportPoints supports;

	float min = -1.0f;
	const float threshold = 1.0E-1f;
	int count = 0;

	int num = m_count;
	for(int i = 0; i < num; i ++)
	{
		float t = (axis * m_vertices[i]);
		if(t < min || i == 0)
			min = t;
	}

	for(int i = 0; i < num; i ++)
	{
		float t = (axis * m_vertices[i]);
		
		if(t < min+threshold)
		{
			supports.m_support[supports.m_count++] = m_vertices[i];
			if (supports.m_count == 2) break;
		}
	}
	return supports;
}