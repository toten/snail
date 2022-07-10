#include "Common.h"
#include "ParallelTask.h"
#include "Tools.h"

using namespace std;

#if PARALLEL
void RayParallel::operator()(const tbb::blocked_range<int>& r) const
{
    for(int i = r.begin(); i != r.end(); ++i)
    {
		mGrid->IntersectRay(mRays[i], mIntersectPoints);
    }
}

void ParseTriangleTask::operator()(const tbb::blocked_range<int>& r) const
{
    for(int i = r.begin(); i != r.end(); ++i)
    {
		BasicTriangle& btri = mBasicTriangles[i];

		mTriangles[i].p0 = btri.vertex[0];
		mTriangles[i].p1 = btri.vertex[1];
		mTriangles[i].p2 = btri.vertex[2];
    }
}

void ParseRayTask::operator()(const tbb::blocked_range<int>& r) const
{
    for(int i = r.begin(); i != r.end(); ++i)
    {
		BasicRay& bray = mBasicRays[i];

		mRays[i].origin = bray.original;
		mRays[i].direction = bray.direction;
    }
}

#endif