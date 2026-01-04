#include <maya/MItMeshVertex.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MMeshIntersector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MPlug.h> 
#include <maya/MPoint.h> 
#include <maya/MPointArray.h> 
#include <maya/MMatrix.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MFnPlugin.h>
#include<maya/MAnimControl.h>
#include <maya/MItGeometry.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnData.h>
#include <maya/MThreadPool.h>
#include <math.h>
#include <vector>
#include <map>
#include <omp.h>

class clothDeformer : public MPxDeformerNode
{
public:
    virtual MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray);
    static  void*   creator();
    static  MStatus initialize();
    
    MStatus deform(MDataBlock& block,
                   MItGeometry& iterator,
                   const MMatrix& matrix,
                   unsigned int multiIndex) override;
    
    static const MTypeId id;
    static MObject aTime;
    static MObject aSpringIterations;
    static MObject aWindIntensity;
    static MObject aWindX;
    static MObject aWindY;
    static MObject aWindZ;
    static MObject aDragIntensity;
    static MObject aGravity;
    static MObject aTimeStep;
    static MObject aDamping;
    static MObject aProjectionTolerance;
    static MObject aCollisionOffset;
    static MObject aCollisionIterations;
    static MObject aSmoothingIntensity;
    static MObject aSmoothingIterations;
    static MObject aCollisionMesh;
    static MObject aWeightMap;
    static MObject aPerGeometry;

private:
    MVector constraintOffset(MVector currentDistance, float rest_length, float multiplier);
    bool ifIn(int value, MIntArray array);
    void debugPrint(int value, MString suffix);
    float dot(MVector f, MVector v);
    bool ifConstraintEqual(MIntArray array_a, MIntArray array_b);
    MStatus JumpToElement(MArrayDataHandle& hArray, unsigned int index);

    std::map<unsigned int, bool> _dirtyMap;
    std::map<unsigned int, bool> _initialized;
    std::vector<float> _constraintLengths;
    std::vector<MIntArray> _constraintArray;
    std::vector<MIntArray> _structuredVertexArray;
    std::vector<MIntArray> _primaryShearVerts;
    std::map<unsigned int, MPointArray> _previousPosition;
    std::map<unsigned int, MPointArray> _previousRelativePosition;
    std::map<unsigned int, MTime> _previousTime;
    std::map<unsigned int, MFloatArray> _weightMap;
};

const MTypeId clothDeformer::id( 0x00080052 );
MObject clothDeformer::aTime;
MObject clothDeformer::aSpringIterations;
MObject clothDeformer::aWindIntensity;
MObject clothDeformer::aWindX;
MObject clothDeformer::aWindY;
MObject clothDeformer::aWindZ;
MObject clothDeformer::aDragIntensity;
MObject clothDeformer::aGravity;
MObject clothDeformer::aTimeStep;
MObject clothDeformer::aDamping;
MObject clothDeformer::aProjectionTolerance;
MObject clothDeformer::aCollisionOffset;
MObject clothDeformer::aCollisionIterations;
MObject clothDeformer::aSmoothingIntensity;
MObject clothDeformer::aSmoothingIterations;
MObject clothDeformer::aCollisionMesh;
MObject clothDeformer::aWeightMap;
MObject clothDeformer::aPerGeometry;


void* clothDeformer::creator()
{
    return new clothDeformer();
}

