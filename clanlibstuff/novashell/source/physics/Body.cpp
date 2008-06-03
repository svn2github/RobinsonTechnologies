#include "AppPrecomp.h"
#include "Body.h"
#include "MovingEntity.h"

void renderARGB(int colour)
{
	glColor4ub(ARGB_R(colour), ARGB_G(colour), ARGB_B(colour), ARGB_A(colour));
}
void renderDottedSegment(const Vector& start, const Vector& end, int colour)
{
	renderARGB(colour);
	
	glLineStipple(1, 15);
	glBegin(GL_LINES);
	glVertex2f(start.x, start.y);
	glVertex2f(end.x, end.y);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void renderSegment(const Vector& start, const Vector& end, int colour)
{
	renderARGB(colour);
	
	glBegin(GL_LINES);
	glVertex2f(start.x, start.y);
	glVertex2f(end.x, end.y);
	glEnd();
}

void renderArrow(const Vector& start, const Vector& dir, float dist, int colour)
{
	renderARGB(colour);
	
	float angle = dir.angle(Vector(1, 0));
	glPushMatrix();
	glTranslatef(start.x, start.y, 0.0f);
	glRotatef(angle * 180.0f / pi(), 0.0f, 0.0f, -1.0f);
	glScalef(dist, dist, 1.0f);
	glColor4ub(ARGB_R(colour), ARGB_G(colour), ARGB_B(colour), ARGB_A(colour));
	glBegin(GL_LINES);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(0.9f, 0.1f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(0.9f,-0.1f);
	glEnd();
	glPopMatrix();
}


void renderArrow(const Vector& start, const Vector& end, int colour)
{
	Vector dir = (end - start);
	float dist = dir.normalise();
	renderArrow(start, dir, dist, colour);
}

Body::Body()
{
	
	m_invinertia	= 0.0f;
	m_invmass		= 0.0f;
	m_orientation	= 0.0f;
	m_position		= Vector(0, 0);
	//m_polygon		= Poly(min, max);
	m_mass = 0;
}

Body::Body(const Vector& min, const Vector& max)
{
	m_invinertia	= 0.0f;
	m_invmass		= 0.0f;
	m_orientation	= 0.0f;
	m_position		= Vector(0, 0);
	//m_polygon		= Poly(min, max);
	m_mass = 0;
}

Vector Body::GetLinVelocity()
{
	return Vector(fabs(m_velocity.x), fabs(m_velocity.y));
}

Body::Body(float worldSize)
{
	m_position.randomise(Vector(worldSize * 0.25f, worldSize * 0.25f), Vector(worldSize * 0.75f, worldSize * 0.75f));
	m_orientation = frand(pi());
	m_velocity.randomise(Vector(-worldSize * 0.01f, -worldSize * 0.01f), Vector(worldSize * 0.01f, worldSize * 0.01f));
	m_angvelocity = frand(0.01f);

	int count = rand() % 6 + 3;
	float rad = frand(worldSize * 0.05f) + worldSize * 0.05f;
	m_polygon = Poly(count, rad);

	float density  = (frand(1.0f) < 0.3f)? 0.0f : frand(0.3f) + 0.7f;
	float inertia  = (density == 0.0f)? 0.0f : m_polygon.calculateInertia();
	m_mass     = (density == 0.0f)? 0.0f : m_polygon.calculateMass(density);

	m_invmass = (m_mass > 0.0f)? 1.0f / m_mass : 0.0f;
	m_invinertia = (inertia > 0.0f)? 1.0f / (inertia *m_mass) : 0.0f; 
}


void Body::update(float dt)
{
	if(m_invmass == 0.0f)
	{
		m_velocity = Vector(0, 0);
		m_angvelocity = 0.0f;
		return;
	}

	m_orientation += (m_angvelocity*dt);	
	m_position += (m_velocity*dt);
}

void Body::render() const
{
	Poly thisPoly = m_polygon;
	thisPoly.transform(m_position, m_orientation);

	renderARGB(0x808080FF);
	thisPoly.render(true);

	renderARGB(0xFFFFFFFF);
	thisPoly.render(false);

	renderArrow(m_position, m_position + m_velocity, 0xff80ff80);
}

void Body::SetMass( float mass )
{
	//TODO
}

void Body::SetVertArray( Vector &pos, Vector *pVertArray, int vertCount )
{
	
	m_position = pos;
	m_polygon.SetVertInfo(pVertArray, vertCount);

	float density  = (frand(1.0f) < 0.3f)? 0.0f : frand(0.3f) + 0.7f;
	float inertia  = (density == 0.0f)? 0.0f : m_polygon.calculateInertia();
	m_mass     = (density == 0.0f)? 0.0f : m_polygon.calculateMass(density);

	m_invmass = (m_mass > 0.0f)? 1.0f / m_mass : 0.0f;
	m_invinertia = (inertia > 0.0f)? 1.0f / (inertia *m_mass) : 0.0f; 
	//TODO
}

void Body::AddForce( Vector f )
{
	m_velocity += f;
}

void Body::AddForce( Vector f, Vector t )
{
	m_velocity += f;
}
CollisionReport::CollisionReport()
{
	m_collisionReported = false;
	m_body[0] = m_body[1] = NULL;
	m_ncoll = Vector(0, 0);
	m_mtd = Vector(0, 0);
	m_tcoll = 0.0f;
}

CollisionReport::CollisionReport(Body* a, Body* b)
{
	m_collisionReported = false;
	m_body[0]			= a;
	m_body[1]			= b;
	m_contacts			= ContactInfo();
	m_ncoll				= Vector(0, 0);
	m_mtd				= Vector(0, 0);
	m_tcoll				= 0.0f;

	// polygons in world space at the time of collision	
	m_poly[0] = a->m_polygon;
	m_poly[1] = b->m_polygon;
	m_poly[0].transform(a->m_position, a->m_orientation);
	m_poly[1].transform(b->m_position, b->m_orientation);
	
	// find collision
	Vector delta = (a->m_velocity - b->m_velocity);
	m_collisionInfo = m_poly[0].collide(m_poly[1], delta);
	m_collisionReported = (m_collisionInfo.m_overlapped || m_collisionInfo.m_collided);
		
	if(!m_collisionReported)
		return;

	// convert collision info into collison plane info
	if(m_collisionInfo.m_overlapped)
	{
		if(m_collisionInfo.m_mtdLengthSquared <= 0.00001f)
		{
			m_collisionReported = false;
			return;
		}

		m_ncoll = m_collisionInfo.m_mtd / sqrt(m_collisionInfo.m_mtdLengthSquared);
		m_tcoll = 0.0f;
		m_mtd   = m_collisionInfo.m_mtd;
	}
	else if(m_collisionInfo.m_collided)
	{
		m_ncoll = m_collisionInfo.m_Nenter;
		m_tcoll = m_collisionInfo.m_tenter;
	}	

	// find contact points at time of collision
	m_poly[0].translate(a->m_velocity * m_tcoll);
	m_poly[1].translate(b->m_velocity * m_tcoll);
	
	SupportPoints asup = m_poly[0].getSupports(m_ncoll);
	SupportPoints bsup = m_poly[1].getSupports(-m_ncoll);

	m_contacts = ContactInfo(asup, bsup);
};

void CollisionReport::render()
{
	if(!m_collisionReported)
		return;

	renderARGB(0x80FFA080);
	m_poly[0].render(true);
	m_poly[1].render(true);

	renderARGB(0xFFFFFFFF);
	m_poly[0].render(false);
	m_poly[1].render(false);

	for(int i = 0; i < m_contacts.m_count; i ++)
	{
		glLineWidth(4.0f);
		renderDottedSegment(m_contacts.m_contact[i].m_position[0], m_contacts.m_contact[i].m_position[1], 0xffff0000);
		glLineWidth(2.0f);
		renderArrow(m_contacts.m_contact[i].m_position[0], m_ncoll, 10, 0xffffff00);
		glLineWidth(1.0f);
	}
}

void CollisionReport::applyReponse(float CoR)
{
	if(!m_collisionReported)
		return;

	Body* a = m_body[0];
	Body* b = m_body[1];

	// overlapped. then separate the bodies.
	a->m_position += m_mtd * (a->m_invmass / (a->m_invmass + b->m_invmass));
	b->m_position -= m_mtd * (b->m_invmass / (a->m_invmass + b->m_invmass));
	
	// move to time of collision
	a->m_position += a->m_velocity * m_tcoll;
	b->m_position += b->m_velocity * m_tcoll;

	// apply collision impusles at contacts
	for(int i = 0; i < m_contacts.m_count; i ++)
	{
		Vector pa = m_contacts.m_contact[i].m_position[0];
		Vector pb = m_contacts.m_contact[i].m_position[1];
		Vector ra = pa - a->m_position;
		Vector rb = pb - b->m_position;
		Vector va = a->m_velocity + ra.perp() * a->m_angvelocity;
		Vector vb = b->m_velocity + rb.perp() * b->m_angvelocity;

		float vn = ((va - vb) * m_ncoll);
		if(vn > 0.0f) continue;

		float ta	= (ra ^ m_ncoll) * (ra ^ m_ncoll) * a->m_invinertia;
		float tb	= (rb ^ m_ncoll) * (rb ^ m_ncoll) * b->m_invinertia;
		float m		= a->m_invmass + b->m_invmass;
		float denom = m + ta + tb;
		float j		= -(1.0f + CoR) * vn / denom;
		
		Vector impulse = m_ncoll * j;

		a->m_velocity += impulse * a->m_invmass;
		b->m_velocity -= impulse * b->m_invmass;

		a->m_angvelocity += (ra ^ impulse) * a->m_invinertia;
		b->m_angvelocity -= (rb ^ impulse) * b->m_invinertia;
	}
}