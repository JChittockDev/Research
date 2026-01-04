// Copyright Joseph Chittock @2022

#include "node.h"

// Find the baraycentric coordinate that is closest to each input point
void tissueNode::findClosestBaraycenter(MPointArray& input_points, MFloatArray& closest_baraycenter, MObject& mesh)
{
    closest_baraycenter.clear();
    MMeshIntersector intersector;
    intersector.create(mesh);

    for (int i = 0; i < (int)input_points.length(); ++i)
    {
        float u, v;
        MPointOnMesh closest_point;
        intersector.getClosestPoint(input_points[i], closest_point);
        closest_point.getBarycentricCoords(u, v);
        int tri_index = closest_point.triangleIndex();
        int face_index = closest_point.faceIndex();
        closest_baraycenter.append(face_index);
        closest_baraycenter.append(tri_index);
        closest_baraycenter.append(u);
        closest_baraycenter.append(v);
    }
}

// Get the the vertex indicies of every triangle on an input mesh
void tissueNode::getTriangleVertices(std::vector<std::vector<MIntArray>>& output, MObject& mesh)
{
    output.clear();
    MItMeshPolygon poly_iterator(mesh);
    for (; !poly_iterator.isDone(); poly_iterator.next())
    {
        int tri_count;
        poly_iterator.numTriangles(tri_count);
        std::vector<MIntArray> triangle_vertices;
        // For each triangle
        for (int x = 0; x < tri_count; ++x)
        {
            MPointArray points;
            MIntArray vertices;
            // Retrieve triangle data
            poly_iterator.getTriangle(x, points, vertices, MSpace::kWorld);
            // Store triangles vertex indicies
            triangle_vertices.push_back(vertices);
        }
        output.push_back(triangle_vertices);
    }
}

// Transfer position based on the closest baraycentric coordinates
void tissueNode::transferPosition(MPointArray& input_points, MPointArray& output_points,
    MFloatArray& closest_baraycenter_points, std::vector<std::vector<MIntArray>>& triangle_vertices)
{
    for (int x = 0; x < (int)output_points.length(); ++x)
    {
        int face_index = closest_baraycenter_points[x * 4];
        int triangle_index = closest_baraycenter_points[x * 4 + 1];
        float u = closest_baraycenter_points[x * 4 + 2];
        float v = closest_baraycenter_points[x * 4 + 3];
        MIntArray triangle_verts = triangle_vertices[face_index][triangle_index];
        output_points[x] += u * (input_points[triangle_verts[0]]) + v * (input_points[triangle_verts[1]]) + (1 - u - v) * (input_points[triangle_verts[2]]);
    }
}

// Override the internal verticies of tetrahedral mesh to follow the input objects motion
void tissueNode::overrideInternalDeformation(MFloatArray& input_points, MFloatArray& output_points, MFloatArray& inverse_mass,
    std::vector<int>& ids, std::vector<int>& tet_ids)
{
    for (int x = 0; x < (int)ids.size(); ++x)
    {
        output_points[tet_ids[x] * 3] = input_points[ids[x] * 3];
        output_points[tet_ids[x] * 3 + 1] = input_points[ids[x] * 3 + 1];
        output_points[tet_ids[x] * 3 + 2] = input_points[ids[x] * 3 + 2];
        inverse_mass[tet_ids[x]] = 0.0;
    }
}

// Find the input objects internal points via raycasting
void tissueNode::findInternalPoints(MFnMesh& mesh, MPointArray& input_points, const float collision_range_value,
    const float collision_tolerance_range, std::vector<int>& internal_ids, std::vector<int>& external_ids)
{
    internal_ids.clear();
    external_ids.clear();
    MMeshIsectAccelParams accel_params = mesh.autoUniformGridParams();

    // For each vertex
    for (int x = 0; x < (int)input_points.length(); ++x)
    {
        int hits = 0;
        // Perform a raycast
        raycast(mesh, x, accel_params, input_points[x], collision_range_value, collision_tolerance_range, hits);
        std::cout << "vertex: " << x << " hits: " << hits << "\n";
        // If the total hits are even, the result is internal
        if (hits % 2 == 1)
        {
            internal_ids.push_back(x);
        }
        else
        {
            external_ids.push_back(x);
        }
    }
}

// Compute a raycast, given a normal direction and position
void tissueNode::raycast(MFnMesh& mesh, const int index, MMeshIsectAccelParams& accel_params, MPoint& point,
    const float collision_range_value, const float collision_tolerance_range, int& hit_count)
{
    MVector normal;
    MVector hit_normal;
    MFloatPointArray hit_points_a;
    MFloatPointArray hit_points_b;
    MFloatArray ray_params_a;
    MFloatArray ray_params_b;
    MIntArray hit_faces;

    // Get input mesh normal
    mesh.getVertexNormal(index, true, normal, MSpace::kObject);

    // Perform an accelerated raycast
    bool intersect_a = mesh.allIntersections(
        point, normal, NULL, NULL, true, MSpace::kObject,
        collision_range_value, false, &accel_params, true, hit_points_a,
        &ray_params_a, &hit_faces, NULL, NULL, NULL, collision_tolerance_range);

    // If there is a hit, store it
    if (intersect_a) {
        hit_count = (hit_points_a.length() - 1);
    }
}

// Retrieve mask via Fstream
void tissueNode::getMask(std::string path, std::vector<float>& mask)
{
    std::fstream myFile(path);
    if (myFile.is_open())
    {
        std::string line;
        while (getline(myFile, line))
        {
            mask.push_back(std::stof(line));
        }
    }
}