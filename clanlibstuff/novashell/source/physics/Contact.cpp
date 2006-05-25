//------------------------------------------------------------------------------------------------ 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// File          : Contact.cpp
// 
// Created by    : OR - 07/02/2004 12:42:27
// 
// Copyright (C) : 2004 Rebellion, All Rights Reserved.
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Description   : 
// --------------- 
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
//------------------------------------------------------------------------------------------------ 
//SETH


#include "AppPrecomp.h"
#include "Contact.h"
#include "Body.h"

 bool   dbg_UseImpulses = true;
 bool   dbg_UseSeparation = true;
 bool   dbg_UseFriction = true;

// HACK : use a shared material for all objects
CMaterial s_xContactMaterial;

/*
void ResolveOverlap(  const Vector& Ncoll, float depth, float spring_value, float spring_damper,
					  const Vector& C0, const Vector& P0, Vector& V0, float w0,
					  const Vector& C1, const Vector& P1, Vector& V1, float w1,
					  Vector& Fspring)
{

	Vector R0    = C0 - P0;
	Vector R1    = C1 - P1;
	Vector T0    = Vector(-R0.y, R0.x);
	Vector T1    = Vector(-R1.y, R1.x);
	Vector VP0   = V0 - T0 * w0; // point velocity (SIGN IS WRONG)
	Vector VP1   = V1 - T1 * w1; // point velocity (SIGN IS WRONG)

	Vector D = C0 - C1;
	Vector V = VP0 - VP1;
	
	float  vn = (V * Ncoll);
	float  dn = (D * Ncoll);

//	if (dn > 0.0f) dn = 0.0f;

	float  f = (spring_value * dn) - (spring_damper * vn);

	f = clampf(f, -1000.0f, 1000.0f);
	Fspring = -f * Ncoll;
}

*/

