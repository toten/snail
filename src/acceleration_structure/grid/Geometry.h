#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

// system include.
#include <math.h>

const Scalar EPSILON = 1.0e-6f;
const Scalar INFINITE_VALUE = 1.0e30f;

struct F3d
{
    Scalar x, y, z;

	F3d() : x(0.0f), y(0.0f), z(0.0f) {}

	F3d(Scalar _x, Scalar _y, Scalar _z) : x(_x), y(_y), z(_z) {}

	F3d(Scalar e) : x(e), y(e), z(e) {}

	F3d& operator = (const F3d& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	Scalar& operator [] (int index)
	{
		return *((Scalar*)this + index);
	}
};

inline F3d operator + (const F3d& v0, const F3d& v1)
{
	F3d v;
	v.x = v0.x + v1.x;
	v.y = v0.y + v1.y;
	v.z = v0.z + v1.z;
	return v;
}

inline F3d operator - (const F3d& v0, const F3d& v1)
{
	F3d v;
	v.x = v0.x - v1.x;
	v.y = v0.y - v1.y;
	v.z = v0.z - v1.z;
	return v;
}

inline F3d operator * (Scalar s, const F3d& v)
{
	F3d o;
	o.x = s * v.x;
	o.y = s * v.y;
	o.z = s * v.z;
	return o;
}

inline Scalar Dot(const F3d& v0, const F3d& v1)
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

inline F3d Cross(const F3d& v0, const F3d& v1)
{
	F3d v;
	v.x = v0.y * v1.z - v0.z * v1.y;
	v.y = v0.z * v1.x - v0.x * v1.z;
	v.z = v0.x * v1.y - v0.y * v1.x;
	return v;
}

struct Box
{
	Box() : MinCorner(INFINITE_VALUE, INFINITE_VALUE, INFINITE_VALUE)
		, MaxCorner(-INFINITE_VALUE, -INFINITE_VALUE, -INFINITE_VALUE){}

	F3d		MinCorner;

	F3d		MaxCorner;

	bool PointInBox(const F3d& point) const
	{
		return ((point.x >= MinCorner.x) && (point.y >= MinCorner.y) && (point.z >= MinCorner.z) &&
			(point.x <= MaxCorner.x) && (point.y <= MaxCorner.y) && (point.z <= MaxCorner.z));
	}

	void Extent(const F3d& point)
	{
		if (point.x < MinCorner.x)
			MinCorner.x = point.x;
		if (point.x > MaxCorner.x)
			MaxCorner.x = point.x;

		if (point.y < MinCorner.y)
			MinCorner.y = point.y;
		if (point.y > MaxCorner.y)
			MaxCorner.y = point.y;

		if (point.z < MinCorner.z)
			MinCorner.z = point.z;
		if (point.z > MaxCorner.z)
			MaxCorner.z = point.z;
	}

	void Extent(const Box& box)
	{
		const F3d& s = box.MinCorner;
		const F3d& e = box.MaxCorner;

		if (s.x < MinCorner.x)
			MinCorner.x = s.x;
		if (s.y < MinCorner.y)
			MinCorner.y = s.y;
		if (s.z < MinCorner.z)
			MinCorner.z = s.z;

		if (e.x > MaxCorner.x)
			MaxCorner.x = e.x;
		if (e.y > MaxCorner.y)
			MaxCorner.y = e.y;
		if (e.z > MaxCorner.z)
			MaxCorner.z = e.z;
	}
};

struct Vector {
	Vector(){}
	Vector(Scalar x1,Scalar y1,Scalar z1):x(x1),y(y1),z(z1){}
	Scalar x;
	Scalar y;
	Scalar z;
};


struct BasicTriangle{
	F3d vertex[3];
};

struct BasicRay {
	F3d original;
	F3d direction;
};

struct Triangle
{
	F3d		p0, p1, p2;

	Box		box;	

	void CalculateBox()
	{
		box.Extent(p0);
		box.Extent(p1);
		box.Extent(p2);
	}
};

struct Ray
{
	F3d		origin;
	F3d		direction;
	F3d		invDirection;

	int		sign[3];
	Scalar	dt[3];
};

bool rayTriangleIntersect(const F3d& o, const F3d& d, Scalar len,
						  const Triangle& triangle,
						  F3d& intPt);

bool rayTriangleIntersect(const F3d& o, const F3d& d,
						  const Triangle& triangle,
						  F3d& intPt);

bool rayBoxIntersect(const Ray& ray, const Box& box, Scalar& tIn);


#endif