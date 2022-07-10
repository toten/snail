#include "Common.h"
#include "Grid.h"
#include "Tools.h"
#include <assert.h>
#include <memory.h>

float Grid::GRID_DENSITY = 2.0f;
Scalar Grid::EXPAND_INCREMENT = 1.0f;
Scalar Grid::DELTA = 1.0e-4f;

#define OUT_RAY 0
#define CHECK_TIN 0

#if CHECK_TIN
#include <iostream>
#endif

#if OUT_RAY
int rayn;
#endif

Grid::Grid()
{
	CellNumber[0] = CellNumber[1] = CellNumber[2] = 0;
	CellNumberXY = 0;
	CellTotalNumber = 0;

	CoordX = NULL;
	CoordY = NULL;
	CoordZ = NULL;

	CellOffset = NULL;
    CellTriangles = NULL;
}

bool Grid::Initialize(Triangle* triangles, int num, const Box& triBoxAll)
{
	// Get the bounding box of the whole grid.
	CreateGridBoundingBox(triBoxAll);

	// SubDivide.
	SubDivide(num);
	CalculateCoordinates();	

	// Categorize.
	CategorizeTriangles(triangles, num);

	return true;
}

void Grid::CreateGridBoundingBox(const Box& triBoxAll)
{
	SceneBox.MinCorner = triBoxAll.MinCorner - F3d(EXPAND_INCREMENT);
	SceneBox.MaxCorner = triBoxAll.MaxCorner + F3d(EXPAND_INCREMENT);

	// The grid box is expanded a little.
	GridBox.MinCorner = triBoxAll.MinCorner - F3d(2.0f * EXPAND_INCREMENT);
	GridBox.MaxCorner = triBoxAll.MaxCorner + F3d(2.0f * EXPAND_INCREMENT);
}

void Grid::SubDivide(int num)
{
	int cellCount = int(num * GRID_DENSITY);

	F3d diagonal = GridBox.MaxCorner - GridBox.MinCorner;
	int order[3] = {0, 1, 2};
 
    Scalar s = Scalar(pow(Scalar(diagonal.x * diagonal.y * diagonal.z / cellCount), Scalar(1.0 / 3.0)));
    if (diagonal[order[0]] <= s)
        CellNumber[order[0]] = 1;
    else
    {
        CellNumber[order[0]] = int(ceil(diagonal[order[0]] / s));
        cellCount = (cellCount + CellNumber[order[0]] - 1) / CellNumber[order[0]];
    }

    // y bins.
    s = sqrt(diagonal[order[1]] * diagonal[order[2]] / cellCount);
    if (diagonal[order[1]] <= s)
        CellNumber[order[1]] = 1;
    else
    {
        CellNumber[order[1]] = int(ceil(diagonal[order[1]] / s));
        cellCount = (cellCount + CellNumber[order[1]] - 1) / CellNumber[order[1]];
    }

    // z bins.
    CellNumber[order[2]] = cellCount;

    // Recompute total count.
    CellTotalNumber = CellNumber[0] * CellNumber[1] * CellNumber[2];	
}

void Grid::CalculateCoordinates()
{
	// Get size and coordinates.
	F3d diff = GridBox.MaxCorner - GridBox.MinCorner;
	CellSize.x = diff.x / CellNumber[0];
	CellSize.y = diff.y / CellNumber[1];
	CellSize.z = diff.z / CellNumber[2];
	InvCellSize.x = 1.0f / CellSize.x;
	InvCellSize.y = 1.0f / CellSize.y;
	InvCellSize.z = 1.0f / CellSize.z;

	CoordX = new Scalar[CellNumber[0] + 1];
	CoordY = new Scalar[CellNumber[1] + 1];
	CoordZ = new Scalar[CellNumber[2] + 1];

	for (int i = 0; i < CellNumber[0]; ++i)
		CoordX[i] = GridBox.MinCorner.x + CellSize.x * i;
	CoordX[CellNumber[0]] = GridBox.MaxCorner.x;

	for (int i = 0; i < CellNumber[1]; ++i)
		CoordY[i] = GridBox.MinCorner.y + CellSize.y * i;
	CoordY[CellNumber[1]] = GridBox.MaxCorner.y;

	for (int i = 0; i < CellNumber[2]; ++i)
		CoordZ[i] = GridBox.MinCorner.z + CellSize.z * i;
	CoordZ[CellNumber[2]] = GridBox.MaxCorner.z;
}

