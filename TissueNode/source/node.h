// Copyright Joseph Chittock @2022

#include "common.h"

// Class implementation for node
class tissueNode : public MPxNode
{
public:
    tissueNode() {};
    virtual MStatus compute(const MPlug& plug, MDataBlock& data);
    static  void* creator();
    static  MStatus initialize();

public:
    // Static class variables
    static const MTypeId id;
    static MObject input_world_matrix;
    static MObject input_gravity;
    static MObject input_drag;
    static MObject input_mesh;
    static MObject target_mesh;
    static MObject output_mesh;
    static MObject input_time;
    static MObject input_substeps;
    static MObject input_edge_compliance;
    static MObject input_volume_compliance;
    static MObject input_timestep;
    static MObject input_density;
    static MObject input_radius_edge_ratio;
    static MObject input_volume_constraint;
    static MObject input_volume_cavity_pointer;
    static MObject input_debug_mesh_diplsay;
    static MObject input_vertex_mask_path;

private:
    // Default data flow functions
    void reset(MPointArray& vertex_positions, MPointArray& previous_vertex_positions, MFloatArray& flat_vertex_positions,
        MFloatArray& previous_flat_vertex_positions, MFloatArray& vertex_velocity, MFloatArray& inverse_mass, MFloatArray& rest_volume,
        MFloatArray& edge_lengths, MFloatArray& grads, MFloatArray& temp);

    void initData(MPointArray& init_vertex_positions, MPointArray& vertex_positions, MPointArray& previous_vertex_positions,
        MFloatArray& flat_vertex_positions, MPointArray& input_vertex_positions,
        MFloatArray& previous_flat_vertex_positions, MFloatArray& vertex_velocity, MFloatArray& inverse_mass,
        MFloatArray& rest_volume, MFloatArray& edge_lengths, MFloatArray& grads, MFloatArray& temp, MIntArray& tetrahedral_ids,
        MIntArray& tetrahedral_edge_ids);

    // Physics related functions
    void preSolve(float dt, MFloatArray& gravity, MFloatArray& vertex_positions, MFloatArray& previous_vertex_positions,
        MFloatArray& inverse_mass, MFloatArray& vertex_velocity, float drag, std::vector<bool> tet_mesh_surface_trigger, MFnMesh& mesh);
    void initPhysics(MFloatArray& vertex_positions, MFloatArray& inverse_mass,
        MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths,
        MFloatArray& temp, MFloatArray& rest_volume);
    void postSolve(float dt, MFloatArray& inverse_mass, MFloatArray& vertex_velocity, MFloatArray& vertex_positions, 
        MFloatArray& previous_vertex_positions);
    void solveEdges(float edge_compliance, float dt, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths, MFloatArray& inverse_mass, 
        MFloatArray& vertex_positions, MFloatArray& grads);
    void solveVolumes(float volume_compliance, float dt, MFloatArray grads, MFloatArray& vertex_positions, MFloatArray& vertex_velocity, 
        MFloatArray& temp, MIntArray& tetrahedral_ids, MFloatArray& inverse_mass, MFloatArray& rest_volume);
    void solve(float dt, float volume_compliance, float edge_compliance, MFloatArray& vertex_positions, MFloatArray& vertex_velocity, 
        MFloatArray& inverse_mass, MFloatArray& rest_volume, MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids, 
        MFloatArray& edge_lengths, MFloatArray& grads, MFloatArray& temp);
    void simulate(float dt, int substeps, float gravity, float drag, float volume_compliance, float edge_compliance, MFloatArray& vertex_positions, 
        MFloatArray& previous_vertex_positions, MFloatArray& vertex_velocity, MFloatArray& inverse_mass, MFloatArray& rest_volume, 
        MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids, MFloatArray& edge_lengths, MFloatArray& grads, MFloatArray& temp,
        std::vector<bool> tet_mesh_surface_trigger, MFnMesh& mesh, std::vector<int>& internal_ids, std::vector<int>& internal_tet_ids,
        MFloatArray& driver_points);
    void dragResist(MFloatArray& vertex_velocity, int i, float drag, MFnMesh& mesh);

    // Tetgen interface fucntions
    void generateTetrahedralMesh(MObject& input_object, MObject& tet_object,
        MMatrix& input_matrix, float radius_edge_ratio, float volume_constraint,
        MPointArray& vertex_positions, MPointArray& input_vertex_positions, MPointArray& target_vertex_positions,
        std::vector<bool>& tet_mesh_surface_trigger, MFloatArray& closest_tetrahedral_baraycenter,
        MPointArray& init_vertex_positions, MPointArray& previous_vertex_positions,
        MIntArray& tetrahedral_ids, MIntArray& tetrahedral_edge_ids,
        std::vector<std::vector<MIntArray>>& triangle_vertex_ids,
        int& particle_count, int& tet_count, std::vector<int>& internal_ids,
        std::vector<int>& external_ids, std::vector<int>& tet_internal_ids,
        MFnMesh& in_mesh, MVector& hole);
    void toTetMesh(MObject& input_mesh, MObject& output_mesh, MMatrix& wmat, 
        double radius_edge_ratio, double volume_constraint, MVector& hole);
    void toTetgenIO(MObject& input_mesh, MMatrix& wmat, MVector& hole);
    void toOutTetMesh(MObject& output_mesh);

