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
MObject UVSpringRelaxNode::aLockBorderUVs;
MObject UVSpringRelaxNode::aRelaxAxisU;
MObject UVSpringRelaxNode::aRelaxAxisV;
MObject UVSpringRelaxNode::aJacobiDamping;

MStatus UVSpringRelaxNode::initialize()
{
    MStatus status;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute   tAttr;

    aInputGeom = tAttr.create("inputGeom", "igeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    status = addAttribute(aInputGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aOutputGeom = tAttr.create("outputGeom", "ogeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(true);
    status = addAttribute(aOutputGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aRestGeom = tAttr.create("restGeom", "rgeo", MFnData::kMesh, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setStorable(true);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    status = addAttribute(aRestGeom);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aStiffness = nAttr.create("stiffness", "stf", MFnNumericData::kFloat, 0.5, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);
    nAttr.setMax(1.0);
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

    aIterations = nAttr.create("iterations", "iter", MFnNumericData::kInt, 3, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(1);
    nAttr.setMax(1000);
    status = addAttribute(aIterations);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnStringData stringData;
    MObject defaultString = stringData.create("", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    aUVSet = tAttr.create("uvSet", "uvs", MFnData::kString, defaultString, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    tAttr.setKeyable(false);
    status = addAttribute(aUVSet);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aEnableRelax = nAttr.create("enable", "enble", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aEnableRelax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aLockBorderUVs = nAttr.create("lockBorderUVs", "lbuv", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aLockBorderUVs);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aRelaxAxisU = nAttr.create("relaxAxisU", "rau", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aRelaxAxisU);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aRelaxAxisV = nAttr.create("relaxAxisV", "rav", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aRelaxAxisV);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aJacobiDamping = nAttr.create("jacobiDamping", "jd", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aJacobiDamping);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    attributeAffects(aInputGeom,        aOutputGeom);
    attributeAffects(aRestGeom,         aOutputGeom);
    attributeAffects(aStiffness,        aOutputGeom);
    attributeAffects(aAdaptiveStepSize, aOutputGeom);
    attributeAffects(aStepSize,         aOutputGeom);
    attributeAffects(aIterations,       aOutputGeom);
    attributeAffects(aUVSet,            aOutputGeom);
    attributeAffects(aEnableRelax,      aOutputGeom);
    attributeAffects(aLockBorderUVs,    aOutputGeom);
    attributeAffects(aRelaxAxisU,       aOutputGeom);
    attributeAffects(aRelaxAxisV,       aOutputGeom);
    attributeAffects(aJacobiDamping,    aOutputGeom);

    return MS::kSuccess;
}
