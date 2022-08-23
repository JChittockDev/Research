// Copyright Joseph Chittock @2022

#include "node.h"

const MTypeId tissueNode::id(0x00080052);
MObject tissueNode::input_world_matrix;
MObject tissueNode::input_gravity;
MObject tissueNode::input_drag;
MObject tissueNode::input_mesh;
MObject tissueNode::target_mesh;
MObject tissueNode::output_mesh;
MObject tissueNode::input_time;
MObject tissueNode::input_substeps;
MObject tissueNode::input_edge_compliance;
MObject tissueNode::input_volume_compliance;
MObject tissueNode::input_timestep;
MObject tissueNode::input_density;
MObject tissueNode::input_radius_edge_ratio;
MObject tissueNode::input_volume_constraint;
MObject tissueNode::input_debug_mesh_diplsay;
MObject tissueNode::input_volume_cavity_pointer;
MObject tissueNode::input_vertex_mask_path;

void* tissueNode::creator()
{
    return new tissueNode();
}

MStatus tissueNode::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;
    MFnCompoundAttribute cAttr;
    MFnMatrixAttribute mAttr;

    output_mesh = tAttr.create("outputMesh", "om", MFnMeshData::kMesh);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    addAttribute(output_mesh);

    input_mesh = tAttr.create("inputMesh", "im", MFnMeshData::kMesh);
    tAttr.setKeyable(false);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(input_mesh);
    attributeAffects(input_mesh, output_mesh);

    target_mesh = tAttr.create("targetMesh", "tgtm", MFnMeshData::kMesh);
    tAttr.setKeyable(false);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(target_mesh);
    attributeAffects(target_mesh, output_mesh);

    input_world_matrix = mAttr.create("worldMatrix", "wm", MFnMatrixAttribute::kDouble);
    mAttr.setKeyable(false);
    mAttr.setStorable(true);
    mAttr.setReadable(true);
    mAttr.setWritable(true);
    addAttribute(input_world_matrix);
    attributeAffects(input_world_matrix, output_mesh);

    input_gravity = nAttr.create("gravity", "gr", MFnNumericData::kFloat, -10.0);
    uAttr.setMin(-10.0);
    uAttr.setMax(10.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    addAttribute(input_gravity);
    attributeAffects(input_gravity, output_mesh);

    input_drag = nAttr.create("drag", "dr", MFnNumericData::kFloat, 0.01);
    uAttr.setMin(0.0);
    uAttr.setMax(10.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    addAttribute(input_drag);
    attributeAffects(input_drag, output_mesh);

    input_time = uAttr.create("time", "tm", MFnUnitAttribute::kTime, 0.0);
    uAttr.setMin(0.0);
    uAttr.setKeyable(true);
    uAttr.setStorable(true);
    addAttribute(input_time);
    attributeAffects(input_time, output_mesh);

    input_substeps = nAttr.create("substeps", "ss", MFnNumericData::kInt, 5);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_substeps);
    attributeAffects(input_substeps, output_mesh);

    input_edge_compliance = nAttr.create("edgeCompliance", "ec", MFnNumericData::kFloat, 100.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_edge_compliance);
    attributeAffects(input_edge_compliance, output_mesh);

    input_volume_compliance = nAttr.create("volumeCompliance", "vmc", MFnNumericData::kFloat, 0.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_volume_compliance);
    attributeAffects(input_volume_compliance, output_mesh);

    input_timestep = nAttr.create("timeStep", "ts", MFnNumericData::kFloat, 0.025);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_timestep);
    attributeAffects(input_timestep, output_mesh);

    input_radius_edge_ratio = nAttr.create("radiusEdgeRatio", "rer", MFnNumericData::kFloat, 1.4);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_radius_edge_ratio);
    attributeAffects(input_radius_edge_ratio, output_mesh);

    input_volume_constraint = nAttr.create("volumeConstraint", "vc", MFnNumericData::kFloat, 0.1);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(input_volume_constraint);
    attributeAffects(input_volume_constraint, output_mesh);

    input_volume_cavity_pointer = nAttr.create("volumeCavityPointer", "vcp", MFnNumericData::k3Double);
    nAttr.setConnectable(true);
    addAttribute(input_volume_cavity_pointer);
    attributeAffects(input_volume_cavity_pointer, output_mesh);

    input_debug_mesh_diplsay = nAttr.create("debugMeshDisplay", "dmd", MFnNumericData::kBoolean, 0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    addAttribute(input_debug_mesh_diplsay);
    attributeAffects(input_debug_mesh_diplsay, output_mesh);

    input_vertex_mask_path = tAttr.create("vertexMask", "vtxm", MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setUsedAsFilename(true);
    addAttribute(input_vertex_mask_path);
    attributeAffects(input_vertex_mask_path, output_mesh);

    return MStatus::kSuccess;
}