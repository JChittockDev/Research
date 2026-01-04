// Copyright Joseph Chittock @2022

#include "node.h"

// Generate a tetrahedral mesh using Tetgen
void tissueNode::generateTetrahedralMesh(MObject& input_object, MObject& tet_object,
    MMatrix& input_matrix, float radius_edge_ratio, float volume_constraint,
    MPointArray& vertex_positions, MPointArray& input_vertex_positions, MPointArray& target_vertex_positions,
    std::vector<bool>& tet_mesh_surface_trigger, MFloatArray& closest_tetrahedral_baraycenter,
    MPointArray& init_vertex_positions, MPointArray& previous_vertex_positions,
    MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids,
    std::vector<std::vector<MIntArray>>& triangle_vertex_ids,
    int& particle_count, int& tet_count, std::vector<int>& internal_ids,
    std::vector<int>& external_ids, std::vector<int>& tet_internal_ids,
    MFnMesh& in_mesh, MVector& hole)
{
    // Generate MFnMeshData object
    MFnMeshData tet_mesh_data_creator;
    tet_object = tet_mesh_data_creator.create();
    // Fill this object with a tet mesh from Tetgen
    toTetMesh(input_object, tet_object, input_matrix, radius_edge_ratio, volume_constraint, hole);
    // Convert this to an MFnMesh
    MFnMesh tet_mesh(tet_object);
    // Get its vertex positions
    tet_mesh.getPoints(vertex_positions, MSpace::kWorld);

    init_vertex_positions = vertex_positions;
    previous_vertex_positions = vertex_positions;

    // Get data to use in simulation stages
    findInternalPoints(in_mesh, input_vertex_positions, 99999999, 0.001, internal_ids, external_ids);
    findTetInternalIds(input_vertex_positions, init_vertex_positions, internal_ids, tet_internal_ids);
    findTetSurfaceIds(input_object, vertex_positions, external_ids, tet_mesh_surface_trigger);
    findClosestBaraycenter(target_vertex_positions, closest_tetrahedral_baraycenter, tet_object);
    getTetIds(tetrahedral_ids);
    getTetEdgeIds(tetrahedral_edge_ids, vertex_positions.length(), tet_object);
    getTriangleVertices(triangle_vertex_ids, tet_object);
    particle_count = vertex_positions.length();
    tet_count = tetrahedral_ids.length() / 4;
}

// Get input object data to feed to Tetgen
void tissueNode::toTetgenIO(MObject& input_mesh, MMatrix& wmat, MVector& hole)
{
    // Convert object to MFnMesh
    MFnMesh input_mesh_fn(input_mesh);

    int vertex_count = input_mesh_fn.numVertices();
    MPointArray vertex_list;

    // Get vertex positions
    input_mesh_fn.getPoints(vertex_list);

    tetgenio::facet* f;
    tetgenio::polygon* p;

    // Set Tetgen data inputs
    in.firstnumber = 1;
    in.numberofpoints = vertex_count;
    in.pointlist = new REAL[in.numberofpoints * 3];

    int point_list_index = 0;
    for (int vertexListIndex = 0; vertexListIndex < vertex_count; vertexListIndex++)
    {
        MPoint mpoint = vertex_list[vertexListIndex] * wmat;
        in.pointlist[point_list_index] = mpoint.x; point_list_index++;
        in.pointlist[point_list_index] = mpoint.y; point_list_index++;
        in.pointlist[point_list_index] = mpoint.z; point_list_index++;
    }

    MIntArray facet_vertex_count;
    MIntArray facet_vertex_index_list;

    // Get vertex indicies
    input_mesh_fn.getVertices(facet_vertex_count, facet_vertex_index_list);

    int facet_count = input_mesh_fn.numPolygons();

    in.numberoffacets = facet_count;
    in.facetlist = new tetgenio::facet[in.numberoffacets];

    // Set the Tetgen data for each mesh facet
    for (int facet_index = 0; facet_index < facet_count; facet_index++)
    {
        f = &in.facetlist[facet_index];
        f->numberofpolygons = 1;
        f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
        p = &f->polygonlist[0];
        p->numberofvertices = facet_vertex_count[facet_index];
        p->vertexlist = new int[p->numberofvertices];
        f->numberofholes = 0;
        f->holelist = NULL;

        MIntArray cur_facet_vertex_indices;
        input_mesh_fn.getPolygonVertices(facet_index, cur_facet_vertex_indices);
        for (int facet_vertex_index = 0; facet_vertex_index < p->numberofvertices; facet_vertex_index++)
        {
            p->vertexlist[facet_vertex_index] = cur_facet_vertex_indices[facet_vertex_index] + 1;
        }
    }

    // Define where interior holes in the mesh are (hard coded to one at this stage)
    std::vector<std::vector<REAL>> holes = { {hole[0],hole[1],hole[0]} };

    in.numberofholes = holes.size();
    in.holelist = new double[3 * in.numberofholes];

    for (int holeID = 0, nHoles = holes.size(); holeID < nHoles; holeID++)
    {
        in.holelist[holeID * 3 + 0] = holes[holeID][0];
        in.holelist[holeID * 3 + 1] = holes[holeID][1];
        in.holelist[holeID * 3 + 2] = holes[holeID][2];
    }
}

