// Copyright Joseph Chittock @2022

#pragma once
#include "common.h"

class UVSpringRelaxNode : public MPxNode
{
public:
    UVSpringRelaxNode();
    virtual ~UVSpringRelaxNode();

    static void* creator() { return new UVSpringRelaxNode(); }
    static MStatus initialize();

    virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

    // Node information
    static MTypeId id;
    static MString typeName;

    // Attributes
    static MObject aInputGeom;
    static MObject aOutputGeom;
    static MObject aRestGeom;
    static MObject aStiffness;
    static MObject aAdaptiveStepSize;
    static MObject aStepSize;
    static MObject aIterations;
    static MObject aUVSet;
    static MObject aEnableRelax;

private:

    MStatus setOutputGeom(const MPlug& plug, MDataBlock& block, MDataHandle& inputGeomHandle, const MFloatArray* newU, const MFloatArray* newV);

    struct EdgeData {
        float length;    // 3D rest length
        float uvLength;  // UV rest length
        int v0;          // mesh vertex index 0
        int v1;          // mesh vertex index 1
        int uv0;         // UV index 0, from per-face lookup (correct shell)
        int uv1;         // UV index 1, from per-face lookup (correct shell)
    };

    // Cached rest data — invalidated when rest mesh topology or UV set changes
    std::map<int, EdgeData> restEdgeLengths;
    MString cachedUVSetName;
    int cachedRestEdgeCount;

    // Helper methods
    void computeRestData(MFnMesh& meshFn, MObject& meshObj, const MString& uvSetName, std::map<int, EdgeData>& restEdgeLengths);

    void findUVShellBorders(MFnMesh& meshFn,
        const MString& uvSetName,
        std::set<int>& borderUVs);

    bool relaxUVs(MFnMesh& meshFn,
        const MFloatArray& uArray,
        const MFloatArray& vArray,
        const MPointArray& currentPoints,
        const std::set<int>& borderUVs,
        float stiffness,
        float stepSize,
        int iterations,
        MFloatArray& newU,
        MFloatArray& newV,
        bool adaptiveStep);
};
