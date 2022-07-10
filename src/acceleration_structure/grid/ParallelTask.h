#ifndef _PARALLEL_TASK_H_
#define _PARALLEL_TASK_H_

#include "Geometry.h"
#include "Grid.h"

// tbb includes
#include "parallel_for.h"

// std includes
#include <vector>

#if PARALLEL
class RayParallel
{
protected:
    Ray* mRays;
    Grid* mGrid;
#if PARSE_INSIDE
    TBB_List<char*>& mIntersectPoints;
#else
    TBB_List<F3d>& mIntersectPoints;
#endif

public:
#if PARSE_INSIDE
    RayParallel(Ray* rays, Grid* scene, TBB_List<char*>& ret)
#else
    RayParallel(Ray* rays, Grid* scene, TBB_List<F3d>& ret)
#endif
        : mRays(rays)
        , mGrid(scene)
        , mIntersectPoints(ret)
    {
    }

    void operator()(const tbb::blocked_range<int>& r) const;

private:
    RayParallel & operator=( const RayParallel& );
};

class ParseTriangleTask
{
protected:
    Triangle* mTriangles;
    BasicTriangle* mBasicTriangles;
public:
    ParseTriangleTask(Triangle* triangle, BasicTriangle* basicTriangles) 
        : mTriangles(triangle)
        , mBasicTriangles(basicTriangles)
    {}
    void operator()(const tbb::blocked_range<int>& r) const;

private:
    ParseTriangleTask & operator=( const ParseTriangleTask& );
};

class ParseRayTask
{
protected:
    Ray* mRays;
    BasicRay* mBasicRays;
public:
    ParseRayTask(Ray* rays, BasicRay* basicRays) 
        : mRays(rays)
        , mBasicRays(basicRays)
    {}
    void operator()(const tbb::blocked_range<int>& r) const;

private:
    ParseRayTask & operator=( const ParseRayTask& );
};

#endif

#endif