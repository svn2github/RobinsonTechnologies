#pragma once

#include "Polygon.h"
class MovingEntity;
class CMaterial;

struct Body
{
	Body();
	Body(const Vector& min, const Vector& max);
	Body(float worldSize);
	void SetVertArray(Vector &pos, Vector *pVertArray, int vertCount);
	float GetAngVelocity() {return m_angvelocity;}
	float GetOrientation() {return m_orientation;}
	void SetOrientation(float orientation) {m_orientation = orientation;}
	void SetParentEntity(MovingEntity *pEnt) {m_pParentEntity = pEnt;}
	MovingEntity * GetParentEntity() {return m_pParentEntity;}
	const Vector& GetPosition	() const { return m_position; }
	Vector& GetPosition		() { return m_position; }
	
	void SetDensity(float fDensity){};
	float GetDensity() { return 1;}
	void SetInertia(float inertia){};
	bool IsUnmovable() const { return (m_mass < 0.0001f); }
	void AddForce(Vector f);
	void AddForce(Vector f, Vector t);
	float GetMass() {return m_mass;}


	void render() const;
	void update(float dt);
	void SetMass(float mass);
	Vector GetLinVelocity();
	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
	Vector  m_velocity;
	float	m_invmass;
	float	m_invinertia;
	float	m_angvelocity;
	float m_mass; //not really used, but should hold it..
	MovingEntity *m_pParentEntity;
	bool m_bDeleteVerts;
	void SetMaterial(CMaterial *pNew) {m_pMaterial = pNew;}
	CMaterial * GetMaterial() {return m_pMaterial;}
	CMaterial * m_pMaterial;

};

struct CollisionReport
{
	CollisionReport();
	
	CollisionReport(Body* a, Body* b);

	void render();

	void applyReponse(float CoR);

	bool			m_collisionReported;
	Body*			m_body[2];
	Poly			m_poly[2];
	Vector			m_ncoll;
	Vector			m_mtd;
	float			m_tcoll;
	CollisionInfo	m_collisionInfo;
	ContactInfo		m_contacts;

	Vector			m_vcoll;
};

