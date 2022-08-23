// Copyright Joseph Chittock @2022

#include "node.h"

// Get the volume of the tetdrahedral mesh
float tissueNode::getTetVolume(int tet_index, MIntArray& tetrahedral_ids, MFloatArray& temp, MFloatArray& vertex_positions)
{
    // Get the tet ids
    int id0 = tetrahedral_ids[4 * tet_index];
    int id1 = tetrahedral_ids[4 * tet_index + 1];
    int id2 = tetrahedral_ids[4 * tet_index + 2];
    int id3 = tetrahedral_ids[4 * tet_index + 3];
    // Calculate their differences
    vecSetDiff(temp, 0, vertex_positions, id1, vertex_positions, id0, 1.0);
    vecSetDiff(temp, 1, vertex_positions, id2, vertex_positions, id0, 1.0);
    vecSetDiff(temp, 2, vertex_positions, id3, vertex_positions, id0, 1.0);
    // Compute their volume via cross & dot product
    vecSetCross(temp, 3, temp, 0, temp, 1);
    return vecDot(temp, 3, temp, 2) / 6.0;
}

// Initialize the physics simulation
void tissueNode::initPhysics(MFloatArray& vertex_positions, MFloatArray& inverse_mass, MIntArray& tetrahedral_ids,
    MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths, MFloatArray& temp, MFloatArray& rest_volume)
{

    // For each tetrahedron
    for (int i = 0; i < (int)_tet_count; ++i)
    {
        // Get the resting mesh volume
        rest_volume[i] = getTetVolume(i, tetrahedral_ids, temp, vertex_positions);
        // Calculate the inverse mass 
        float particle_inverse_mass = rest_volume[i] > 0.0 ? 1.0 / (rest_volume[i] / 4.0) : 0.0;
        inverse_mass[tetrahedral_ids[4 * i]] += particle_inverse_mass;
        inverse_mass[tetrahedral_ids[4 * i + 1]] += particle_inverse_mass;
        inverse_mass[tetrahedral_ids[4 * i + 2]] += particle_inverse_mass;
        inverse_mass[tetrahedral_ids[4 * i + 3]] += particle_inverse_mass;
    }

    // for each edge, calculate its euclidean length
    for (int i = 0; i < (int)edge_lengths.length(); ++i)
    {
        int id0 = tetrahedral_edge_ids[2 * i];
        int id1 = tetrahedral_edge_ids[2 * i + 1];
        edge_lengths[i] = sqrt(vecDistSquared(vertex_positions, id0, vertex_positions, id1));
    }

}

// Performance a pre solve to prepare physics measurment variables
void tissueNode::preSolve(float dt, MFloatArray& gravity, MFloatArray& vertex_positions, MFloatArray& previous_vertex_positions,
    MFloatArray& inverse_mass, MFloatArray& vertex_velocity, float drag, std::vector<bool> tet_mesh_surface_trigger, MFnMesh& mesh)
{
    for (int i = 0; i < _particle_count; i++) {
        if (inverse_mass[i] == 0.0)
            continue;

        // Add gravity to total velocity
        vecAdd(vertex_velocity, i, gravity, 0, dt);
        // If current vertex is on the surface, perform drag as velocity
        if (tet_mesh_surface_trigger[i])
        {
            dragResist(vertex_velocity, i, drag, mesh);
        }
        // Store the vertex positions for the next computation cycle
        vecCopy(previous_vertex_positions, i, vertex_positions, i);
        // Add the velocity to the current vertex positions, as a pre solve target
        vecAdd(vertex_positions, i, vertex_velocity, i, dt);

        // Dirty prediction if simulation is hitting the ground plane
        float y = vertex_positions[3 * i + 1];
        if (y < 0.0)
        {
            vecCopy(vertex_positions, i, previous_vertex_positions, i);
            vertex_positions[3 * i + 1] = 0.0;
        }
    }
}

// Function to add drag as a velocity sum vector
void tissueNode::dragResist(MFloatArray& vertex_velocity, int i, float drag, MFnMesh& mesh)
{
    // Get the vertex normal
    MVector normal;
    mesh.getVertexNormal(i, true, normal, MSpace::kObject);

    i *= 3;
    // Get the current velocity
    MVector velocity = MVector(vertex_velocity[i] * drag,
        vertex_velocity[i + 1] * drag,
        vertex_velocity[i + 2] * drag);

    // Create a weight control to determine if the inverse normal velocity is facing the inverse of the normal
    float aim_weight = std::clamp((-1.0 * velocity.normal()) * -normal, 0.0, 1.0);

    // Add the inverse velocity multiplied by this weight to the vertex velocity
    vertex_velocity[i] += (-1.0 * velocity[0]) * aim_weight;
    vertex_velocity[i + 1] += (-1.0 * velocity[1]) * aim_weight;
    vertex_velocity[i + 2] += (-1.0 * velocity[2]) * aim_weight;
}

// Perform a post solve, this finds the difference between the previous and 
// current vertex positions and stores it as the initial velocity for the next computation cycle
void tissueNode::postSolve(float dt, MFloatArray& inverse_mass, MFloatArray& vertex_velocity, MFloatArray& vertex_positions,
    MFloatArray& previous_vertex_positions)
{
    for (int i = 0; i < _particle_count; i++) {
        if (inverse_mass[i] == 0.0)
            continue;
        vecSetDiff(vertex_velocity, i, vertex_positions, i, previous_vertex_positions, i, 1.0 / dt);
    }
}

