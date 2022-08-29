#include "node.h"

std::map<int, std::vector<MVector>> collisionDeformer::getMeshData(MObject& mesh)
{
    std::map<int, std::vector<MVector>> output;
    MItMeshPolygon iterator(mesh);

    int index = 0;
    for (; !iterator.isDone(); iterator.next())
    {
        int tri_count;
        iterator.numTriangles(tri_count);
        for (int x = 0; x < tri_count; ++x)
        {
            MPointArray points;
            MIntArray vertices;
            std::vector<MVector> triangle_positions;
            iterator.getTriangle(x, points, vertices, MSpace::kWorld);
            for (int y = 0; y < points.length(); ++y)
            {
                triangle_positions.push_back(MVector(points[y][0], points[y][1], points[y][2]));
            }
            output[index] = triangle_positions;
            index += 1;
        }
    }
    return output;
}