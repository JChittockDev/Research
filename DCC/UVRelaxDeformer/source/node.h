// Copyright Joseph Chittock @2022

#pragma once
#include "common.h"
#include <unordered_set>
#include <unordered_map>

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
    static MObject aLockMeshBoundary;
    static MObject aLockUVSeams;
    static MObject aRelaxAxisU;
    static MObject aRelaxAxisV;
    static MObject aJacobiDamping;
    static MObject aTolerance;

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

    // Rest data cache
    std::vector<EdgeData> restEdgeLengths;
    MString               cachedUVSetName;
    int                   cachedRestEdgeCount;
    float                 cachedRestHash;

    // Border UV cache
    std::unordered_set<int> cachedBorderUVs;
    int                     cachedInputEdgeCount;
    bool                    cachedLockMeshBoundary;
    bool                    cachedLockUVSeams;

    static float hashPoints(const MPointArray& pts);

    void computeRestData(MFnMesh& meshFn, MObject& meshObj,
                         const MString& uvSetName,
                         std::vector<EdgeData>& outEdges);

    void findUVShellBorders(MObject& meshObj,
                            const MString& uvSetName,
                            bool lockMeshBoundary, bool lockUVSeams,
                            std::unordered_set<int>& borderUVs);

    bool relaxUVs(const MFloatArray& uArray, const MFloatArray& vArray,
                  const MPointArray& currentPoints,
                  const MPointArray& restPoints,
                  const std::unordered_set<int>& borderUVs,
                  float stiffness, float stepSize, int iterations,
                  MFloatArray& newU, MFloatArray& newV,
                  bool adaptiveStep, bool jacobiDamping, float tolerance);
};
