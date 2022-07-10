#ifndef _GRID_H_
#define _GRID_H_

#include "Geometry.h"
#include "Container.h"

struct Grid
{
public:
	// Bounding box of the grid.
	Box			SceneBox;
	Box			GridBox;

	// Cell number in x, y, z directions.
	int			CellNumber[3];
	int			CellNumberXY;
	int			CellTotalNumber;

	// Size of the cell.
	F3d			CellSize;
	F3d			InvCellSize;

	// Coordinates for cell boarder in x, y, z directions.
	Scalar*		CoordX;
	Scalar*		CoordY;
	Scalar*		CoordZ;

	// Cells.
    int*        CellOffset;
	Triangle**	CellTriangles;

public:
	// Grid density.
	static float            GRID_DENSITY;

	static Scalar			EXPAND_INCREMENT;

	static Scalar			DELTA;

public:
	Grid();

	~Grid();

public:	
	bool Initialize(Triangle* triangles, int num, const Box& triBoxAll);

	void CreateGridBoundingBox(const Box& triBoxAll);

	void SubDivide(int num);

	void CalculateCoordinates();

	void CategorizeTriangles(Triangle* triangles, int num);

	void IntersectTriangleBBox(Triangle& triangle, int startIndex[], int endIndex[]);

#if PARALLEL
#if PARSE_INSIDE
	void IntersectRay(const Ray& ray, TBB_List<char*>& intersectPoints) const;
    void IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, TBB_List<char*>& intersectPoints) const;
#else
    void IntersectRay(const Ray& ray, TBB_List<F3d>& intersectPoints) const;
    void IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, TBB_List<F3d>& intersectPoints) const;
#endif
#else    
	void IntersectRay(const Ray& ray, List<F3d>& intersectPoints) const;
    void IntersectRay(const F3d& o, const F3d& d, Scalar len, int cellIndex, List<F3d>& intersectPoints) const;
#endif

#if PARALLEL

	
#else
	
#endif

	bool TestRayToSceneBox(const Ray& ray, Scalar& tIn) const;
};

#endif