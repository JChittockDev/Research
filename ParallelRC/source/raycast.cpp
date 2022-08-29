// Copyright Joseph Chittock @2022

#include "raycast.h"

double dotProduct(const MVector& vectorA, const MVector& vectorB)
{
    return vectorA * vectorB;
}

MVector crossProduct(const MVector& vectorA, const MVector& vectorB)
{
    return vectorA ^ vectorB;
}

PRCTriangle::PRCTriangle(const MVector& a, const MVector& b, const MVector& c)
{
    N = crossProduct(b - a, c - a).normal();
    D = -dotProduct(N, a);

    N1 = crossProduct(N, b - a).normal();
    D1 = -dotProduct(N1, a);

    N2 = crossProduct(N, c - b).normal();
    D2 = -dotProduct(N2, b);

    N3 = crossProduct(N, a - c).normal();
    D3 = -dotProduct(N3, c);
}

bool PRCTriangle::Inside(const MVector& point)
{
    if(dotProduct(N1, point) + D1 < 0.0f) return false;
    if(dotProduct(N2, point) + D2 < 0.0f) return false;
    if(dotProduct(N3, point) + D3 < 0.0f) return false;

    return true;
}

bool PRCTriangle::Trace(const MVector& origin, const MVector& ray, float maxDistance, float& distance, MVector& point)
{
    float NdotR = -dotProduct(N, ray);

    distance = (dotProduct(N, origin) + D) / NdotR;
    if(distance >= 0.0f && distance < maxDistance)
    {
        point = ray * distance + origin;
        return Inside(point);
    }

    return false;
}

void PRCRayTracer::DetectIntersections(const MVector& origin, const MVector& ray, float maxDistance, PRCMesh *collider)
{
	int triangles = collider->meshData.size();
    #pragma omp parallel for private(data, triangle)
	for (int i = 0; i < triangles; i++) 
	{   
        PRCData data;
        PRCTriangle *triangle = &collider->meshData[i];
		if(triangle->Trace(origin, ray, maxDistance, data.distance, data.point))
		{
			intersectionData.push_back(data);
		}
	}
}

PRCMesh::PRCMesh(std::map<int, std::vector<MVector>>& data)
{
    Set(data);
};

void PRCMesh::Set(std::map<int, std::vector<MVector>>& data)
{
    int triangles = data.size();
    for (int i = 0; i < triangles; i++)
    {
        PRCTriangle triangle = PRCTriangle(data[i][0], data[i][1], data[i][2]);
        meshData[i] = triangle;
    }
};

PRCTriangle::PRCTriangle()
{
};

PRCData::PRCData()
{
};

PRCMesh::PRCMesh()
{
};