MStatus clothDeformer::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;
    MFnCompoundAttribute cAttr;

    aTime = uAttr.create("time", "t", MFnUnitAttribute::kTime, 0.0);
    addAttribute(aTime);
    attributeAffects(aTime, outputGeom);

    aSpringIterations = nAttr.create("springIterations", "sit", MFnNumericData::kInt, 1);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSpringIterations);
    attributeAffects(aSpringIterations, outputGeom);

    aWindIntensity = nAttr.create("windIntensity", "wi", MFnNumericData::kFloat, 0.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aWindIntensity);
    attributeAffects(aWindIntensity, outputGeom);

    aWindX = nAttr.create("windX", "wx", MFnNumericData::kFloat, 0.0);
    nAttr.setMax(1.0);
    nAttr.setMin(-1.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aWindX);
    attributeAffects(aWindX, outputGeom);

    aWindY = nAttr.create("windY", "wy", MFnNumericData::kFloat, 0.0);
    nAttr.setMax(1.0);
    nAttr.setMin(-1.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aWindY);
    attributeAffects(aWindY, outputGeom);

    aWindZ = nAttr.create("windZ", "wz", MFnNumericData::kFloat, 0.0);
    nAttr.setMax(1.0);
    nAttr.setMin(-1.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aWindZ);
    attributeAffects(aWindZ, outputGeom);

    aDragIntensity = nAttr.create("dragIntensity", "di", MFnNumericData::kFloat, 1.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aDragIntensity);
    attributeAffects(aDragIntensity, outputGeom);

    aGravity = nAttr.create("gravity", "g", MFnNumericData::kFloat, 0.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aGravity);
    attributeAffects(aGravity, outputGeom);

    aTimeStep = nAttr.create("timeStep", "ts", MFnNumericData::kFloat, 0.25);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aTimeStep);
    attributeAffects(aTimeStep, outputGeom);

    aDamping = nAttr.create("damping", "d", MFnNumericData::kFloat, 0.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aDamping);
    attributeAffects(aDamping, outputGeom);

    aProjectionTolerance = nAttr.create("projectionTolerance", "pt", MFnNumericData::kFloat, 0.0001);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aProjectionTolerance);
    attributeAffects(aProjectionTolerance, outputGeom);

    aCollisionOffset = nAttr.create("collisionOffset", "co", MFnNumericData::kFloat, 0.0001);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aCollisionOffset);
    attributeAffects(aCollisionOffset, outputGeom);

    aCollisionIterations = nAttr.create("collisionIterations", "ci", MFnNumericData::kInt, 1);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aCollisionIterations);
    attributeAffects(aCollisionIterations, outputGeom);

    aSmoothingIntensity = nAttr.create("smoothingIntensity", "si", MFnNumericData::kFloat, 1.0);
    nAttr.setMax(1.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSmoothingIntensity);
    attributeAffects(aSmoothingIntensity, outputGeom);

    aSmoothingIterations = nAttr.create("smoothingIterations", "sitr", MFnNumericData::kInt, 0);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSmoothingIterations);
    attributeAffects(aSmoothingIterations, outputGeom);

    aWeightMap = nAttr.create("weightMap", "wm", MFnNumericData::kFloat, 1.0);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aWeightMap);
    attributeAffects(aWeightMap, outputGeom);

    aCollisionMesh = tAttr.create("collisionMesh", "cMesh", MFnData::kMesh);
    addAttribute(aCollisionMesh);
    attributeAffects(aCollisionMesh, outputGeom);

    aPerGeometry = cAttr.create("perGeometry", "pg");
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    cAttr.addChild(aWeightMap);
    addAttribute(aPerGeometry);
    attributeAffects(aPerGeometry, outputGeom);

    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer clothDeformer weightMap");

    return MStatus::kSuccess;
}

MStatus clothDeformer::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) {
    if (plug == aWeightMap) {
        unsigned int geomIndex = 0;
        if (plug.isArray()) {
            geomIndex = plug.parent().logicalIndex();
        }
        else {
            geomIndex = plug.array().parent().logicalIndex();
        }
        _dirtyMap[geomIndex] = true;
    }
    return MS::kSuccess;
}