////////////////////////////////////////////////////////////////
// ResolveCollision
////////////////////////////////////////////////////////////////
// calculates the change in angular and linear velocity of two 
// colliding objects
////////////////////////////////////////////////////////////////
// parameters : 
// ------------
// Ncoll : normal of collision
// t : time of collision, (if negative, it's a penetration depth)
// fCoF : coefficient of friction
// fCoR : coefficient of restitution
// C0 : point of contact on object 0
// P0 : centre of gravity (position) of object 0
// V0 : linear Velocity of object 0
// w0 : angular velocity of object 0
// m0 : inverse mass of object 0
// i0 : inverse inertia of object 0
// C1 : point of contact on object 1
// P1 : centre of gravity (position) of object 1
// V1 : linear Velocity of object 1
// w1 : angular velocity of object 1
// m1 : inverse mass of object 1
// i1 : inverse inertia of object 1
//
// return values : V0, w0, V1, w1 will change upon a collision
// -------------
///////////////////////////////////////////////////////////////
void ResolveCollision(Vector Ncoll, float t, float fCoF, float fCoR,
					  const Vector& C0, const Vector& P0, Vector& V0, float& w0, float m0, float i0, 
					  const Vector& C1, const Vector& P1, Vector& V1, float& w1, float m1, float i1,
					  float fCoS)
{
	
	
	//------------------------------------------------------------------------------------------------------
	// pre-computations
	//------------------------------------------------------------------------------------------------------
	float tcoll = (t > 0.0f)? t : 0.0f;


	//seth's hack for stability

	
	const float epsilon = 0.1f;

	if (fabs(double(Ncoll.x)) < epsilon ) Ncoll.x = 0;
	if (fabs(double(Ncoll.y)) < epsilon ) Ncoll.y = 0;
	
	Ncoll.Normalise();
	

	Vector Q0    = P0 + V0 * tcoll;
	Vector Q1    = P1 + V1 * tcoll;
	Vector R0    = C0 - Q0;
	Vector R1    = C1 - Q1;
	Vector T0    = Vector(-R0.y, R0.x);
	Vector T1    = Vector(-R1.y, R1.x);
	Vector VP0   = V0 - T0 * w0; // point velocity (SIGN IS WRONG)
	Vector VP1   = V1 - T1 * w1; // point velocity (SIGN IS WRONG)

	//------------------------------------------------------------------------------------------------------
	// impact velocity
	//------------------------------------------------------------------------------------------------------
	Vector Vcoll = VP0 - VP1;
	float  vn	 = Vcoll * Ncoll;
	Vector Vn	 = vn    * Ncoll;
	Vector Vt	 = Vcoll - Vn;

	// separation
	if (vn > 0.0f)
		return;
		
	float  vt = Vt.Normalise();

	
	//------------------------------------------------------------------------------------------------------
	// compute impulse (frction and restitution).
	// ------------------------------------------
	//
	//									-(1+Cor)(Vel.norm)
	//			j =  ------------------------------------------------------------
	//			     [1/Ma + 1/Mb] + [Ia' * (ra x norm)²] + [Ib' * (rb x norm)²]
	//------------------------------------------------------------------------------------------------------
	Vector J;
	Vector Jt(0.0f, 0.0f);
	Vector Jn(0.0f, 0.0f);
		
	float t0 = (R0 ^ Ncoll) * (R0 ^ Ncoll) * i0;
	float t1 = (R1 ^ Ncoll) * (R1 ^ Ncoll) * i1;
	float m  = m0 + m1;
	
	float denom = m + t0 + t1;

	float jn = vn / denom;

	Jn = (-(1.0f + fCoR) * jn) * Ncoll;
	
	if (dbg_UseFriction)
	{
		Jt = (fCoF * jn) * Vt.Direction();
	//	printf("jt is %.2f, %.2f", Jt.x, Jt.y);
	}

	J = Jn + Jt;

	//------------------------------------------------------------------------------------------------------
	// changes in momentum
	//------------------------------------------------------------------------------------------------------
	Vector dV0 = J * m0;
	Vector dV1 =-J * m1;

	float dw0 =-(R0 ^ J) * i0; // (SIGN IS WRONG)
	float dw1 = (R1 ^ J) * i1; // (SIGN IS WRONG)

	//------------------------------------------------------------------------------------------------------
	// apply changes in momentum
	//------------------------------------------------------------------------------------------------------
	if (m0 > 0.0f) V0 += dV0;
	if (m1 > 0.0f) V1 += dV1;
	
	if (m0 > 0.0f) w0 += dw0;
	if (m1 > 0.0f) w1 += dw1;
	
	//LogMsg("Vel added %f, %f, Ang added %f", dV1.x, dV1.y,  dw1);

	//------------------------------------------------------------------------------------------------------
	// Check for static frcition
	//------------------------------------------------------------------------------------------------------
	if (vn < 0.0f && fCoF > 0.0f)
	{
		float cone = -vt / vn;

		//printf("Maybe? cone %.2f, cof %.2f", cone, fCoF);
		if ( cone < fCoF)
		{
		
			// treat static friciton as a collision at the contact point
			Vector Nfriction = -Vt.Direction();
			//float fCoS = s_xContactMaterial.GetStaticFriction();
			//printf("fric is %f, dir is %f, %f\n", fCoS, Vt.x, Vt.y);
			ResolveCollision(Nfriction, 0.0f, 0.0f, fCoS,
							 C0, P0, V0, w0, m0, i0, 
							 C1, P1, V1, w1, m1, i1, fCoS);
							 
		}
	}

//	LogMsg("w0: %.2f, V0: %.2f, %.2f", w0, V0.x, V0.y );
}

CContact::CContact()
{
	Reset();
}

void CContact::Reset()
{
	m_pxBodies[0] = m_pxBodies[1] = NULL;
	m_iNumContacts = 0;
}
CContact::CContact(const Vector* CA, const Vector* CB, int Cnum, 
				   const Vector& N, float t, 
				   CBody* pxBodyA, CBody* pxBodyB, const CMaterial *mat)
{
	m_iNumContacts = 0;
	m_pxBodies[0]  = pxBodyA;
	m_pxBodies[1]  = pxBodyB;
	m_xNormal	   = N;
	m_t			   = t;

	m_pMaterial = mat;
	for(int i = 0; i < Cnum; i ++)
	{
		AddContactPair(CA[i], CB[i]);
	}
}

