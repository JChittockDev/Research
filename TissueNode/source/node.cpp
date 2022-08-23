// Copyright Joseph Chittock @2022
// Custom Maya implementation of https://mmacklin.com/neohookean.pdf by Miles Macklin and Matthias Müller

#include "node.h"

// Base compute function for node
MStatus tissueNode::compute(const MPlug& plug, MDataBlock& data)
{
    // If there is an output mesh connected
    if (plug == output_mesh)
    {
        // Get all settings as variables from input handles
        float gravity = data.inputValue(input_gravity).asFloat();
        float drag = data.inputValue(input_drag).asFloat();
        int substeps = data.inputValue(input_substeps).asInt();
        float volume_compliance = data.inputValue(input_volume_compliance).asFloat();
        float edge_compliance = data.inputValue(input_edge_compliance).asFloat();
        float time_step = data.inputValue(input_timestep).asFloat();
        float radius_edge_ratio = data.inputValue(input_radius_edge_ratio).asFloat();
        float volume_constraint = data.inputValue(input_volume_constraint).asFloat();
        MTime time = data.inputValue(input_time).asTime();
        MDataHandle input_mesh_handle = data.inputValue(input_mesh);
        MDataHandle target_mesh_handle = data.inputValue(target_mesh);
        MDataHandle output_mesh_handle = data.outputValue(output_mesh);
        MDataHandle world_matrix_handle = data.inputValue(input_world_matrix);
        MDataHandle vertex_mask_handle = data.inputValue(input_vertex_mask_path);
        MMatrix input_matrix = world_matrix_handle.asMatrix();
        bool debug_mesh_diplsay = data.inputValue(input_debug_mesh_diplsay).asBool();
        MVector volume_cavity_pointer = data.inputValue(input_volume_cavity_pointer).asVector();
        std::string vertex_mask_path = vertex_mask_handle.asString().asChar();

        // Set the input & target objects as MFnMesh
        _input_object = input_mesh_handle.asMesh();
        _target_object = target_mesh_handle.asMesh();
        MFnMesh in_mesh(_input_object);
        MFnMesh target_mesh(_target_object);

        // Check if they exist, else dont compute
        if (_input_object.isNull() || _target_object.isNull())
        {
            return MS::kSuccess;
        }

        // Get their vertex positions
        in_mesh.getPoints(_input_vertex_positions, MSpace::kWorld);
        target_mesh.getPoints(_target_vertex_positions, MSpace::kWorld);

        // Flatten
        flattenPointArray(_input_vertex_positions, _flat_input_vertex_positions);

        // If node not initialized, or settings have changed
        if (!initialized || radius_edge_ratio != _radius_edge_ratio || volume_constraint != _volume_constraint)
        {
            initialized = true;
            // Update constant settings for next compute
            _radius_edge_ratio = radius_edge_ratio;
            _volume_constraint = volume_constraint;
            _init_target_vertex_positions = _target_vertex_positions;

            // Retrieve vertex mask
            getMask(vertex_mask_path, _vertex_mask);

            // Generate the tetrahedral version of the input mesh
            generateTetrahedralMesh(_input_object, _tet_object,
                                    input_matrix, radius_edge_ratio, volume_constraint, 
                                     _vertex_positions, _input_vertex_positions, _target_vertex_positions,
                                    _tet_mesh_surface_trigger, _closest_tetrahedral_baraycenter, 
                                     _init_vertex_positions, _previous_vertex_positions, 
                                    _tetrahedral_ids, _tetrahedral_edge_ids, _triangle_vertex_ids,
                                    _particle_count, _tet_count, _internal_ids, _external_ids, 
                                    _tet_internal_ids, in_mesh, volume_cavity_pointer);

            // Reset all data storage variables/arrays
            reset(_vertex_positions, _previous_vertex_positions, _flat_vertex_positions,
                _previous_flat_vertex_positions, _vertex_velocity, _inverse_mass, _rest_volume,
                _edge_lengths, _grads, _temp);
            
            // Intialize all data storage variables/arrays
            initData(_init_vertex_positions, _vertex_positions, _previous_vertex_positions,
                _flat_vertex_positions, _input_vertex_positions, _previous_flat_vertex_positions,
                _vertex_velocity, _inverse_mass, _rest_volume, _edge_lengths, _grads, _temp, 
                _tetrahedral_ids, _tetrahedral_edge_ids);
        }

        // Set the new tetrahedral mesh as a MFnMesh
        MFnMesh tet_mesh(_tet_object);

        // If the timeline time == 0 then again reset and initialize data storage variabels/arrays
        if (time == 0.0)
        {
            reset(_vertex_positions, _previous_vertex_positions, _flat_vertex_positions,
                _previous_flat_vertex_positions, _vertex_velocity, _inverse_mass, _rest_volume,
                _edge_lengths, _grads, _temp);

            initData(_init_vertex_positions, _vertex_positions, _previous_vertex_positions,
                _flat_vertex_positions, _input_vertex_positions, _previous_flat_vertex_positions,
                _vertex_velocity, _inverse_mass, _rest_volume, _edge_lengths, _grads, _temp,
                _tetrahedral_ids, _tetrahedral_edge_ids);
        }
        else 
        {
            // Otherwise, begin tetrahedral simulation
            float dt = time_step / substeps;
            simulate(time_step, substeps, gravity, drag, volume_compliance,
                    edge_compliance, _flat_vertex_positions,
                    _previous_flat_vertex_positions, _vertex_velocity,
                    _inverse_mass, _rest_volume, _tetrahedral_ids,
                    _tetrahedral_edge_ids, _edge_lengths, _grads,
                    _temp, _tet_mesh_surface_trigger, tet_mesh, _internal_ids, 
                    _tet_internal_ids, _flat_input_vertex_positions);
        }

        // Convert flat array to point array
        concatToPointArray(_flat_vertex_positions, _vertex_positions);
        MPointArray delta_positions;
        // Convert the transformations to relative
        subtractPointArray(_init_vertex_positions, _vertex_positions, delta_positions);

        // Transfer this relative position onto the initial vertex position
        MPointArray output_vertex_positions = _init_target_vertex_positions;
        transferPosition(delta_positions, output_vertex_positions,
            _closest_tetrahedral_baraycenter, _triangle_vertex_ids);

        // If the number of vertices matches the vert mask
        if (output_vertex_positions.length() == _vertex_mask.size())
        {
            // Use the vertex mask to dictate what areas recieve the tetrahedral simulation
            for (int i = 0; i < (int)output_vertex_positions.length(); ++i)
            {
                output_vertex_positions[i] = output_vertex_positions[i] * (1.0 - _vertex_mask[i]) + _target_vertex_positions[i] * _vertex_mask[i];
            }
        }

        // Set the visual output mesh's new positions
        tet_mesh.setPoints(_vertex_positions, MSpace::kWorld);
        target_mesh.setPoints(output_vertex_positions, MSpace::kWorld);

        // If debug mode is on, show the tetrahedral mesh, otherwise use the output target.
        if (debug_mesh_diplsay)
        {
            output_mesh_handle.set(_tet_object);
        }
        else
        {
            output_mesh_handle.set(_target_object);
        }
        data.setClean(plug);
    }
    else
    {
        return MS::kUnknownParameter;
    }
    
    return MS::kSuccess;
}