// Output Tetgen mesh to maya object
void tissueNode::toOutTetMesh(MObject& output_mesh)
{
    // Get output tet vertex data
    int	vertex_count = out.numberofpoints;
    MPointArray points;
    MFnMesh	mesh_fs;
    REAL* tetpoints = out.pointlist;

    for (int i = 0; i < vertex_count * 3; i++)
    {
        double x = tetpoints[i]; i++;
        double y = tetpoints[i]; i++;
        double z = tetpoints[i];
        MPoint point(x, y, z);
        points.append(point);
    }

    // Get output tet face data
    const int face_count = out.numberoftetrahedra * 4;
    int* face_counts = new int[face_count];

    for (int i = 0; i < face_count; i++)
    {
        face_counts[i] = 3;
    }

    MIntArray face_count_array(face_counts, face_count);
    int face_connect_count = face_count * 3;
    int* face_connects = new int[face_connect_count];
    int tet_vertex_index = 0;

    // Get output tet index data
    for (int i = 0; i < face_connect_count; i++)
    {
        int index1 = out.tetrahedronlist[tet_vertex_index]; tet_vertex_index++;
        int index2 = out.tetrahedronlist[tet_vertex_index]; tet_vertex_index++;
        int index3 = out.tetrahedronlist[tet_vertex_index]; tet_vertex_index++;
        int index4 = out.tetrahedronlist[tet_vertex_index]; tet_vertex_index++;

        face_connects[i] = index1 - 1; i++;
        face_connects[i] = index3 - 1; i++;
        face_connects[i] = index2 - 1; i++;
        face_connects[i] = index1 - 1; i++;
        face_connects[i] = index2 - 1; i++;
        face_connects[i] = index4 - 1; i++;
        face_connects[i] = index1 - 1; i++;
        face_connects[i] = index4 - 1; i++;
        face_connects[i] = index3 - 1; i++;
        face_connects[i] = index4 - 1; i++;
        face_connects[i] = index2 - 1; i++;
        face_connects[i] = index3 - 1;
    }

    // Create a new mesh with the corresponding data points
    MIntArray face_connects_array(face_connects, face_connect_count);
    MObject newMesh = mesh_fs.create(vertex_count, face_count,
        points, face_count_array, face_connects_array,
        output_mesh);
}

