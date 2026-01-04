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

class velocityDeformer : public MPxDeformerNode
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
    static MObject aMultiplier;
    static MObject aInverseSpread;
    static MObject aAmplitude;
    static MObject aFrequency;
    static MObject aDecay;
    static MObject aOffset;
    static MObject aBounceTolerance;
    static MObject aVelocityTolerance;
    static MObject aNormalIntensity;
    static MObject aSmoothingIntensity;
    static MObject aSmoothingIterations;
    static MObject aWaveTime;
    static MObject aMass;
    static MObject aStickiness;
    static MObject aStickyTolerance;
    static MObject aPerGeometry;
    static MObject aWeightMap;
    static MObject aMassMap;
    static MObject aSmoothMap;
    static MObject aStickyMap;

private:
    MStatus JumpToElement(MArrayDataHandle& hArray, unsigned int index);

    std::map<unsigned int, bool> _dirtyMap;
    std::map<unsigned int, bool> _initialized;
    std::map<unsigned int, float> _computeTime;
    std::map<unsigned int, MPointArray> _previousPosition;
    std::map<unsigned int, MFloatArray> _previousVelocity;
    std::map<unsigned int, MVectorArray> _goalShape;
    std::map<unsigned int, MVectorArray> _previousGoalShape;
    std::map<unsigned int, MFloatArray> _previousWavePoint;
    std::map<unsigned int, MFloatArray> _velocityComparison;
    std::map<unsigned int, MIntArray> _sineBounceIndex;
    std::map<unsigned int, MFloatArray> _weightMap;
    std::map<unsigned int, MFloatArray> _massMap;
    std::map<unsigned int, MFloatArray> _smoothMap;
    std::map<unsigned int, MFloatArray> _stickyMap;
    std::vector<MIntArray> _structuredVertexArray;
};

const MTypeId velocityDeformer::id( 0x00080042 );
MObject velocityDeformer::aMultiplier;
MObject velocityDeformer::aInverseSpread;
MObject velocityDeformer::aAmplitude;
MObject velocityDeformer::aFrequency;
MObject velocityDeformer::aDecay;
MObject velocityDeformer::aOffset;
MObject velocityDeformer::aBounceTolerance;
MObject velocityDeformer::aVelocityTolerance;
MObject velocityDeformer::aNormalIntensity;
MObject velocityDeformer::aSmoothingIntensity;
MObject velocityDeformer::aSmoothingIterations;
MObject velocityDeformer::aWaveTime;
MObject velocityDeformer::aMass;
MObject velocityDeformer::aStickiness;
MObject velocityDeformer::aStickyTolerance;
MObject velocityDeformer::aPerGeometry;
MObject velocityDeformer::aWeightMap;
MObject velocityDeformer::aMassMap;
MObject velocityDeformer::aSmoothMap;
MObject velocityDeformer::aStickyMap;

void* velocityDeformer::creator()
{
    return new velocityDeformer();
}


