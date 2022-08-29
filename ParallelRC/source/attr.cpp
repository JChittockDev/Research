// Copyright Joseph Chittock @2022

#include "node.h"

const MTypeId collisionDeformer::id(0x00081152);
MObject collisionDeformer::collider_mesh;

void* collisionDeformer::creator()
{
    return new collisionDeformer();
}

MStatus collisionDeformer::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    collider_mesh = tAttr.create("collisionMesh", "cMesh", MFnData::kMesh);
    addAttribute(collider_mesh);
    attributeAffects(collider_mesh, outputGeom);

    return MStatus::kSuccess;
}