// Copyright Joseph Chittock @2022

#include "common.h"

class PRCTriangle
{
public:
    float D, D1, D2, D3;
    MVector a, b, c, N, N1, N2, N3;

public:
    PRCTriangle();
    PRCTriangle(const MVector& a, const MVector& b, const MVector& c);
    bool Inside(const MVector& point);
    bool Trace(const MVector& origin, const MVector& ray, float maxDistance, float& distance, MVector& point);
};

class PRCMesh
{
public:
    PRCMesh();
    PRCMesh(std::map<int, std::vector<MVector>>& data);
    void Set(std::map<int, std::vector<MVector>>& data);
public:
    std::map<int, PRCTriangle> meshData;
};

class PRCData
{
public:
    float distance;
    MVector point;
    PRCTriangle *triangle;

public:
    PRCData();
};

class PRCRayTracer
{
public:
    void DetectIntersections(const MVector& origin, const MVector& ray, float maxDistance, PRCMesh* collider);
	std::vector<PRCData> intersectionData;
};