void Grid::CategorizeTriangles(Triangle* triangles, int num)
{
    // Allocate offset array.
    CellOffset = new int[CellTotalNumber + 1];
    memset(CellOffset, 0, sizeof(int) * (CellTotalNumber + 1));

    // Two pass of triangle traversal.
    // First pass, get the triangle count for each cell, and allocate memory.    
    for (int n = 0; n < num; ++n)
	{
		assert(GridBox.PointInBox(triangles[n].box.MinCorner) &&
			GridBox.PointInBox(triangles[n].box.MaxCorner));
		
        // Check which cells overlap with the triangle bounding box.
        int startIndex[3], endIndex[3];
        IntersectTriangleBBox(triangles[n], startIndex, endIndex);

        // Accumulate triangle count for each cell.
    	for (int i = startIndex[2]; i <= endIndex[2]; ++i)
        {
	        for (int j = startIndex[1]; j <= endIndex[1]; ++j)
	        {
		        for (int k = startIndex[0]; k <= endIndex[0]; ++k)
		        {
			        int index = (CellNumber[0]*CellNumber[1])*i + CellNumber[0]*j + k;
			        CellOffset[index]++;
		        }
	        }
        }
    }
    
    // Calculate offset. After this, CellOffset[i] = offset + triangle count of this cell = offset of next cell.
    for (int i = 1; i < CellTotalNumber + 1; ++i)
        CellOffset[i] += CellOffset[i - 1];
    
    // Get the total triangle number, and allocate the triangle array.
    int totalTriangleCount = CellOffset[CellTotalNumber];
    CellTriangles = new Triangle*[totalTriangleCount];

    // Second pass, set the triangle pointers for each cell.
    for (int n = 0; n < num; ++n)
	{	
        int startIndex[3], endIndex[3];
        IntersectTriangleBBox(triangles[n], startIndex, endIndex);
        
    	for (int i = startIndex[2]; i <= endIndex[2]; ++i)
        {
	        for (int j = startIndex[1]; j <= endIndex[1]; ++j)
	        {
		        for (int k = startIndex[0]; k <= endIndex[0]; ++k)
		        {
			        int index = (CellNumber[0]*CellNumber[1])*i + CellNumber[0]*j + k;
                    // Subtract the triangle count of this cell, so get the real offset.
			        CellOffset[index]--;
                    CellTriangles[CellOffset[index]] = &triangles[n];
		        }
	        }
        }
    }
}

void Grid::IntersectTriangleBBox(Triangle& triangle, int startIndex[], int endIndex[])
{
	const F3d& start = triangle.box.MinCorner;
	const F3d& end = triangle.box.MaxCorner;

	startIndex[0] = int((start.x - GridBox.MinCorner.x) * InvCellSize.x);
	startIndex[1] = int((start.y - GridBox.MinCorner.y) * InvCellSize.y);
	startIndex[2] = int((start.z - GridBox.MinCorner.z) * InvCellSize.z);
	
	endIndex[0] = int((end.x - GridBox.MinCorner.x) * InvCellSize.x);
	endIndex[1] = int((end.y - GridBox.MinCorner.y) * InvCellSize.y);
	endIndex[2] = int((end.z - GridBox.MinCorner.z) * InvCellSize.z);

	assert((startIndex[0] <= endIndex[0]) && (startIndex[1] <= endIndex[1]) && (startIndex[2] <= endIndex[2]));
}