    // Tetgen utility functions 
    void getTetIds(MIntArray& input);
    void getTetEdgeIds(MIntArray& input, int count, MObject& input_mesh);
    float getTetVolume(int tet_index, MIntArray& tetrahedral_ids, MFloatArray& temp, MFloatArray& vertex_positions);
    void findTetSurfaceIds(MObject& mesh, MPointArray& input_points, std::vector<int>& search_ids,
        std::vector<bool>& ids);
    void findTetInternalIds(MPointArray& input_points, MPointArray& tet_points, std::vector<int>& ids,
        std::vector<int>& tet_ids);
    
    // Maya C++ api utility functions
    void flattenPointArray(const MPointArray& input, MFloatArray& output);
    void concatToPointArray(const MFloatArray& input, MPointArray& output);
    void initArray(MFloatArray& input, int length, float value);
    void subtractPointArray(MPointArray& init_points, MPointArray& input_points, MPointArray& output_points);
    int contains(std::vector<int>& vec, const int elem);

    // Math functions
    float vecDot(MFloatArray& a, int anr, MFloatArray& b, int bnr);
    float vecLengthSquared(MFloatArray& a, int anr);
    float vecDistSquared(MFloatArray& a, int anr, MFloatArray& b, int bnr);
    void vecSetCross(MFloatArray& a, int anr, MFloatArray& b, int bnr, MFloatArray& c, int cnr);
    void vecSetDiff(MFloatArray& dst, int dnr, MFloatArray& a, int anr, MFloatArray& b, int bnr, float scale);
    void vecCopy(MFloatArray& a, int anr, const MFloatArray& b, int bnr);
    void vecAdd(MFloatArray& a, int anr, MFloatArray& b, int bnr, float scale);
    void vecScale(MFloatArray& a, int anr, float scale);

    // Deformation transfer functions
    void findClosestBaraycenter(MPointArray& input_points, MFloatArray& closest_baraycenter, MObject& mesh);
    void getTriangleVertices(std::vector<std::vector<MIntArray>>& output, MObject& mesh);
    void transferPosition(MPointArray& input_points, MPointArray& output_points, 
        MFloatArray& closest_baraycenter_points, std::vector<std::vector<MIntArray>>& triangle_vertices);
    void raycast(MFnMesh& mesh, const int index, MMeshIsectAccelParams& accel_params, MPoint& point, 
        const float collision_range_value, const float collision_tolerance_range, int& hit_count);
    void overrideInternalDeformation(MFloatArray& input_points, MFloatArray& output_points, MFloatArray& inverse_mass,
        std::vector<int>& ids, std::vector<int>& tet_ids);
    void findInternalPoints(MFnMesh& mesh, MPointArray& input_points, const float collision_range_value, 
        const float collision_tolerance_range, std::vector<int>& internal_ids, std::vector<int>& external_ids);
    void getMask(std::string path, std::vector<float>& mask);

private:
    // Tetgen data stream
    tetgenio in, out;
    // Target mesh objects
    MObject _input_object;
    MObject _tet_object;
    MObject _target_object;
    // Data storage arrays
    MPointArray _init_vertex_positions;
    MPointArray _vertex_positions;
    MPointArray _input_vertex_positions;
    MPointArray _target_vertex_positions;
    MPointArray _init_target_vertex_positions;
    MFloatArray _flat_input_vertex_positions;
    MFloatArray _flat_vertex_positions;
    MPointArray _previous_vertex_positions;
    MFloatArray _previous_flat_vertex_positions;
    MFloatArray _closest_tetrahedral_baraycenter;
    MIntArray _tetrahedral_ids;
    MIntArray _tetrahedral_edge_ids;
    MFloatArray _edge_lengths;
    MFloatArray _vertex_velocity;
    MFloatArray _inverse_mass;
    MFloatArray _rest_volume;
    MFloatArray _temp;
    MFloatArray _grads;
    std::vector<int> _internal_ids;
    std::vector<int> _tet_internal_ids;
    std::vector<int> _external_ids;
    std::vector<float> _vertex_mask;
    std::vector<bool> _tet_mesh_surface_trigger;
    std::vector<std::vector<MIntArray>> _triangle_vertex_ids;
    // Initialization variable
    bool initialized;
    // Count intergers
    int _particle_count;
    int _tet_count;
    // Floating setting variables
    float _radius_edge_ratio;
    float _volume_constraint;
};