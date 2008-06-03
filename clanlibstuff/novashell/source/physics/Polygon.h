#pragma once

#include "Vector.h"
#include "Collision.h"

// basic polygon structure
struct Poly
{
public:
	Poly();
	Poly(const Vector& min, const Vector& max);
	Poly(int count, float radius);

	float calculateMass(float density);
	float calculateInertia();

	void			transform	(const Vector& position, float rotation);
	void			translate	(const Vector& delta);
	CollisionInfo	collide		(const Poly& poly, const Vector& delta) const;
	SupportPoints	getSupports	(const Vector& axis);
	void			render		(bool solid=true) const;
	void SetVertInfo(Vector *pVerts, int count);
public:
	//enum { MAX_VERTICES = 32 };
	Vector	* m_vertices;
	int		m_count;

	// collision functions
private:
	void calculateInterval(const Vector& axis, float& min, float& max) const;
	bool separatedByAxis		(const Vector& axis, const Poly& poly, const Vector& delta, CollisionInfo& info) const;
	bool separatedByAxis_swept	(const Vector& axis, float d0, float d1, float v, CollisionInfo& info) const;
	bool separatedByAxis_overlap(const Vector& axis, float d0, float d1, CollisionInfo& info) const;

};