#if PARALLEL
#if PARSE_INSIDE
void Grid::IntersectRay(const Ray& ray, TBB_List<char*>& intersectPoints) const
#else
void Grid::IntersectRay(const Ray& ray, TBB_List<F3d>& intersectPoints) const
#endif
#else
void Grid::IntersectRay(const Ray& ray, List<F3d>& intersectPoints) const
#endif
{
	if (ray.sign[0] == 0 && ray.sign[1] == 0 && ray.sign[2] == 0)
		return;

	// Do intersection of the ray on the scene box(rather than the grid box).
	Scalar tIn;
	if (!TestRayToSceneBox(ray, tIn))
		return;

#if CHECK_TIN
	if (tIn < 0.0f)
	{
		std::cout << "tIn < 0" << tIn << std::endl;
	}
#endif

	const F3d& o = ray.origin + tIn * ray.direction;

	// Find the start cell of the origin.
	int c[3];
	c[0] = int((o.x - GridBox.MinCorner.x) * InvCellSize.x);
	c[1] = int((o.y - GridBox.MinCorner.y) * InvCellSize.y);
	c[2] = int((o.z - GridBox.MinCorner.z) * InvCellSize.z);

	assert ((c[0] >= 0) && (c[0] < CellNumber[0]) &&
		(c[1] >= 0) && (c[1] < CellNumber[1]) &&
		(c[2] >= 0) && (c[2] < CellNumber[2]));

	// Get initial tx.
	Scalar t[3];
	if (ray.sign[0] == 0)
	{
		t[0] = INFINITE_VALUE;
	}
	else 
	{
		if (ray.sign[0] == 1)
		{
			t[0] = (CoordX[c[0] + 1] - o.x) * ray.invDirection.x;
		}
		else
		{
			t[0] = (CoordX[c[0]] - o.x) * ray.invDirection.x;
		}
	}
	// Get initial ty.
	if (ray.sign[1] == 0)
	{
		t[1] = INFINITE_VALUE;
	}
	else 
	{
		if (ray.sign[1] == 1)
		{
			t[1] = (CoordY[c[1] + 1] - o.y) * ray.invDirection.y;
		}
		else
		{
			t[1] = (CoordY[c[1]] - o.y) * ray.invDirection.y;
		}
	}
	// Get initial tz.
	if (ray.sign[2] == 0)
	{
		t[2] = INFINITE_VALUE;
	}
	else 
	{
		if (ray.sign[2] == 1)
		{
			t[2] = (CoordZ[c[2] + 1] - o.z) * ray.invDirection.z;
		}
		else
		{
			t[2] = (CoordZ[c[2]] - o.z) * ray.invDirection.z;
		}
	}

	// 3DDA. Access the grid cells.
	Scalar tNear = 0;
	Scalar tLen = 0;
	F3d no = o;
	while ((c[0] >= 0) && (c[0] < CellNumber[0]) &&
		(c[1] >= 0) && (c[1] < CellNumber[1]) &&
		(c[2] >= 0) && (c[2] < CellNumber[2]))
	{
		assert(c[2] * (CellNumber[0] * CellNumber[1]) + c[1] * CellNumber[0] + c[0] >= 0);
		assert(c[2] * (CellNumber[0] * CellNumber[1]) + c[1] * CellNumber[0] + c[0] < CellTotalNumber);

		// Get current cell.
		int cellIndex = c[2] * (CellNumber[0] * CellNumber[1]) + c[1] * CellNumber[0] + c[0];

		// Find the next cell.
		int minIndex = 0;
		if (t[1] < t[minIndex])
			minIndex = 1;
		if (t[2] < t[minIndex])
			minIndex = 2;

		tLen = t[minIndex] - tNear;
		tNear = t[minIndex];
		t[minIndex] += ray.dt[minIndex];
		c[minIndex] += ray.sign[minIndex];		

		// Test current cell.
		tLen = (tLen - Scalar(1e-16) < 0) ? 0 : tLen - Scalar(1e-16);
		IntersectRay(no, ray.direction, tLen, cellIndex, intersectPoints);

		// Get next start.
		no = o + tNear * ray.direction;
	};
}

#if PARALLEL
#if PARSE_INSIDE
void Grid::IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, TBB_List<char*>& intersectPoints) const
#else
void Grid::IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, TBB_List<F3d>& intersectPoints) const
#endif
#else
void Grid::IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, List<F3d>& intersectPoints) const
#endif
{
    F3d intPt;
    for (int i = CellOffset[cellIndex]; i < CellOffset[cellIndex + 1]; ++i)
    {
        if (rayTriangleIntersect(o, d, len, *(CellTriangles[i]), intPt))
        {
#if PARSE_INSIDE
            char *buf = new char[128];
#if CUSTOM_OUT
            vertex2str_g(&intPt.x, buf);
#else
            sprintf_s( buf, 128, "%g %g %g\r\n", intPt.x,intPt.y,intPt.z );
#endif
            intersectPoints.push_back(buf);
#else
            intersectPoints.push_back(intPt);
#endif
        }
    }
}

bool Grid::TestRayToSceneBox(const Ray& ray, Scalar& tIn) const
{
	// If ray.origin in the scene box, no need to clip.
	if (SceneBox.PointInBox(ray.origin))
	{
		tIn = 0.0f;
		return true;
	}

#if OUT_RAY
	std::cout << rayn++ << std::endl;
#endif
	
	// Do ray-box inersect, and find the t-value of the intersect point.
	if (rayBoxIntersect(ray, SceneBox, tIn))
		return true;

	return false;
}

Grid::~Grid()
{
	delete [] CoordX;
	delete [] CoordY;
	delete [] CoordZ;

	delete [] CellOffset;
    delete [] CellTriangles;
}