/*
void CContact::Render(void) const
{
	for(int i = 0; i < m_iNumContacts; i ++)
	{
		glBegin(GL_LINES);
		glColor4f(1.0f, 0.3f, 0.3f, 1.0f);
		glVertex2fv(&m_xContacts[i][0].x);
		glVertex2fv(&m_xContacts[i][1].x);
		glEnd();

		glBegin(GL_POINTS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex2fv(&m_xContacts[i][0].x);
		glVertex2fv(&m_xContacts[i][1].x);
		glEnd();
	}
}
*/

void CContact::AddContactPair(const Vector& CA, const Vector& CB)
{
	if (m_iNumContacts >= eMaxContacts)
		return;

	m_xContacts[m_iNumContacts][0] = CA;
	m_xContacts[m_iNumContacts][1] = CB;
	m_iNumContacts++;
}

void CContact::Solve()
{
	
	if (m_t < 0.0f)
		ResolveOverlap();
	
	ResolveCollision();
}

void CContact::ResolveOverlap()
{
	if (!m_pxBodies[0] || !m_pxBodies[1])
		return;

	for(int i = 0; i < m_iNumContacts; i ++)
	{
		ResolveOverlap(m_xContacts[i][0], m_xContacts[i][1]);
	}
}

void CContact::ResolveOverlap(const Vector& C0, const Vector& C1)
{
/*	Vector Fspring;
	float spring_value = 10000.0f;
	float spring_damper = 100.0f;

	const Vector& P0	= m_pxBodies[0]->GetPosition();
	const Vector& P1	= m_pxBodies[1]->GetPosition();
	Vector& V0			= m_pxBodies[0]->GetLinVelocity();
	Vector& V1			= m_pxBodies[1]->GetLinVelocity();
	float&  w0			= m_pxBodies[0]->GetAngVelocity();
	float&  w1			= m_pxBodies[1]->GetAngVelocity();

	::ResolveOverlap(m_xNormal, m_t, spring_value, spring_damper, 
					 C0, P0, V0, w0,
					 C1, P1, V1, w1,
					 Fspring);

	m_pxBodies[0]->AddForce(C0, Fspring);
	m_pxBodies[1]->AddForce(C1,-Fspring);

	return;
/**/
	float m0 = m_pxBodies[0]->GetInvMass();
	float m1 = m_pxBodies[1]->GetInvMass();
	float m  = m0 + m1;

	Vector D = C1 - C0;
	float fRelaxation = m_pMaterial->GetSeparation();

	D *= fRelaxation;
	Vector D0 = Vector(0.0f, 0.0f);
	Vector D1 = Vector(0.0f, 0.0f);

	if (m0 > 0.0f)
	{
		D0 = D * (m0 / m);
		m_pxBodies[0]->GetPosition() += D0;
	//	LogMsg("Added %f, %f", D0.x, D0.y);
	}
	if (m1 > 0.0f) 
	{
		D1 = D * -(m1 / m);
		m_pxBodies[1]->GetPosition() += D1;
		
	}
}

void CContact::ResolveCollision()
{
	if (!m_pxBodies[0] || !m_pxBodies[1])
		return;

	for(int i = 0; i < m_iNumContacts; i ++)
	{
		ResolveCollision(m_xContacts[i][0], m_xContacts[i][1]);
	}
}

void CContact::ResolveCollision(const Vector& C0, const Vector& C1)
{
	float m0 = m_pxBodies[0]->GetInvMass();
	float m1 = m_pxBodies[1]->GetInvMass();
	float i0 = m_pxBodies[0]->GetInvInertia();
	float i1 = m_pxBodies[1]->GetInvInertia();

	const Vector& P0	= m_pxBodies[0]->GetPosition();
	const Vector& P1	= m_pxBodies[1]->GetPosition();
	Vector& V0			= m_pxBodies[0]->GetLinVelocity();
	Vector& V1			= m_pxBodies[1]->GetLinVelocity();
	float&  w0			= m_pxBodies[0]->GetAngVelocity();
	float&  w1			= m_pxBodies[1]->GetAngVelocity();


	::ResolveCollision(-m_xNormal, m_t, m_pMaterial->GetFriction(), m_pMaterial->GetRestitution(),
					   C1, P1, V1, w1, m1, i1,
					   C0, P0, V0, w0, m0, i0,
					   m_pMaterial->GetStaticFriction());

}
