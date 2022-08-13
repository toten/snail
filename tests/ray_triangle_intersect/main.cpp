// ContestSample.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "Geometry.h"
#include "Grid.h"
#include "Tools.h"

// tbb includes
#include "ParallelTask.h"
#if PARALLEL
#include "task_scheduler_init.h"
#include "blocked_range.h"
#endif

// system include.
#include <stdio.h>
#include <time.h>
#include <windows.h>

#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#define TIGHT_BOX 1
#define MORE_OUTPUT 0

const Scalar CONFINEMENT = 1000.0f;

#if !PARALLEL
void ComputeIntersections(Grid& grid, int numOfRays, Ray* rays, List<F3d>& intersectPoints);
#endif

void RayPreCompute(Ray& ray, const Grid& grid);

void* FileRead(const char* filename, int interval, std::vector<char*>& outList, unsigned int& numberOfGeom);

int main(int argc, char* argv[])
{
	printf("hello\n");

    if (argc != 4)
    {
        printf("Usage: ContestSample.exe geometry_input.txt ray_input.txt output.txt\n");
        return 0;
    }

	LARGE_INTEGER performanceCount;
    QueryPerformanceFrequency(&performanceCount);
    Scalar freqency = (Scalar)(performanceCount.QuadPart);

	QueryPerformanceCounter(&performanceCount);
	long long startCount = performanceCount.QuadPart;
	long long endCount;
	long long totalCount = 0;

    const char* geomFile = argv[1];
	const char* rayFile = argv[2];
	const char* outputFile = argv[3];

	FILE *fp1 = NULL;
	fopen_s(&fp1, geomFile, "rb");
	if (fp1 == NULL) {
		printf("Can't open %s.\n", geomFile);
		return -1;
	}
	FILE *fp2 = NULL;
	fopen_s(&fp2, rayFile, "rb");
	if (fp2 == NULL) {
		printf("Can't open %s.\n", rayFile);
		return -1;
	}

	unsigned int numOfTriangles;
	unsigned int numOfRays;
	BasicTriangle *basicTriangles;
	BasicRay *basicRays;

	// Read triangles
	fread(&numOfTriangles, sizeof(numOfTriangles), 1, fp1);
	basicTriangles = new BasicTriangle[numOfTriangles];
	fread(basicTriangles, sizeof(BasicTriangle) ,numOfTriangles, fp1);
	fclose(fp1);

	// Read rays
	fread(&numOfRays, sizeof(numOfRays), 1, fp2);
	basicRays = new BasicRay[numOfRays];
	fread(basicRays, sizeof(BasicRay) ,numOfRays, fp2);
	fclose(fp2);

    // allocate triangle memory
    Triangle* triangles = new Triangle[numOfTriangles];

    // convert BasicTriangle to Triangle.
#if PARALLEL
    tbb::parallel_for(tbb::blocked_range<int>(0,numOfTriangles),ParseTriangleTask(triangles,basicTriangles));
#else
    for (unsigned int i = 0; i < numOfTriangles; ++i)
    {
		BasicTriangle& btri = basicTriangles[i];

		triangles[i].p0 = btri.vertex[0];
		triangles[i].p1 = btri.vertex[1];
		triangles[i].p2 = btri.vertex[2];
    }    
#endif

    // allocate the ray memory
    Ray* rays = new Ray[numOfRays];

    // Convert BasicRay to ray
#if PARALLEL
    tbb::parallel_for(tbb::blocked_range<int>(0,numOfRays),ParseRayTask(rays,basicRays));
#else
    for (unsigned int i = 0; i < numOfRays; ++i)
    {
        BasicRay& bray = basicRays[i];

		rays[i].origin = bray.original;
		rays[i].direction = bray.direction;
    }
#endif

	QueryPerformanceCounter(&performanceCount);
	endCount = performanceCount.QuadPart;
	cout << "read time: " << (Scalar)(endCount - startCount) / freqency << endl;
	totalCount += endCount - startCount;
	startCount = endCount;

    // this is the container for all the output intersection points
    // here we simply use std::vector as the container but you will
    // need to change to a thread-safe container if your computing is
    // multi-threaded.
#if PARALLEL
#if PARSE_INSIDE
    TBB_List<char*> intersectPoints;
    intersectPoints.reserve(1024*1024);
    intersectPoints.clear();
#else
    TBB_List<F3d> intersectPoints;
    intersectPoints.reserve(1024*1024);
    intersectPoints.clear();
#endif
#else
    List<F3d> intersectPoints;
#endif

    // now you can perform the computation of the intersection points
    // but please note that parallelism isn't the panacea. It cannot 
    // eliminate the need for a smart sequential algorithsm that reduces
    // the time complexity of the computation. One good approach is to first
    // write your program like you usually do - write a sequential version first.
    // And try your best to reduce the time complexity of your sequential algorithsm
    // e.g. if it's O(N * N), try to reduce it to O(N * logN). If it's O(N), try to
    // see if it's possible to reduce it to O(logN). When you have a good sequential 
    // algorithsm you can start thinking about where to parallelize.


	// Compute bbox of triangle.
	Box triBoxAll;
	for (unsigned int i = 0; i < numOfTriangles; ++i)
	{
		triangles[i].CalculateBox();
#if TIGHT_BOX
		triBoxAll.Extent(triangles[i].box);
#endif
	}

#if !TIGHT_BOX
	triBoxAll.MinCorner = F3d(-CONFINEMENT);
	triBoxAll.MaxCorner = F3d(CONFINEMENT);
#endif

	// Create grid.
	Grid grid;
	grid.Initialize(triangles, numOfTriangles, triBoxAll);

	// Precompute of Ray.	
    for (unsigned int i = 0; i < numOfRays; ++i)
	{
		RayPreCompute(rays[i], grid);
	}

	QueryPerformanceCounter(&performanceCount);
	endCount = performanceCount.QuadPart;
	cout << "create grid time: " << (Scalar)(endCount - startCount) / freqency << endl;
	totalCount += endCount - startCount;
	startCount = endCount;

#if PARALLEL
	tbb::task_scheduler_init init;
	tbb::parallel_for(tbb::blocked_range<int>(0, numOfRays), RayParallel(rays, &grid, intersectPoints), tbb::auto_partitioner());
#else
	ComputeIntersections(grid, numOfRays, rays, intersectPoints);
#endif

	QueryPerformanceCounter(&performanceCount);
	endCount = performanceCount.QuadPart;
	cout << "intersect time: " << (Scalar)(endCount - startCount) / freqency << endl;
	totalCount += endCount - startCount;
	startCount = endCount;

    // now computation finishes. All the intersection points are now in the container intersectPoints.
    // before we output results, we first delete the input data that we no longer use
	delete[] rays;
	delete[] triangles;	

    FILE* file;
    fopen_s( &file,outputFile,"wb");

    // we now output the results into the specified file
    long nBytes = long(intersectPoints.size());
    // can the length of the buffer exceed 128?
    char buf[128];
    sprintf_s(buf,128,"%d\r\n",nBytes);
    string s(buf);

#if PARALLEL
#if PARSE_INSIDE
    TBB_List< char* >::iterator itEnd = intersectPoints.end();
    TBB_List< char* >::iterator it;
#else
    TBB_List<F3d>::iterator itEnd = intersectPoints.end();
    TBB_List<F3d>::iterator it;
#endif
#else
    List<F3d>::iterator itEnd = intersectPoints.end();
    List<F3d>::iterator it;
#endif
    for(it = intersectPoints.begin() ;it != itEnd; ++it )
    {
#if PARSE_INSIDE
        char* pStr = (*it);
        s += pStr;
        delete[] pStr;
#else
        F3d& intPt = *it;
#if CUSTOM_OUT
        vertex2str_g(&intPt.x, buf);
#else
        sprintf_s( buf, 128, "%g %g %g\r\n", intPt.x,intPt.y,intPt.z );
#endif
        s += buf;
#endif
    }

    fwrite(s.c_str(),s.length(), 1, file);

    fclose(file);

	QueryPerformanceCounter(&performanceCount);
	endCount = performanceCount.QuadPart;
	cout << "write time: " << (Scalar)(endCount - startCount) / freqency << endl;
	totalCount += endCount - startCount;
	cout << "total: " << (Scalar)(totalCount) / freqency << endl;

    // congratulations!!!
    // your program finished the intensive computation 
    // most importantly it survived and didn't crash. :)

	return 0;
}

