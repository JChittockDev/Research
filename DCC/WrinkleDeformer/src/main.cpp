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

class wrinkleDeformer : public MPxDeformerNode
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
    static MObject aSpringIntensity;
    static MObject aSpringIterations;
    static MObject aSmoothingIntensity;
    static MObject aSmoothingIterations;
    static MObject aWeightMap;
    static MObject aSpringConstraintMap;
    static MObject aSmoothMap;
    static MObject aPerGeometry;

private:
    MVector constraintOffset(MVector currentDistance, float rest_length, float multiplier);
    bool ifIn(int value, MIntArray array);
    MFloatArray normalizeFloatingArray(MFloatArray array);
    MStatus JumpToElement(MArrayDataHandle& hArray, unsigned int index);

    std::map<unsigned int, bool> _initialized;
    std::map<unsigned int, bool> _dirtyMap;
    std::vector<float> _constraintLengths;
    std::vector<MIntArray> _constraintArray;
    std::vector<MIntArray> _structuredVertexArray;
    std::vector<MIntArray> _primaryShearVerts;
    std::map<unsigned int, MPointArray> _previousRelativePosition;
    std::map<unsigned int, MPointArray> _initPosition;
    std::map<unsigned int, MTime> _previousTime;
    std::map<unsigned int, MFloatArray> _weightMap;
    std::map<unsigned int, MFloatArray> _springMap;
    std::map<unsigned int, MFloatArray> _smoothMap;
};

const MTypeId wrinkleDeformer::id( 0x00080052 );
MObject wrinkleDeformer::aSpringIntensity;
MObject wrinkleDeformer::aSpringIterations;
MObject wrinkleDeformer::aSmoothingIntensity;
MObject wrinkleDeformer::aSmoothingIterations;
MObject wrinkleDeformer::aWeightMap;
MObject wrinkleDeformer::aSpringConstraintMap;
MObject wrinkleDeformer::aSmoothMap;
MObject wrinkleDeformer::aPerGeometry;
;


void* wrinkleDeformer::creator()
{
    return new wrinkleDeformer();
}

MStatus wrinkleDeformer::initialize()
{
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;
    MFnCompoundAttribute cAttr;

    aSpringIntensity = nAttr.create("springIntensity", "sint", MFnNumericData::kFloat, 1.0);
    nAttr.setMax(1.0);
    nAttr.setMin(0.0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSpringIntensity);
    attributeAffects(aSpringIntensity, outputGeom);

    aSpringIterations = nAttr.create("springIterations", "sit", MFnNumericData::kInt, 1);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSpringIterations);
    attributeAffects(aSpringIterations, outputGeom);

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
    nAttr.setStorable(true);
    nAttr.setArray(true);
    nAttr.setConnectable(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aWeightMap);
    attributeAffects(aWeightMap, outputGeom);

    aSpringConstraintMap = nAttr.create("springConstraintMap", "scm", MFnNumericData::kFloat, 1.0);
    nAttr.setStorable(true);
    nAttr.setArray(true);
    nAttr.setConnectable(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aSpringConstraintMap);
    attributeAffects(aSpringConstraintMap, outputGeom);

    aSmoothMap = nAttr.create("smoothingMap", "sm", MFnNumericData::kFloat, 1.0);
    nAttr.setStorable(true);
    nAttr.setArray(true);
    nAttr.setConnectable(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aSmoothMap);
    attributeAffects(aSmoothMap, outputGeom);

    aPerGeometry = cAttr.create("perGeometry", "pg");
    cAttr.setStorable(true);
    cAttr.setArray(true);
    cAttr.setConnectable(true);
    cAttr.setUsesArrayDataBuilder(true);
    cAttr.addChild(aWeightMap);
    cAttr.addChild(aSpringConstraintMap);
    cAttr.addChild(aSmoothMap);
    addAttribute(aPerGeometry);
    attributeAffects(aPerGeometry, outputGeom);

    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer wrinkleDeformer weightMap");
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer wrinkleDeformer springConstraintMap");
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer wrinkleDeformer smoothingMap");

    return MStatus::kSuccess;
}