void tissueNode::toTetMesh(MObject& input_mesh, MObject& output_mesh, MMatrix& wmat, double radius_edge_ratio, double volume_constraint, MVector& hole)
{
    // Create Tetgen compatible data structure
    toTetgenIO(input_mesh, wmat, hole);
    // Convert this to a string command
    std::ostringstream strs;
    strs << radius_edge_ratio;
    std::string radius_edge_ratio_string = strs.str();
    strs.clear();
    strs.seekp(0);
    strs << volume_constraint;
    std::string volume_constraint_string = strs.str();
    std::string cmd = "pq" + radius_edge_ratio_string + "a" + volume_constraint_string;
    char* c_cmd = new char[cmd.size() + 1];
    std::copy(cmd.begin(), cmd.end(), c_cmd);
    c_cmd[cmd.size()] = '\0';
    // Perform terahedralization
    tetrahedralize(c_cmd, &in, &out);
    // Output as a Maya object
    toOutTetMesh(output_mesh);
    delete[] c_cmd;
}

// Retrieve tetrahedral ids
void tissueNode::getTetIds(MIntArray& input)
{
    input.clear();
    int numTets = out.numberoftetrahedra;
    input.clear();
    for (int i = 0; i < numTets * 4; i++)
    {
        int index = out.tetrahedronlist[i];
        input.append(index - 1);
    }
}

// Retrieve the vertex ids for every edge connected via triangle
void tissueNode::getTetEdgeIds(MIntArray& input, int count, MObject& input_mesh)
{
    input.clear();
    MItMeshVertex vertex_iterator(input_mesh);
    // For each vertex
    for (; !vertex_iterator.isDone(); vertex_iterator.next())
    {
        int i = vertex_iterator.index();
        MIntArray structural_vertices;
        // Get the connected vertices (by edge)
        vertex_iterator.getConnectedVertices(structural_vertices);
        for (int x = 0; x < (int)structural_vertices.length(); ++x)
        {
            // store these ids
            input.append(i);
            input.append(structural_vertices[x]);
        }
    }
}

// Find the points on tetrahedral mesh that are closest to the internal points of target mesh
void tissueNode::findTetInternalIds(MPointArray& input_points, MPointArray& tet_points, std::vector<int>& internal_ids, std::vector<int>& tet_ids)
{
    tet_ids.clear();
    // For each target internal ID
    for (int x = 0; x < (int)internal_ids.size(); ++x)
    {
        MPoint target_point = input_points[internal_ids[x]];
        float closest_distance = 99999999.9999;
        int closest_index = 0;
        for (int y = 0; y < (int)tet_points.length(); ++y)
        {
            // determine the distance to every tetrahedral point
            MVector delta = MVector(target_point[0] - tet_points[y][0], target_point[1] - tet_points[y][1], target_point[2] - tet_points[y][2]);
            // if it is closest, store this id and distance
            float distance = abs(delta.length());
            if (distance <= closest_distance)
            {
                closest_distance = distance;
                closest_index = y;
            }
        }
        tet_ids.push_back(closest_index);
    }
}
// Find the points on tetrahedral mesh that are closest to the external points of target mesh

void tissueNode::findTetSurfaceIds(MObject& mesh, MPointArray& input_points, std::vector<int>& search_ids, std::vector<bool>& ids)
{
    ids.clear();
    MMeshIntersector intersector;
    intersector.create(mesh);

    // For each input point
    for (int x = 0; x < (int)input_points.length(); ++x)
    {
        MPointOnMesh closest_point;
        // Use an intersector to find the closest point
        intersector.getClosestPoint(input_points[x], closest_point);
        MFloatPoint point = closest_point.getPoint();

        // Get the distance between these points
        MVector vector_point = MVector(point[0] - input_points[x][0], point[1] - input_points[x][1], point[2] - input_points[x][2]);
        float distance = vector_point.length();

        // If the distance is below the threshold, and it is external, then store this id.
        if (distance <= 0.0001)
        {
            if (contains(search_ids, x))
            {
                ids.push_back(true);
            }
            else
            {
                ids.push_back(false);
            }
        }
        else
        {
            ids.push_back(false);
        }
    }
}