MStatus velocityDeformer::initialize()
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MStatus status;

    aMultiplier = nAttr.create("drag", "mlt", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aMultiplier);
    attributeAffects(aMultiplier, outputGeom);

    aInverseSpread = nAttr.create("inverseSpread", "iSp", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aInverseSpread);
    attributeAffects(aInverseSpread, outputGeom);

    aAmplitude = nAttr.create("amplitude", "amp", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aAmplitude);
    attributeAffects(aAmplitude, outputGeom);

    aFrequency = nAttr.create("frequency", "fq", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aFrequency);
    attributeAffects(aFrequency, outputGeom);

    aDecay = nAttr.create("decay", "dc", MFnNumericData::kFloat, 0.25, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aDecay);
    attributeAffects(aDecay, outputGeom);

    aOffset = nAttr.create("offset", "os", MFnNumericData::kFloat, 0.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aOffset);
    attributeAffects(aOffset, outputGeom);

    aBounceTolerance = nAttr.create("bounceTolerance", "btl", MFnNumericData::kFloat, 0.01, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aBounceTolerance);
    attributeAffects(aBounceTolerance, outputGeom);

    aVelocityTolerance = nAttr.create("velocityMinimumTolerance", "vmt", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aVelocityTolerance);
    attributeAffects(aVelocityTolerance, outputGeom);

    aNormalIntensity = nAttr.create("normalIntensity", "ni", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setMax(1.0);
    nAttr.setMin(0.0);
    addAttribute(aNormalIntensity);
    attributeAffects(aNormalIntensity, outputGeom);

    aSmoothingIntensity = nAttr.create("smoothingIntensity", "si", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setMax(1.0);
    nAttr.setMin(0.0);
    addAttribute(aSmoothingIntensity);
    attributeAffects(aSmoothingIntensity, outputGeom);

    aSmoothingIterations = nAttr.create("smoothingIterations", "sit", MFnNumericData::kInt, 2);
    nAttr.setMin(0);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aSmoothingIterations);
    attributeAffects(aSmoothingIterations, outputGeom);

    aMass = nAttr.create("mass", "m", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    addAttribute(aMass);
    attributeAffects(aMass, outputGeom);

    aStickiness = nAttr.create("stickiness", "stck", MFnNumericData::kFloat, 0.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setMax(1.0);
    nAttr.setMin(0.0);
    addAttribute(aStickiness);
    attributeAffects(aStickiness, outputGeom);

    aStickyTolerance = nAttr.create("stickyTolerance", "st", MFnNumericData::kFloat, 100.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setMin(0.0);
    addAttribute(aStickyTolerance);
    attributeAffects(aStickyTolerance, outputGeom);

    aWaveTime = nAttr.create("waveTime", "wt", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setMin(1.0);
    addAttribute(aWaveTime);
    attributeAffects(aWaveTime, outputGeom);

    aWeightMap = nAttr.create("weightMap", "wm", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aWeightMap);
    attributeAffects(aWeightMap, outputGeom);

    aMassMap = nAttr.create("massMap", "mm", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aMassMap);
    attributeAffects(aMassMap, outputGeom);

    aSmoothMap = nAttr.create("smoothMap", "sm", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aSmoothMap);
    attributeAffects(aSmoothMap, outputGeom);

    aStickyMap = nAttr.create("stickyMap", "smp", MFnNumericData::kFloat, 1.0, &status);
    nAttr.setArray(true);
    nAttr.setUsesArrayDataBuilder(true);
    addAttribute(aStickyMap);
    attributeAffects(aStickyMap, outputGeom);

    aPerGeometry = cAttr.create("perGeometry", "pg", &status);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    cAttr.addChild(aWeightMap);
    cAttr.addChild(aMassMap);
    cAttr.addChild(aStickyMap);
    addAttribute(aPerGeometry);
    attributeAffects(aPerGeometry, outputGeom);

    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer velocityDeformer weightMap");
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer velocityDeformer massMap");
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer velocityDeformer smoothMap");
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer velocityDeformer stickyMap");

    return MStatus::kSuccess;
}

MStatus velocityDeformer::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) {
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


MStatus velocityDeformer::JumpToElement(MArrayDataHandle& hArray, unsigned int index) {
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


MStatus velocityDeformer::deform(MDataBlock& dataBlock,
                                MItGeometry& geomIter,
                                const MMatrix& matrix,
                                unsigned int multiIndex)
{
    MStatus status;
    MObject oInputMesh;
    MPointArray input_points;
    MPointArray deformed_points;
    MPointArray final_points;
    MFloatArray sineArray;
    geomIter.allPositions(input_points);
    deformed_points = input_points;
    final_points = input_points;

    MArrayDataHandle hInput = dataBlock.outputArrayValue(input, &status);
    hInput.jumpToElement(multiIndex);
    MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
    oInputMesh = hInputGeom.asMesh();
    MFnMesh fnMesh(oInputMesh, &status);

    float envelopeValue = dataBlock.inputValue(envelope).asFloat();
    float multiplier = dataBlock.inputValue(aMultiplier).asFloat();
    float amplitude = dataBlock.inputValue(aAmplitude).asFloat();
    float spread = dataBlock.inputValue(aInverseSpread).asFloat();
    float frequency = dataBlock.inputValue(aFrequency).asFloat();
    float decay = dataBlock.inputValue(aDecay).asFloat();
    float offset = dataBlock.inputValue(aOffset).asFloat();
    float tolerance = dataBlock.inputValue(aBounceTolerance).asFloat();
    float normalIntensity = dataBlock.inputValue(aNormalIntensity).asFloat();
    float velocityTolerance = dataBlock.inputValue(aVelocityTolerance).asFloat();
    float smoothingIntensity = dataBlock.inputValue(aSmoothingIntensity).asFloat();
    int smoothingIterations = dataBlock.inputValue(aSmoothingIterations).asInt();
    float waveTime = dataBlock.inputValue(aWaveTime).asFloat();
    float mass = dataBlock.inputValue(aMass).asFloat();
    float stickiness = dataBlock.inputValue(aStickiness).asFloat();
    float stickyTolerance = dataBlock.inputValue(aStickyTolerance).asFloat();
    float timelineTime = MAnimControl::currentTime().value();
    float startTime = MAnimControl::animationStartTime().value();
    
    MArrayDataHandle hGeo = dataBlock.inputArrayValue(aPerGeometry);
    JumpToElement(hGeo, multiIndex);
    MDataHandle hPerGeometry = hGeo.inputValue();

    for (unsigned int w = 0; w < 1000; ++w) {
        float sine = powf(2.0,-1.0*decay*float(w))*(sinf(float(w)*frequency+offset));
        if (fabsf(sine) > tolerance) {
            sineArray.append(sine);
        }
    }

    MPointArray& previousPosition = _previousPosition[multiIndex];
    MFloatArray& previousVelocity = _previousVelocity[multiIndex];
    MVectorArray& goalShape = _goalShape[multiIndex];
    MVectorArray& previousGoalShape = _previousGoalShape[multiIndex];
    MFloatArray& previousWavePoint = _previousWavePoint[multiIndex];
    MFloatArray& velocityComparison = _velocityComparison[multiIndex];
    MIntArray& sineBounceIndex = _sineBounceIndex[multiIndex];
    float& computeTime = _computeTime[multiIndex];
    
    if (!_initialized[multiIndex]) {
        previousPosition.setLength(geomIter.count());
        previousVelocity.setLength(geomIter.count());
        goalShape.setLength(geomIter.count());
        previousGoalShape.setLength(geomIter.count());
        previousWavePoint.setLength(geomIter.count());
        velocityComparison.setLength(geomIter.count());
        sineBounceIndex.setLength(geomIter.count());
        previousPosition = input_points;
        computeTime = 0.0;
        _initialized[multiIndex] = true;
        _dirtyMap[multiIndex] = true;

        for (unsigned int i = 0; i < input_points.length(); ++i) {
            previousVelocity[i] = 0.0;
            goalShape[i] = MVector(0.0,0.0,0.0);
            previousGoalShape[i] = MVector(0.0, 0.0, 0.0);
            previousWavePoint[i] = 0.0;
            velocityComparison[i] = 0.0;
            sineBounceIndex[i] = 0;
        }
    }

    if (timelineTime == startTime) {
        for (unsigned int i = 0; i < input_points.length(); ++i) {
            previousVelocity[i] = 0.0;
            goalShape[i] = MVector(0.0, 0.0, 0.0);
            previousGoalShape[i] = MVector(0.0, 0.0, 0.0);
            previousWavePoint[i] = 0.0;
            velocityComparison[i] = 0.0;
            sineBounceIndex[i] = 0;
            computeTime = 0.0;
        }
    }

    MFloatArray& weightMap = _weightMap[multiIndex];
    MFloatArray& smoothMap = _smoothMap[multiIndex];
    MFloatArray& massMap = _massMap[multiIndex];
    MFloatArray& stickyMap = _stickyMap[multiIndex];
    if (_dirtyMap[multiIndex]) {
        weightMap.setLength(geomIter.count());
        smoothMap.setLength(geomIter.count());
        massMap.setLength(geomIter.count());
        stickyMap.setLength(geomIter.count());
        MArrayDataHandle hWeightMap = hPerGeometry.child(aWeightMap);
        MArrayDataHandle hMassMap = hPerGeometry.child(aMassMap);
        MArrayDataHandle hSmoothMap = hPerGeometry.child(aSmoothMap);
        MArrayDataHandle hStickyMap = hPerGeometry.child(aStickyMap);
        int ii = 0;
        for (geomIter.reset(); !geomIter.isDone(); geomIter.next(), ii++) {
            status = JumpToElement(hWeightMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            weightMap[ii] = hWeightMap.inputValue().asFloat();
            status = JumpToElement(hMassMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            massMap[ii] = hMassMap.inputValue().asFloat();
            status = JumpToElement(hSmoothMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            smoothMap[ii] = hSmoothMap.inputValue().asFloat();
            status = JumpToElement(hStickyMap, geomIter.index());
            CHECK_MSTATUS_AND_RETURN_IT(status);
            stickyMap[ii] = hStickyMap.inputValue().asFloat();
        }
        _dirtyMap[multiIndex] = false;
    }

    MMeshIntersector intersector;
    intersector.create(oInputMesh);
    #pragma omp parallel for private(normal, null, point, time, velocity, floating_velocity, normalizedVelocity, inverseVelocity, inverseNormalizedVelocity, aimWeight, spreadWeight, aw_mult, aw_inv, precalc, hitPoint)
    for (int i = 0; i < (int)input_points.length(); ++i) {
        
        MVector normal;
        MPointOnMesh hitPoint;
        MVector null = MVector(0.0, 0.0, 0.0);
        MPoint point = input_points[i];
        float time = computeTime;
        fnMesh.getVertexNormal(i, true, normal, MSpace::kObject);
        MPoint velocity = point - previousPosition[i];
        float floating_velocity = fabsf(sqrtf((powf(point[0] - previousPosition[i][0], 2) + powf(point[1] - previousPosition[i][1], 2) + powf(point[2] - previousPosition[i][2], 2))));
        MVector normalizedVelocity = MVector(velocity[0], velocity[1], velocity[2]).normal();
        MVector inverseVelocity = MVector(velocity[0], velocity[1], velocity[2]) * -1.0;
        MVector inverseNormalizedVelocity = normalizedVelocity * -1.0;
        float aimWeight = inverseNormalizedVelocity * normal;
        if (aimWeight > 1.0) {
            aimWeight = 1.0;
        }
        else if (aimWeight < 0.0) {
            aimWeight = 0.0;
        }
        float spreadWeight = powf(aimWeight, spread);
        if (spreadWeight > 1.0) {
            spreadWeight = 1.0;
        }
        else if (spreadWeight < 0.0) {
            spreadWeight = 0.0;
        }
        MPoint aw_mult = inverseVelocity * multiplier;
        MPoint aw_inv = ((aw_mult * spreadWeight * normalIntensity) + (aw_mult * (1.0 - normalIntensity))) / mass;

        if (floating_velocity < previousVelocity[i]) {
            if (velocityComparison[i] == 0.0) {
                velocityComparison[i] = 1.0;
                goalShape[i] = aw_inv;
                previousWavePoint[i] = time;
            }
        }

        if (floating_velocity > previousVelocity[i]) {
            if (floating_velocity > velocityTolerance) {
                velocityComparison[i] = 0.0;
                sineBounceIndex[i] = 0;
            }
        }

        if (velocityComparison[i] == 1.0) {
            float timeChecker = waveTime * mass * massMap[i];
            float timeDifferential = time - previousWavePoint[i];
            float bouncePercentage = timeDifferential / timeChecker;
            MPoint precalc = goalShape[i] * sineArray[sineBounceIndex[i]];
            null = previousGoalShape[i] + (precalc - previousGoalShape[i]) * bouncePercentage;
            if (timeDifferential >= timeChecker) {
                previousGoalShape[i] = precalc;
                sineBounceIndex[i] = sineBounceIndex[i] + 1;
                previousWavePoint[i] = time;
            }
            if (sineBounceIndex[i] == sineArray.length()) {
                velocityComparison[i] = 0.0;
                sineBounceIndex[i] = 0;
            }
        }
        previousVelocity[i] = floating_velocity;
        deformed_points[i] += (null * amplitude + aw_inv) * envelopeValue * weightMap[i] ;
        if (stickiness > 0.0) {
            if (stickyMap[i] > 0.0) {
                intersector.getClosestPoint(deformed_points[i], hitPoint, stickyTolerance);
                deformed_points[i] = ((hitPoint.getPoint() * stickiness) + (deformed_points[i] * (1.0 - stickiness)));
            }
        }
    }

    MItMeshVertex vertIter(oInputMesh);
    #pragma omp parallel for private(i,structuralVertices)
    for (; !vertIter.isDone(); vertIter.next()) {
        int i = vertIter.index();
        MIntArray structuralVertices;
        vertIter.getConnectedVertices(structuralVertices);
        _structuredVertexArray.push_back(structuralVertices);
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
        final_points[i] = (deformed_points[i] * envelopeValue) + (input_points[i] * (1.0 - envelopeValue));
    }

    previousPosition = input_points;
    geomIter.setAllPositions(final_points);
    computeTime += 1.0;
    return status;
}

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode(
        "velocityDeformer" ,
        velocityDeformer::id ,
        &velocityDeformer::creator ,
        &velocityDeformer::initialize ,
        MPxNode::kDeformerNode
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( velocityDeformer::id );

    return result;
}
