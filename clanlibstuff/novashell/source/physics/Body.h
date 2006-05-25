#ifndef OLI_BODY_H
#define OLI_BODY_H


/*
#include "Body.h"
*/

/*
------------------------------------------------------------------
File: Body.h
Started: 07/02/2004 20:55:15
  
$Header: $
$Revision: $
$Locker: $
$Date: $
  
Author: Olivier renault
------------------------------------------------------------------
Module: 
Description: 
------------------------------------------------------------------
$History: $
------------------------------------------------------------------
*/

#include "vector.h"
#include "Matrix.h"
#include "Polygon.h"
#include <ClanLib/signals.h>

class CMaterial;
class MovingEntity;

class CBody
{
public:
	CBody();
	CBody(const Vector& xPosition, float fDensity, float width, float height);
	CBody(const Vector& Min, const Vector& Max, float fDensity);
	void Initialise(const Vector& xPosition, float fDensity, Vector* axVertices, int iNumVertices, bool bDelete);
	void Shutdown();
	
	~CBody();

	void AddForce(const Vector& F);
	
	void AddForce(const Vector& F, const Vector& P);
	
	bool Collide(CBody& Body, float dt);
	
	bool IntersectSegment(const Vector& Start, const Vector& End, float fDist, Vector& N, float& t) const;

	void Update(float dt);

	void Render() const;

	void SetAngVelocity(float fNew) {m_fAngVelocity = fNew;}

	void SetOrientation(float fAngle);
	void SetDensity(float fDensity);
	float GetDensity() {return m_fDensity;}
	void SetParentEntity(MovingEntity *pEnt) {m_pParentEntity = pEnt;}
	MovingEntity * GetParentEntity() {return m_pParentEntity;}

	bool IsUnmovable() const { return (m_fMass < 0.0001f); }
	Vector& GetPosition		() { return m_xPosition; }
	Vector& GetLinVelocity	() { return m_xVelocity; }
	float&  GetAngVelocity  () { return m_fAngVelocity; }
	float&  GetMass			() { return m_fMass; }
	float&  GetInvMass		() { return m_fInvMass; }
	float&  GetInertia		() { return m_fInertia; }
	float&  GetInvInertia	() { return m_fInvInertia; }
	float GetOrientation() {return m_fOrientation;}	
	void SetLinVelocity(Vector vel)	{m_xVelocity = vel;}
	const Vector& GetPosition	() const { return m_xPosition; }

	/*
	float GetAngVelocity		() const { return m_fAngVelocity; }
	float GetMass				() const { return m_fMass; }
	float GetInvMass			() const { return m_fInvMass; }
	float GetInvInertia			() const { return m_fInvInertia; }
*/
	Vector * GetVertArray() {return m_axVertices;}
	void SetVertArray(Vector &pos, Vector *pVertArray, int vertCount);
	int GetVertCount() {return m_iNumVertices;}
	void SetInertia(float inertia);
	void SetMass(float mass);
	Vector & GetNetForce() {return m_xNetForce;}
	void SetNetForce(Vector &netForce) {m_xNetForce = netForce;}
	CL_Signal_v4<const Vector&, float&, CBody*, bool*> sig_collision; //returns information on contact
	void SetMaterial(CMaterial *pNew) {m_pMaterial = pNew;}
	CMaterial * GetMaterial() {return m_pMaterial;}

protected:
	void ProcessCollision(CBody& xBody, const Vector& N, float t);
	void ProcessOverlap  (CBody& xBody, const Vector& MTD);

	Vector* m_axVertices;
	int     m_iNumVertices;
	
	Vector	m_xVelocity;
	Vector	m_xPosition;

	float	m_fDensity;
	float	m_fMass;
	float   m_fInertia;
	float	m_fInvMass;
	float   m_fInvInertia;

	float m_fOrientation;
	float m_fAngVelocity;
	Matrix m_xOrientation;

	Vector m_xNetForce;
	float  m_fNetTorque;
	bool m_bDeleteVerts;
	CMaterial * m_pMaterial;
	MovingEntity *m_pParentEntity;

};



#endif OLI_BODY_H