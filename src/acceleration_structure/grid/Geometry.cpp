#include "Common.h"
#include "Geometry.h"

#define GRID_INTERSECT_ACCURACY 0

#if GRID_INTERSECT_ACCURACY 
#include <iostream>
int n;
#endif

bool rayTriangleIntersect(const F3d& o, const F3d& d, Scalar len,
						  const Triangle& triangle,
						  F3d& intPt)
{
	F3d e1 = triangle.p1 - triangle.p0;
	F3d e2 = triangle.p2 - triangle.p0;

	F3d p = Cross(d, e2);
	Scalar det = Dot(e1, p);
	if (det < -EPSILON)
	{
		F3d s = o - triangle.p0;
		Scalar u = Dot(s, p);
		if (u > 0.0f || u < det)
			return false;

		F3d q = Cross(s, e1);
		Scalar v = Dot(d, q);
		if (v > 0.0f || u + v < det)
			return false;

		Scalar t = Dot(e2, q);
		if (t > 0.0f || t < len * det)		
			return false;

#if GRID_INTERSECT_ACCURACY
		if (t > -EPSILON)
		{
			std::cout << "t > -EPSILON: " << n++ << std::endl; 
		}
		if (t < (len - EPSILON) * det)
		{
			std::cout << "t < (len - EPSILON) * det: " << n++ << std::endl; 
		}
#endif
		
		Scalar invD = 1.0f / det;
		u *= invD;
		v *= invD;
		t *= invD;

		intPt = o + t * d;

		return true;
	}
	else if (det > EPSILON)
	{
		F3d s = o - triangle.p0;
		Scalar u = Dot(s, p);
		if (u < 0.0f || u > det)
			return false;

		F3d q = Cross(s, e1);
		Scalar v = Dot(d, q);
		if (v < 0.0f || u + v > det)
			return false;

		Scalar t = Dot(e2, q);
		if (t < 0.0f || t > len * det)
			return false;

#if GRID_INTERSECT_ACCURACY
		if (t < EPSILON)
		{
			std::cout << "t < EPSILON: " << n++ << std::endl; 
		}
		if (t > (len - EPSILON) * det)
		{
			std::cout << "t > (len - EPSILON) * det: " << n++ << std::endl; 
		}
#endif
		
		Scalar invD = 1.0f / det;
		u *= invD;
		v *= invD;
		t *= invD;

		intPt = o + t * d;

		return true;
	}

	return false;
}

bool rayTriangleIntersect(const F3d& o, const F3d& d,
						  const Triangle& triangle,
						  F3d& intPt)
{
	F3d e1 = triangle.p1 - triangle.p0;
	F3d e2 = triangle.p2 - triangle.p0;

	F3d p = Cross(d, e2);
	Scalar det = Dot(e1, p);
	if (det > -EPSILON && det < EPSILON)
		return false;
	Scalar invD = 1.0f / det;

	F3d s = o - triangle.p0;
	Scalar u = invD * Dot(s, p);
	if (u < 0.0 || u > 1.0)
		return false;

	F3d q = Cross(s, e1);
	Scalar v = invD * Dot(d, q);
	if (v < 0.0 || u + v > 1.0)
		return false;

	Scalar t = invD * Dot(e2, q);
	if (t < 0.0)
		return false;
	
	intPt = o + t * d;

	return true;
}

bool rayBoxIntersect(const Ray& ray, const Box& box, Scalar& tIn)
{
	Scalar tNear = -INFINITE_VALUE;
	Scalar tFar = INFINITE_VALUE;

	// x.
	Scalar t1, t2;
	if (ray.sign[0] == 0)
	{
		if (ray.origin.x < box.MinCorner.x || ray.origin.x > box.MaxCorner.x)
			return false;
	}
	else
	{
		if (ray.sign[0] == 1)
		{
			t1 = (box.MinCorner.x - ray.origin.x) * ray.invDirection.x;
			t2 = (box.MaxCorner.x - ray.origin.x) * ray.invDirection.x;
		}
		else
		{
			t2 = (box.MinCorner.x - ray.origin.x) * ray.invDirection.x;
			t1 = (box.MaxCorner.x - ray.origin.x) * ray.invDirection.x;
		}

		tNear = t1;
		tFar = t2;

		if (tFar < 0.0f)
			return false;
	}

	// y.
	if (ray.sign[1] == 0)
	{
		if (ray.origin.y < box.MinCorner.y || ray.origin.y > box.MaxCorner.y)
			return false;
	}
	else
	{
		if (ray.sign[1] == 1)
		{
			t1 = (box.MinCorner.y - ray.origin.y) * ray.invDirection.y;
			t2 = (box.MaxCorner.y - ray.origin.y) * ray.invDirection.y;
		}
		else
		{
			t2 = (box.MinCorner.y - ray.origin.y) * ray.invDirection.y;
			t1 = (box.MaxCorner.y - ray.origin.y) * ray.invDirection.y;
		}

		if (t1 > tNear)
			tNear = t1;
		if (t2 < tFar)
			tFar = t2;

		if (tNear > tFar || tFar < 0.0f)
			return false;
	}

	// z.
	if (ray.sign[2] == 0)
	{
		if (ray.origin.z < box.MinCorner.z || ray.origin.z > box.MaxCorner.z)
			return false;
	}
	else
	{
		if (ray.sign[2] == 1)
		{
			t1 = (box.MinCorner.z - ray.origin.z) * ray.invDirection.z;
			t2 = (box.MaxCorner.z - ray.origin.z) * ray.invDirection.z;
		}
		else
		{
			t2 = (box.MinCorner.z - ray.origin.z) * ray.invDirection.z;
			t1 = (box.MaxCorner.z - ray.origin.z) * ray.invDirection.z;
		}

		if (t1 > tNear)
			tNear = t1;
		if (t2 < tFar)
			tFar = t2;

		if (tNear > tFar || tFar < 0.0f)
			return false;
	}

	tIn = tNear;
	return true;
}