#if !PARALLEL
void ComputeIntersections(Grid& grid, int numOfRays, Ray* rays, List<F3d>& intersectPoints)
{
#if MORE_OUTPUT 
	ofstream out;
	out.open("out_more.txt");
	int oldNumber = 0;
#endif
	for (int i = 0; i < numOfRays; ++i)
	{
		grid.IntersectRay(rays[i], intersectPoints);

#if MORE_OUTPUT
		int intNumber = intersectPoints.size() - oldNumber;
		oldNumber = intersectPoints.size();
		if (intNumber > 0)
		{
			out << i << " : " << intNumber << endl;
		}
#endif
	}

#if MORE_OUTPUT
	out.close();
#endif
}
#endif

void RayPreCompute(Ray& ray, const Grid& grid)
{
	// x-direction.
	if (fabs(ray.direction.x) < EPSILON)
	{
		ray.sign[0] = 0;
	}
	else
	{
		ray.invDirection.x = 1.0f / ray.direction.x;
		if (ray.direction.x > 0)
		{
			ray.sign[0] = 1;
			ray.dt[0] = grid.CellSize.x * ray.invDirection.x;
		}
		else
		{
			ray.sign[0] = -1;
			ray.dt[0] = -grid.CellSize.x * ray.invDirection.x;
		}
	}

	// y-direction.
	if (fabs(ray.direction.y) < EPSILON)
	{
		ray.sign[1] = 0;
	}
	else
	{
		ray.invDirection.y = 1.0f / ray.direction.y;
		if (ray.direction.y > 0)
		{
			ray.sign[1] = 1;
			ray.dt[1] = grid.CellSize.y * ray.invDirection.y;
		}
		else
		{
			ray.sign[1] = -1;
			ray.dt[1] = -grid.CellSize.y * ray.invDirection.y;
		}
	}

	// z-direction.
	if (fabs(ray.direction.z) < EPSILON)
	{
		ray.sign[2] = 0;
	}
	else
	{
		ray.invDirection.z = 1.0f / ray.direction.z;
		if (ray.direction.z > 0)
		{
			ray.sign[2] = 1;
			ray.dt[2] = grid.CellSize.z * ray.invDirection.z;
		}
		else
		{
			ray.sign[2] = -1;
			ray.dt[2] = -grid.CellSize.z * ray.invDirection.z;
		}
	}
}

void* FileRead(const char* filename, int interval, std::vector<char*>& outList, unsigned int& numberOfGeom)
{
    // query the size of the file
    struct _stat fInfo;
    _stat(filename, &fInfo);
    long buffersize = fInfo.st_size;
    char* pBuffer = new char[buffersize];

    // read all data into one buffer
    DWORD nNumberOfBytesRead;
    HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
    ReadFile(hFile,pBuffer,buffersize,&nNumberOfBytesRead,NULL);
    CloseHandle(hFile);

    //first get the first line to get the number of geom
    int i = 0;
    while(pBuffer[i]!='\n') {i++;}
    pBuffer[i] = '\0';
    numberOfGeom = atoi(pBuffer);

    // then get the points
    char* token = &pBuffer[i+1];
    char* pStart = token;

    // parse the points
    i = 0;
    int n = 0;
    int tempResult = interval - 1;
    while (token[i]!='\0')
    {
        if(token[i]=='\n')
        {
            if(n%interval == tempResult)
            {
                outList.push_back(pStart);
                pStart = &token[i+1];
                token[i] = '\0';
            }
            n++;
        }
        i++;
    }

    // to clean
    return pBuffer;
}