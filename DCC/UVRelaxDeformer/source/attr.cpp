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
MObject UVSpringRelaxNode::aLockMeshBoundary;
MObject UVSpringRelaxNode::aLockUVSeams;
MObject UVSpringRelaxNode::aRelaxAxisU;
MObject UVSpringRelaxNode::aRelaxAxisV;
MObject UVSpringRelaxNode::aJacobiDamping;
MObject UVSpringRelaxNode::aTolerance;

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

<<<<<<< Updated upstream
    aStiffness = nAttr.create("stiffness", "stf", MFnNumericData::kFloat, 0.5, &status);
=======
    // Stiffness attribute (0.0 to 1.0)
    aStiffness = nAttr.create("stiffness", "stf", MFnNumericData::kFloat, 1.0, &status);
>>>>>>> Stashed changes
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);
    nAttr.setMax(1.0);
    status = addAttribute(aStiffness);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    aStepSize = nAttr.create("stepSize", "ss", MFnNumericData::kFloat, 0.5, &status);
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

<<<<<<< Updated upstream
    aIterations = nAttr.create("iterations", "iter", MFnNumericData::kInt, 3, &status);
=======
    // Iterations attribute
    aIterations = nAttr.create("iterations", "iter", MFnNumericData::kInt, 1, &status);
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
    aLockMeshBoundary = nAttr.create("lockMeshBoundary", "lmb", MFnNumericData::kBoolean, true, &status);
=======
	// Lock Mesh Boundary attribute

    aLockMeshBoundary = nAttr.create("lockMeshBoundary", "lmb", MFnNumericData::kBoolean, false, &status);
>>>>>>> Stashed changes
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aLockMeshBoundary);
    CHECK_MSTATUS_AND_RETURN_IT(status);

<<<<<<< Updated upstream
    aLockUVSeams = nAttr.create("lockUVSeams", "luvs", MFnNumericData::kBoolean, true, &status);
=======
    // Lock UV seams attribute
    aLockUVSeams = nAttr.create("lockUVSeams", "lus", MFnNumericData::kBoolean, false, &status);
>>>>>>> Stashed changes
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aLockUVSeams);
    CHECK_MSTATUS_AND_RETURN_IT(status);

<<<<<<< Updated upstream
=======
    // Relax axis U attribute
>>>>>>> Stashed changes
    aRelaxAxisU = nAttr.create("relaxAxisU", "rau", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aRelaxAxisU);
    CHECK_MSTATUS_AND_RETURN_IT(status);

<<<<<<< Updated upstream
=======
    // Relax axis V attribute
>>>>>>> Stashed changes
    aRelaxAxisV = nAttr.create("relaxAxisV", "rav", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aRelaxAxisV);
    CHECK_MSTATUS_AND_RETURN_IT(status);

<<<<<<< Updated upstream
    aJacobiDamping = nAttr.create("jacobiDamping", "jd", MFnNumericData::kBoolean, true, &status);
=======
    // Jacobi damping attribute
    aJacobiDamping = nAttr.create("jacobiDamping", "jdmp", MFnNumericData::kBoolean, false, &status);
>>>>>>> Stashed changes
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    status = addAttribute(aJacobiDamping);
    CHECK_MSTATUS_AND_RETURN_IT(status);

<<<<<<< Updated upstream
    aTolerance = nAttr.create("tolerance", "tol", MFnNumericData::kFloat, 0.0001f, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);
    nAttr.setMax(0.1);
    status = addAttribute(aTolerance);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    attributeAffects(aInputGeom,        aOutputGeom);
    attributeAffects(aRestGeom,         aOutputGeom);
    attributeAffects(aStiffness,        aOutputGeom);
    attributeAffects(aAdaptiveStepSize, aOutputGeom);
    attributeAffects(aStepSize,         aOutputGeom);
    attributeAffects(aIterations,       aOutputGeom);
    attributeAffects(aUVSet,            aOutputGeom);
    attributeAffects(aEnableRelax,      aOutputGeom);
    attributeAffects(aLockMeshBoundary, aOutputGeom);
    attributeAffects(aLockUVSeams,      aOutputGeom);
    attributeAffects(aRelaxAxisU,       aOutputGeom);
    attributeAffects(aRelaxAxisV,       aOutputGeom);
    attributeAffects(aJacobiDamping,    aOutputGeom);
    attributeAffects(aTolerance,        aOutputGeom);
=======
    // Tolerance attribute
    aTolerance = nAttr.create("tolerance", "tol", MFnNumericData::kFloat, 0.0001f, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0f);
    nAttr.setMax(1.0f);
    status = addAttribute(aTolerance);
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
    attributeAffects(aLockMeshBoundary, aOutputGeom);
    attributeAffects(aLockUVSeams, aOutputGeom);
    attributeAffects(aRelaxAxisU, aOutputGeom);
    attributeAffects(aRelaxAxisV, aOutputGeom);
    attributeAffects(aJacobiDamping, aOutputGeom);
    attributeAffects(aTolerance, aOutputGeom);
>>>>>>> Stashed changes

    return MS::kSuccess;
}