MStatus clothDeformer::JumpToElement(MArrayDataHandle& hArray, unsigned int index) {
    MStatus status;
    status = hArray.jumpToElement(index);
    if (MFAIL(status)) {
        MArrayDataBuilder builder = hArray.builder(&status);
        //MArrayDataBuilder builder = hArray.builder(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        builder.addElement(index, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        status = hArray.set(builder);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        status = hArray.jumpToElement(index);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    return status;
}

MVector clothDeformer::constraintOffset(MVector currentDistance, float rest_length, float multiplier) {
    float length = fabs(currentDistance.length());
    float percentage = 1.0 - rest_length / length;
    MVector correctionVector = (currentDistance * percentage) * multiplier;
    return correctionVector;
}

bool clothDeformer::ifIn(int value, MIntArray array) {
    bool result = false;
    for (int y = 0; y < (int)array.length(); ++y) {
        if (value == array[y]) {
            result = true;
            break;
        }
    }
    return result;
}

bool clothDeformer::ifConstraintEqual(MIntArray array_a, MIntArray array_b) {
    bool result = false;
    if (array_a[0] == array_b[0]) {
        if (array_a[1] == array_b[1]) {
            result = true;
        }
    }
    if (array_a[0] == array_b[1]) {
        if (array_a[1] == array_b[0]) {
            result = true;
        }
    }
    if (array_a[1] == array_b[0]) {
        if (array_a[0] == array_b[1]) {
            result = true;
        }
    }
    return result;
}

void clothDeformer::debugPrint(int value, MString suffix) {
    MString output;
    output += value;
    MGlobal::displayInfo(output + " " + suffix);
}

float clothDeformer::dot(MVector f,MVector v)
{
    return f[0] * v[0] + f[1] * v[1] + f[2] * v[2];
}

MStatus clothDeformer::deform(MDataBlock& dataBlock,
                                MItGeometry& geomIter,
                                const MMatrix& matrix,
                                unsigned int multiIndex)
{
    MStatus status;
    MObject oInputMesh;
    MObject oCollisionMesh;
    MPointArray input_points;
    MPointArray deformed_points;
    geomIter.allPositions(input_points);
    deformed_points = input_points;

    MArrayDataHandle hInput = dataBlock.outputArrayValue(input, &status);
    hInput.jumpToElement(multiIndex);
    MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
    oInputMesh = hInputGeom.asMesh();
    MFnMesh fnMesh(oInputMesh, &status);

    oCollisionMesh = dataBlock.inputValue(aCollisionMesh).asMesh();
    MFnMesh collide_fnMesh(oCollisionMesh, &status);

    float envelopeValue = dataBlock.inputValue(envelope).asFloat();
    int springIterations = dataBlock.inputValue(aSpringIterations).asInt();
    float windIntensity = dataBlock.inputValue(aWindIntensity).asFloat();
    float windX = dataBlock.inputValue(aWindX).asFloat();
    float windY = dataBlock.inputValue(aWindY).asFloat();
    float windZ = dataBlock.inputValue(aWindZ).asFloat();
    float dragIntensity = dataBlock.inputValue(aDragIntensity).asFloat();
    float gravity = dataBlock.inputValue(aGravity).asFloat();
    float time_step = dataBlock.inputValue(aTimeStep).asFloat();
    float damping = dataBlock.inputValue(aDamping).asFloat();
    float projectionTolerance = dataBlock.inputValue(aProjectionTolerance).asFloat();
    float collisionOffset = dataBlock.inputValue(aCollisionOffset).asFloat();
    int collisionIterations = dataBlock.inputValue(aCollisionIterations).asInt();
    float smoothingIntensity = dataBlock.inputValue(aSmoothingIntensity).asFloat();
    int smoothingIterations = dataBlock.inputValue(aSmoothingIterations).asInt();
    MTime currentTime = dataBlock.inputValue(aTime).asTime();

    MArrayDataHandle hGeo = dataBlock.inputArrayValue(aPerGeometry);
    JumpToElement(hGeo, multiIndex);
    MDataHandle hPerGeometry = hGeo.inputValue();

    if (!_initialized[multiIndex]) {
        _dirtyMap[multiIndex] = true;
        _initialized[multiIndex] = true;
        _previousTime[multiIndex] = currentTime;
        _previousRelativePosition[multiIndex].setLength(input_points.length());
        _previousPosition[multiIndex].setLength(input_points.length());
        for (int i = 0; i < (int)input_points.length(); ++i) {
            _previousRelativePosition[multiIndex][i] = deformed_points[i] - input_points[i];
            _previousPosition[multiIndex][i] = deformed_points[i];
        }

        MItMeshVertex vertIter(oInputMesh);
        MItMeshVertex computeIter(oInputMesh);
        // Generate Structural Constraints
        #pragma omp parallel for private(i,structuralVertices,constraint) collapse()
        for (; !vertIter.isDone(); vertIter.next()) {
            int i = vertIter.index();
            MIntArray structuralVertices;
            vertIter.getConnectedVertices(structuralVertices);
            _structuredVertexArray.push_back(structuralVertices);
            for (int x = 0; x < (int)structuralVertices.length(); ++x) {
                MIntArray constraint;
                constraint.append(i);
                constraint.append(structuralVertices[x]);
                 _constraintArray.push_back(constraint);
            }
        }

        // Generate Primary Shear Constraints
        #pragma omp parallel for private(conflictedVertices,_connectedSecondaryVerts,result,exists,constraint) collapse()
        for (int i = 0; i < (int)input_points.length(); ++i) {
            MIntArray conflictedVertices;
            std::vector<MIntArray> _connectedSecondaryVerts;
            for (int x = 0; x < (int)_structuredVertexArray[i].length(); ++x) {
                _connectedSecondaryVerts.push_back(_structuredVertexArray[_structuredVertexArray[i][x]]);
            }
            for (int y = 0; y < (int)_connectedSecondaryVerts.size(); ++y) {
                for (int z = 0; z < (int)_connectedSecondaryVerts[y].length(); ++z) {
                    for (int b = 0; b < (int)_connectedSecondaryVerts.size(); ++b) {
                        if (b != y) {
                            bool result = ifIn(_connectedSecondaryVerts[y][z], _connectedSecondaryVerts[b]);
                            if (result == true) {
                                if (_connectedSecondaryVerts[y][z] != i) {
                                    bool exists = ifIn(_connectedSecondaryVerts[y][z], _structuredVertexArray[i]);
                                    if (exists == false) {
                                        conflictedVertices.append(_connectedSecondaryVerts[y][z]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            for (int x = 0; x < (int)conflictedVertices.length(); ++x) {
                MIntArray constraint;
                constraint.append(i);
                constraint.append(conflictedVertices[x]);
                _constraintArray.push_back(constraint);
            }
            _primaryShearVerts.push_back(conflictedVertices);
        }

        // Generate Secondary Shear Constraints 
        #pragma omp parallel for private(constraint) collapse()
        for (int i = 0; i < (int)input_points.length(); ++i) {
            for (int x = 0; x < (int)_structuredVertexArray[i].length(); ++x) {
                MIntArray constraint;
                constraint.append(_structuredVertexArray[i][x]);
                if (x < (_structuredVertexArray[i].length() - 1)) {
                    constraint.append(_structuredVertexArray[i][x + 1]);
                }
                else {
                    constraint.append(_structuredVertexArray[i][0]);
                    _constraintArray.push_back(constraint);
                }
            }
        }

        // Generate Bending Constraints //
        for (int i = 0; i < (int)input_points.length(); ++i) {
            for (int x = 0; x < (int)_primaryShearVerts[i].length(); ++x) {
                MIntArray connectedVerts = _structuredVertexArray[_primaryShearVerts[i][x]];
                MIntArray freeVerts;
                for (int y = 0; y < (int)connectedVerts.length(); ++y) {
                    bool result = ifIn(connectedVerts[y], _structuredVertexArray[i]);
                    if (result == false) {
                        if (connectedVerts[y] != _primaryShearVerts[i][x]) {
                            freeVerts.append(connectedVerts[y]);
                        }
                    }
                }
                for (int g = 0; g < (int)freeVerts.length(); ++g) {
                    MIntArray freeVert_connectedVertsA;
                    MIntArray freeVert_connectedVertsB;
                    if (g < (freeVerts.length() - 1)) {
                        freeVert_connectedVertsA = _structuredVertexArray[freeVerts[g]];
                        freeVert_connectedVertsB = _structuredVertexArray[freeVerts[g + 1]];
                    }
                    else {
                        freeVert_connectedVertsA = _structuredVertexArray[freeVerts[g]];
                        freeVert_connectedVertsB = _structuredVertexArray[freeVerts[0]];
                    }
                    for (int n = 0; n < (int)freeVert_connectedVertsA.length(); ++n) {
                        bool check = ifIn(freeVert_connectedVertsA[n], freeVert_connectedVertsB);
                        if (check == true) {
                            if (freeVert_connectedVertsA[n] != _primaryShearVerts[i][x]) {
                                MIntArray constraint;
                                constraint.append(freeVert_connectedVertsA[n]);
                                constraint.append(i);
                                _constraintArray.push_back(constraint);
                            }
                        }
                    }
                }
            }
        }

        #pragma omp parallel for private(distance,rest_length)
        for (int i = 0; i < (int)_constraintArray.size(); ++i) {
            MVector distance = input_points[_constraintArray[i][0]] - input_points[_constraintArray[i][1]];
            float rest_length = fabs(distance.length());
            _constraintLengths.push_back(rest_length);
        }
    }

    MFloatArray& weightMap = _weightMap[multiIndex];
    if (_dirtyMap[multiIndex]) {
        weightMap.setLength(geomIter.count());
        MArrayDataHandle hWeightMap = hPerGeometry.child(aWeightMap);
        int ii = 0;
        for (geomIter.reset(); !geomIter.isDone(); geomIter.next(), ii++) {
            status = JumpToElement(hWeightMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            weightMap[ii] = hWeightMap.inputValue().asFloat();
        }
        _dirtyMap[multiIndex] = false;
    }

    #pragma omp parallel for private() collapse()
    for (int i = 0; i < (int)input_points.length(); ++i) {
        deformed_points[i] = input_points[i] + _previousRelativePosition[multiIndex][i];
    }

    #pragma omp parallel for private(normal,acceleration,normalizedNormal,wind,velocity,drag) collapse()
    for (int i = 0; i < (int)input_points.length(); ++i) {
        MVector normal;
        fnMesh.getVertexNormal(i, true, normal, MSpace::kObject);
        MVector acceleration = MVector(0.0, 0.0, 0.0);
        acceleration += MVector(0.0, -gravity, 0.0);
        MVector normalizedNormal = normal;
        normalizedNormal.normalize();
        MVector wind = normal * (dot(normalizedNormal, MVector(windX, windY, windZ)));
        acceleration += wind * windIntensity;
        deformed_points[i] += ((_previousPosition[multiIndex][i] - deformed_points[i]) * damping + acceleration * time_step)* weightMap[i];
        MPoint velocity = (deformed_points[i] - _previousPosition[multiIndex][i]) * -1.0;
        MVector drag = normal * (dot(normalizedNormal, velocity));
        deformed_points[i] += drag * dragIntensity * weightMap[i];
    }

    #pragma omp parallel for private(currentDistance,structuredVertex_constraintOffset) collapse()
    for (int y = 0; y < springIterations; ++y) {
        for (int i = 0; i < (int)_constraintArray.size(); ++i) {
            MVector currentDistance = deformed_points[_constraintArray[i][0]] - deformed_points[_constraintArray[i][1]];
            MVector structuredVertex_constraintOffset = constraintOffset(currentDistance, _constraintLengths[i], 0.5);
            deformed_points[_constraintArray[i][0]] += -structuredVertex_constraintOffset * weightMap[_constraintArray[i][0]];
            deformed_points[_constraintArray[i][1]] += structuredVertex_constraintOffset * weightMap[_constraintArray[i][1]];
        }
    }

    if (oCollisionMesh.isNull() != true) {
        MMeshIntersector intersector;
        intersector.create(oCollisionMesh);
        MMeshIsectAccelParams accelParams = collide_fnMesh.autoUniformGridParams();
        #pragma omp parallel for private (normal, hitPoints, rayParams, gotHit, hitCount, signChange, collision, hitPoint, transformPoint, closestPoint) collapse()
        for (int z = 0; z < collisionIterations; ++z) {
            for (int i = 0; i < (int)deformed_points.length(); ++i) {
                MVector normal;
                MFloatPointArray hitPoints;
                MFloatArray rayParams;
                fnMesh.getVertexNormal(i, true, normal, MSpace::kObject);
                bool gotHit = false;
                gotHit = collide_fnMesh.allIntersections(deformed_points[i], normal, NULL, NULL, true, MSpace::kObject, 999999999, true, &accelParams, true, hitPoints, &rayParams, NULL, NULL, NULL, NULL, projectionTolerance);
                if (gotHit == true) {
                    int hitCount = hitPoints.length();
                    int signChange{};
                    for (int x = 0; x < (hitCount - 1); ++x) {
                        if (rayParams[x] * rayParams[x + 1] < 0) {
                            signChange = x;
                        }
                        else {
                            signChange = -1;
                        }
                    }
                    bool collision = false;
                    if (hitCount == 2 && (signChange + 1) == 1 && signChange != -1) {
                        collision = true;
                    }
                    //else if (hitCount > 2 && (hitCount / (signChange + 1)) != 2 && signChange != -1) {
                    //    collision = true;
                    //}
                    if (collision == true) {
                        MPointOnMesh hitPoint;
                        MPoint transformPoint;
                        intersector.getClosestPoint(deformed_points[i], hitPoint, 999999999);
                        MPoint closestPoint = hitPoint.getPoint();
                        deformed_points[i] = closestPoint + (closestPoint - deformed_points[i]) * collisionOffset;
                    }
                }
            }
        }
    }
    
    #pragma omp parallel for private()
    for (int i = 0; i < (int)input_points.length(); ++i) {
        _previousRelativePosition[multiIndex][i] = deformed_points[i] - input_points[i];
        _previousPosition[multiIndex][i] = deformed_points[i];
    }

    #pragma omp parallel for private(averagePoint) collapse()
    for (int y = 0; y < smoothingIterations; ++y) {
        for (int i = 0; i < (int)input_points.length(); ++i) {
            MPoint averagePoint = deformed_points[i];
            for (int x = 0; x < (int)_structuredVertexArray[i].length(); ++x) {
                averagePoint += deformed_points[_structuredVertexArray[i][x]];
            deformed_points[i] = (deformed_points[i] * (1.0 - smoothingIntensity)) + ((averagePoint / (_structuredVertexArray[i].length() + 1)) * smoothingIntensity);
            }
        }
    }

    geomIter.setAllPositions(deformed_points);
    _previousTime[multiIndex] = currentTime;
    return status;
}

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode(
        "clothDeformer" ,
        clothDeformer::id ,
        &clothDeformer::creator ,
        &clothDeformer::initialize ,
        MPxNode::kDeformerNode
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( clothDeformer::id );

    return result;
}