// Solves the loss in elastic integrity of edges
void tissueNode::solveEdges(float edge_compliance, float dt, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths,
    MFloatArray& inverse_mass, MFloatArray& vertex_positions, MFloatArray& grads)
{
    // Create edge compliance weight
    float alpha = edge_compliance / dt / dt;
    // For each edge
    for (int i = 0; i < edge_lengths.length(); i++)
    {
        // Get its relevant data
        int id0 = tetrahedral_edge_ids[2 * i];
        int id1 = tetrahedral_edge_ids[2 * i + 1];
        float w0 = inverse_mass[id0];
        float w1 = inverse_mass[id1];
        float w = w0 + w1;

        if (w == 0.0)
            continue;

        // find the euclidean distance of the edge
        vecSetDiff(grads, 0, vertex_positions, id0, vertex_positions, id1, 1.0);
        float len = sqrt(vecLengthSquared(grads, 0));

        if (len == 0.0)
            continue;

        // Determine the correction to conform the edge to compliance
        vecScale(grads, 0, 1.0 / len);
        float restLen = edge_lengths[i];
        float C = len - restLen;
        float s = -C / (w + alpha);
        vecAdd(vertex_positions, id0, grads, 0, s * w0);
        vecAdd(vertex_positions, id1, grads, 0, -s * w1);
    }
}

// Solves the loss in volume integrity of tetrahedra
void tissueNode::solveVolumes(float volume_compliance, float dt, MFloatArray grads, MFloatArray& vertex_positions,
    MFloatArray& vertex_velocity, MFloatArray& temp, MIntArray& tetrahedral_ids, MFloatArray& inverse_mass,
    MFloatArray& rest_volume)
{
    std::vector<std::vector<int>> volume_id_order = { {1, 3, 2}, { 0, 2, 3 }, { 0, 3, 1 }, { 0, 1, 2 } };

    // Create volume compliance weight
    float alpha = volume_compliance / dt / dt;
    // For each tetrahedra
    for (int i = 0; i < _tet_count; i++) {

        float w = 0.0;
        // Get the following tetrahderal point positions corresponding with the volume id order
        for (int j = 0; j < 4; j++) {
            int id0 = tetrahedral_ids[4 * i + volume_id_order[j][0]];
            int id1 = tetrahedral_ids[4 * i + volume_id_order[j][1]];
            int id2 = tetrahedral_ids[4 * i + volume_id_order[j][2]];

            // find the difference between the given verticies
            vecSetDiff(temp, 0, vertex_positions, id1, vertex_positions, id0, 1.0);
            vecSetDiff(temp, 1, vertex_positions, id2, vertex_positions, id0, 1.0);
            vecSetCross(grads, j, temp, 0, temp, 1);
            vecScale(grads, j, 1.0 / 6.0);

            w += inverse_mass[tetrahedral_ids[4 * i + j]] * vecLengthSquared(grads, j);
        }

        if (w == 0.0)
            continue;

        // Determine the rest volume & the correction to vertex positions given the loss/gain in volume
        float vol = getTetVolume(i, tetrahedral_ids, temp, vertex_positions);
        float restVol = rest_volume[i];
        float C = vol - restVol;
        float s = -C / (w + alpha);

        for (int j = 0; j < 4; j++) {
            int id = tetrahedral_ids[4 * i + j];
            vecAdd(vertex_positions, id, grads, j, s * inverse_mass[id]);
        }
    }
}

// Function to solves both the edges and volume concurrently
void tissueNode::solve(float dt, float volume_compliance, float edge_compliance, MFloatArray& vertex_positions, MFloatArray& vertex_velocity,
    MFloatArray& inverse_mass, MFloatArray& rest_volume, MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths,
    MFloatArray& grads, MFloatArray& temp)
{
    solveEdges(edge_compliance, dt, tetrahedral_edge_ids, edge_lengths, inverse_mass, vertex_positions, grads);
    solveVolumes(volume_compliance, dt, grads, vertex_positions, vertex_velocity, temp, tetrahedral_ids, inverse_mass, rest_volume);
}

// Wrapper function to complete the full simulation cycle for the given frame
void tissueNode::simulate(float dt, int substeps, float gravity, float drag, float volume_compliance, float edge_compliance,
    MFloatArray& vertex_positions, MFloatArray& previous_vertex_positions, MFloatArray& vertex_velocity, MFloatArray& inverse_mass,
    MFloatArray& rest_volume, MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths,
    MFloatArray& grads, MFloatArray& temp, std::vector<bool> tet_mesh_surface_trigger, MFnMesh& mesh, std::vector<int>& internal_ids,
    std::vector<int>& tet_internal_ids, MFloatArray& input_points)
{

    float sdt = dt / substeps;

    MFloatArray gravity_array;
    gravity_array.append(0.0);
    gravity_array.append(gravity);
    gravity_array.append(0.0);

    for (int step = 0; step < substeps; step++)
    {
        preSolve(sdt, gravity_array, vertex_positions, previous_vertex_positions, inverse_mass, vertex_velocity, drag,
            tet_mesh_surface_trigger, mesh);
        solve(sdt, volume_compliance, edge_compliance, vertex_positions, vertex_velocity, inverse_mass, rest_volume,
            tetrahedral_ids, tetrahedral_edge_ids, edge_lengths, grads, temp);
        overrideInternalDeformation(input_points, vertex_positions, inverse_mass, internal_ids, tet_internal_ids);
        postSolve(sdt, inverse_mass, vertex_velocity, vertex_positions, previous_vertex_positions);
    }
}