// Resets all data storage variables/arrays
void tissueNode::reset(MPointArray& vertex_positions, MPointArray& previous_vertex_positions, MFloatArray& flat_vertex_positions, 
    MFloatArray& previous_flat_vertex_positions, MFloatArray& vertex_velocity, MFloatArray& inverse_mass, MFloatArray& rest_volume, 
    MFloatArray& edge_lengths, MFloatArray& grads, MFloatArray& temp)
{
    vertex_positions.clear();
    previous_vertex_positions.clear();
    vertex_velocity.clear();
    flat_vertex_positions.clear();
    previous_flat_vertex_positions.clear();
    inverse_mass.clear();
    rest_volume.clear();
    edge_lengths.clear();
    grads.clear();
    temp.clear();
}

// Initializes all data storage variables/arrays
void tissueNode::initData(MPointArray& init_vertex_positions, MPointArray& vertex_positions, MPointArray& previous_vertex_positions,
    MFloatArray& flat_vertex_positions, MPointArray& input_vertex_positions,MFloatArray& previous_flat_vertex_positions,
    MFloatArray& vertex_velocity, MFloatArray& inverse_mass, MFloatArray& rest_volume, MFloatArray& edge_lengths, 
    MFloatArray& grads, MFloatArray& temp, MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids)
{
    vertex_positions = init_vertex_positions;
    previous_vertex_positions = init_vertex_positions;
    flattenPointArray(vertex_positions, flat_vertex_positions);
    flattenPointArray(previous_vertex_positions, previous_flat_vertex_positions);
    initArray(vertex_velocity, flat_vertex_positions.length(), 0.0);
    initArray(inverse_mass, _particle_count, 0.0);
    initArray(rest_volume, _tet_count, 0.0);
    edge_lengths.setLength(tetrahedral_ids.length() / 2);
    temp.setLength(12);
    grads.setLength(12);
    initPhysics( flat_vertex_positions,
        inverse_mass, tetrahedral_ids,
        tetrahedral_edge_ids, edge_lengths,
        temp, rest_volume);
}