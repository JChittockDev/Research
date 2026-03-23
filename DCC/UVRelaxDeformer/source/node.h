// Copyright Joseph Chittock @2022

#pragma once
#include "common.h"

class UVSpringRelaxNode : public MPxNode
{
public:
    UVSpringRelaxNode();
    virtual ~UVSpringRelaxNode();

    static void*   creator()    { return new UVSpringRelaxNode(); }
    static MStatus initialize();

    virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

    static MTypeId id;
    static MString typeName;

    static MObject aInputGeom;
    static MObject aOutputGeom;
    static MObject aRestGeom;
    static MObject aStiffness;
    static MObject aAdaptiveStepSize;
    static MObject aStepSize;
    static MObject aIterations;
    static MObject aUVSet;
    static MObject aEnableRelax;
    static MObject aLockBorderUVs;
    static MObject aRelaxAxisU;
    static MObject aRelaxAxisV;
    static MObject aJacobiDamping;

private:

    MStatus setOutputGeom(const MPlug& plug, MDataBlock& block,
                          MDataHandle& inputGeomHandle,
                          const MFloatArray* newU, const MFloatArray* newV);

    struct EdgeData {
        float length;    // 3D rest length
        float uvLength;  // UV rest length
        int   v0, v1;    // mesh vertex indices
        int   uv0, uv1;  // UV indices (per-face lookup, correct shell)
    };

    std::vector<EdgeData> restEdgeLengths;
    MString               cachedUVSetName;
    int                   cachedRestEdgeCount;

    void computeRestData(MFnMesh& meshFn, MObject& meshObj,
                         const MString& uvSetName,
                         std::vector<EdgeData>& outEdges);

    void findUVShellBorders(MObject& meshObj,
                            const MString& uvSetName,
                            std::set<int>& borderUVs);

    bool relaxUVs(MFnMesh& meshFn,
                  const MFloatArray& uArray, const MFloatArray& vArray,
                  const MPointArray& currentPoints,
                  const std::set<int>& borderUVs,
                  float stiffness, float stepSize, int iterations,
                  MFloatArray& newU, MFloatArray& newV,
                  bool adaptiveStep, bool jacobiDamping);
};
