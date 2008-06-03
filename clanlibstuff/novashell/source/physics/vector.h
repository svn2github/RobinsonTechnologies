#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glut.h>

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

#define ARGB_A(u) (((u)>>24) & 0x000000FF)
#define ARGB_R(u) (((u)>>16) & 0x000000FF)
#define ARGB_G(u) (((u)>> 8) & 0x000000FF)
#define ARGB_B(u) (((u)>> 0) & 0x000000FF)

//assert macros
#define assertf(a, text) \
{ \
	extern int assert_internal(const char* file, int line, const char* desc, ...); \
	static int s_result=0;\
	if(!(a) && (s_result == 0)) s_result = assert_internal(__FILE__, __LINE__, text); \
}
//#define assert(a) assertf(a, #a)

inline float sign(float x)
{
	return (x < 0.0f)? -1.0f : 1.0f;
}

inline float frand(float x=1.0f)
{
	return (rand() / (float) RAND_MAX) * x;
}
inline void swapf(float& a, float& b)
{
	float c = a;
	a = b;
	b = c;
}

inline float pi()
{
    static const float g_pi = atan(1.0f) * 4.0f;

	return g_pi;
}

//===========================================================================
// VECTORS
//===========================================================================
class Vector
{
public:
	float x,y;
public:
	inline Vector(void)
	{}

	inline Vector(float Ix,float Iy)
	: x(Ix)
	, y(Iy)
	{}

	inline Vector &operator /=(const float Scalar)	{ x /= Scalar; y /= Scalar;		return *this; }

	inline Vector &operator *=(const float Scalar)	{ x *= Scalar; y *= Scalar;		return *this; }
	
	inline Vector &operator +=(const Vector &Other) { x += Other.x;	y += Other.y;	return *this; }

	inline Vector &operator -=(const Vector &Other) { x -= Other.x;	y -= Other.y;	return *this;	}

	inline float operator ^ (const Vector &V)	const	{	return (x * V.y) - (y * V.x); } // cross product

	inline float operator * (const Vector &V)	const	{	return (x*V.x) + (y*V.y); } // dot product

	inline Vector operator * (float  s)			const	{	return Vector(x*s, y*s); }
	
	inline Vector operator / (float  s)			const	{	return Vector(x/s, y/s); }
	
	inline Vector operator + (const Vector &V)	const	{	return Vector(x+V.x, y+V.y); }
		
	inline Vector operator - (const Vector &V)	const	{	return Vector(x-V.x, y-V.y); }

	friend Vector operator * (float k, const Vector& V) {	return Vector(V.x*k, V.y*k); }

	
	inline Vector operator -(void) const { return Vector(-x, -y); }
	
	inline float length(void) const { return (float) sqrt(x*x + y*y); }

	void swap(Vector& other)
	{
		Vector temp = *this;
		*this = other;
		other = temp;
	}

	float normalise(void) 
	{	
		float l = length();	
		
		if (l == 0.0f) 
			return 0.0f; 
		
		(*this) /= l; 
		return l;	
	}

	Vector perp() const
	{
		return Vector(-y, x);
	}

	float angle(const Vector& xE) const
	{
		float dot = (*this) * xE;
		float cross = (*this) ^ xE;
		float angle = (float) atan2(cross, dot);
		return angle;
	}

	Vector& rotate(float angle)
	{
		float tx = x;
		x =  x * cos(angle) - y * sin(angle);
		y = tx * sin(angle) + y * cos(angle);
		return *this;
	}

	Vector& transform(const Vector& trans, float rot)
	{
		Vector D = *this;
		D.rotate(rot);
		*this = trans + D;
		return *this;
	}

	void randomise(const Vector& min, const Vector& max)
	{
		x = frand(max.x - min.x) + min.x;
		y = frand(max.y - min.y) + min.y;
	}
};
