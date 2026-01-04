// Copyright Joseph Chittock @2022

#include "node.h"

MObject UVSpringRelaxNode::aInputGeom;
MObject UVSpringRelaxNode::aOutputGeom;
MObject UVSpringRelaxNode::aRestGeom;
MObject UVSpringRelaxNode::aStiffness;
MObject UVSpringRelaxNode::aStepSize;
MObject UVSpringRelaxNode::aIterations;
MObject UVSpringRelaxNode::aUVSet;
MObject UVSpringRelaxNode::aEnableRelax;
MObject UVSpringRelaxNode::aAdaptiveStepSize;

MStatus UVSpringRelaxNode::initialize()
{
    MStatus status;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    aInputGeom = tAttr.create("inputGeom", "igeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(aInputGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aOutputGeom = tAttr.create("outputGeom", "ogeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(true);
    addAttribute(aOutputGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Input geometry attribute (array, accepts mesh)
    aRestGeom = tAttr.create("restGeom", "rgeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    status = addAttribute(aRestGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Stiffness attribute (0.0 to 1.0)
    aStiffness = nAttr.create("stiffness", "stf", MFnNumericData::kFloat, 0.5, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);
    nAttr.setMax(1000000000000.0);
    status = addAttribute(aStiffness);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aStepSize = nAttr.create("stepSize", "ss", MFnNumericData::kFloat, 0.02, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);
    nAttr.setMax(1.0);
    status = addAttribute(aStepSize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aAdaptiveStepSize = nAttr.create("adaptiveStepSize", "ass", MFnNumericData::kBoolean, false, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aAdaptiveStepSize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Iterations attribute
    aIterations = nAttr.create("iterations", "iter", MFnNumericData::kInt, 3, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(1);
    nAttr.setMax(1000);
    status = addAttribute(aIterations);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // UV Set name attribute
    MFnStringData stringData;
    MObject defaultString = stringData.create("", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    aUVSet = tAttr.create("uvSet", "uvs", MFnData::kString, defaultString, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setKeyable(false);
    status = addAttribute(aUVSet);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Enable/disable attribute
    aEnableRelax = nAttr.create("enable", "enble", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aEnableRelax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Set up attribute affects relationships
    attributeAffects(aInputGeom, aOutputGeom);
    attributeAffects(aRestGeom, aOutputGeom);
    attributeAffects(aStiffness, aOutputGeom);
	attributeAffects(aAdaptiveStepSize, aOutputGeom);
    attributeAffects(aStepSize, aOutputGeom);
    attributeAffects(aIterations, aOutputGeom);
    attributeAffects(aUVSet, aOutputGeom);
    attributeAffects(aEnableRelax, aOutputGeom);

    return MS::kSuccess;
}