MStatus selfCollisionDeformer::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) {
    if (plug == aWeightMap || plug == aSpringConstraintMap || plug == aSmoothMap) {
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

MStatus wrinkleDeformer::JumpToElement(MArrayDataHandle& hArray, unsigned int index) {
    MStatus status;
    status = hArray.jumpToElement(index);
    if (MFAIL(status)) {
        MArrayDataBuilder builder = hArray.builder(&status);
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

MVector wrinkleDeformer::constraintOffset(MVector currentDistance, float rest_length, float multiplier) {
    float length = fabs(currentDistance.length());
    float percentage = 1.0 - rest_length / length;
    MVector correctionVector = (currentDistance * percentage) * multiplier;
    return correctionVector;
}

bool wrinkleDeformer::ifIn(int value, MIntArray array) {
    bool result = false;
    for (int y = 0; y < (int)array.length(); ++y) {
        if (value == array[y]) {
            result = true;
            break;
        }
    }
    return result;
}

MFloatArray wrinkleDeformer::normalizeFloatingArray(MFloatArray array) {
    float highestValue = 0.0;
    for (int i = 0; i < (int)array.length(); ++i) {
        if (array[i] > highestValue) {
            highestValue = array[i];
        }
    }
    for (int i = 0; i < (int)array.length(); ++i) {
        if (array[i] != 0) {
            array[i] = array[i] / highestValue;
        }
    }
    return array;
}

MStatus wrinkleDeformer::deform(MDataBlock& dataBlock,
                                MItGeometry& geomIter,
                                const MMatrix& matrix,
                                unsigned int multiIndex)
{
    MStatus status;
    MObject oInputMesh;
    MPointArray input_points;
    MPointArray deformed_points;
    geomIter.allPositions(input_points);
    deformed_points = input_points;

    MFloatArray deltaDifferentialLength;
    deltaDifferentialLength.setLength(input_points.length());

    MArrayDataHandle hInput = dataBlock.outputArrayValue(input, &status);
    hInput.jumpToElement(multiIndex);
    MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
    oInputMesh = hInputGeom.asMesh();
    MFnMesh fnMesh(oInputMesh, &status);

    float envelopeValue = dataBlock.inputValue(envelope).asFloat();
    float springIntensity = dataBlock.inputValue(aSpringIntensity).asFloat();
    int springIterations = dataBlock.inputValue(aSpringIterations).asInt();
    float smoothingIntensity = dataBlock.inputValue(aSmoothingIntensity).asFloat();
    int smoothingIterations = dataBlock.inputValue(aSmoothingIterations).asInt();
    float timelineTime = MAnimControl::currentTime().value();

    MArrayDataHandle hGeo = dataBlock.inputArrayValue(aPerGeometry);
    JumpToElement(hGeo, multiIndex);
    MDataHandle hPerGeometry = hGeo.inputValue();

    if (!_initialized[multiIndex]) {
        _dirtyMap[multiIndex] = true;
        _initialized[multiIndex] = true;
        _initPosition[multiIndex] = input_points;
        _previousRelativePosition[multiIndex].setLength(input_points.length());
        for (int i = 0; i < (int)input_points.length(); ++i) {
            _previousRelativePosition[multiIndex][i] = deformed_points[i] - input_points[i];
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
    MFloatArray& springMap = _springMap[multiIndex];
    MFloatArray& smoothMap = _smoothMap[multiIndex];
    if (_dirtyMap[multiIndex]) {
        weightMap.setLength(geomIter.count());
        springMap.setLength(geomIter.count());
        smoothMap.setLength(geomIter.count());
        MArrayDataHandle hWeightMap = hPerGeometry.child(aWeightMap);
        MArrayDataHandle hSpringMap = hPerGeometry.child(aSpringConstraintMap);
        MArrayDataHandle hSmoothMap = hPerGeometry.child(aSmoothMap);
        int ii = 0;
        for (geomIter.reset(); !geomIter.isDone(); geomIter.next(), ii++) {
            status = JumpToElement(hWeightMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            weightMap[ii] = hWeightMap.inputValue().asFloat();
            status = JumpToElement(hSpringMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            springMap[ii] = hSpringMap.inputValue().asFloat();
            status = JumpToElement(hSmoothMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            smoothMap[ii] = hSmoothMap.inputValue().asFloat();
        }
        _dirtyMap[multiIndex] = false;
    }

    float highestValue = 0.0;
    #pragma omp parallel for private(delta,deltaLength) collapse()
    for (int i = 0; i < (int)input_points.length(); ++i) {
        MVector delta = input_points[i] - _initPosition[multiIndex][i];
        float deltaLength = fabs(delta.length());
        deltaDifferentialLength[i] = deltaLength;
        if (deltaLength > highestValue) {
            highestValue = deltaLength;
        }
    }

    #pragma omp parallel for private() collapse()
    for (int i = 0; i < (int)deltaDifferentialLength.length(); ++i) {
        if (deltaDifferentialLength[i] != 0) {
            deltaDifferentialLength[i] = deltaDifferentialLength[i] / highestValue;
        }
    }

    if (timelineTime <= 0) {
        deformed_points = input_points;
    }
    else {
        #pragma omp parallel for private() collapse()
        for (int i = 0; i < (int)input_points.length(); ++i) {
            deformed_points[i] = input_points[i] + _previousRelativePosition[multiIndex][i] * deltaDifferentialLength[i];
        }
    }

    #pragma omp parallel for private(currentDistance,structuredVertex_constraintOffset) collapse()
    for (int y = 0; y < springIterations; ++y) {
        for (int i = 0; i < (int)_constraintArray.size(); ++i) {
            MVector currentDistance = deformed_points[_constraintArray[i][0]] - deformed_points[_constraintArray[i][1]];
            MVector structuredVertex_constraintOffset = constraintOffset(currentDistance, _constraintLengths[i], 0.5);
            deformed_points[_constraintArray[i][0]] += -structuredVertex_constraintOffset * deltaDifferentialLength[_constraintArray[i][0]] * springIntensity * springMap[_constraintArray[i][0]];
            deformed_points[_constraintArray[i][1]] += structuredVertex_constraintOffset * deltaDifferentialLength[_constraintArray[i][1]] * springIntensity * springMap[_constraintArray[i][1]];
        }
    }


    #pragma omp parallel for private()
    for (int i = 0; i < (int)input_points.length(); ++i) {
        _previousRelativePosition[multiIndex][i] = deformed_points[i] - input_points[i];
    }

    #pragma omp parallel for private(averagePoint,vertexSmoothingIntensity) collapse()
    for (int y = 0; y < smoothingIterations; ++y) {
        for (int i = 0; i < (int)input_points.length(); ++i) {
            MPoint averagePoint = deformed_points[i] - input_points[i];
            for (int x = 0; x < (int)_structuredVertexArray[i].length(); ++x) {
                averagePoint += deformed_points[_structuredVertexArray[i][x]] - input_points[_structuredVertexArray[i][x]];
            }
            float vertexSmoothingIntensity = smoothingIntensity * smoothMap[i];
            deformed_points[i] = (deformed_points[i] * (1.0 - vertexSmoothingIntensity)) + (((averagePoint / (_structuredVertexArray[i].length() + 1)) + input_points[i]) * vertexSmoothingIntensity);
        }
    }

    #pragma omp parallel for private(envelopeVertex)
    for (int i = 0; i < (int)input_points.length(); ++i) {
        float envelopeVertex = envelopeValue * weightMap[i];
        deformed_points[i] = (deformed_points[i] * envelopeVertex) + (input_points[i] * (1.0 - envelopeVertex));
    }


    geomIter.setAllPositions(deformed_points);

    return status;
}

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode(
        "wrinkleDeformer" ,
        wrinkleDeformer::id ,
        &wrinkleDeformer::creator ,
        &wrinkleDeformer::initialize ,
        MPxNode::kDeformerNode
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( wrinkleDeformer::id );

    return result;
}
