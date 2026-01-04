// Copyright Joseph Chittock @2022

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

    MStatus setOutputGeom(const MPlug& plug, MDataBlock& block, const MObject& object, const MFloatArray* newU, const MFloatArray* newV);

    struct EdgeData {
        float length;      // 3D rest length
        float uvLength;    // UV rest length
        int v0;
        int v1;
    };

    struct EdgeStretch
    {
        float ratio;
        int v0;
        int v1;
        float restLength;
        float currentLength;
    };

    struct UVNeighbor
    {
        int uvIndex;
        float stretchRatio;
    };

    // Cached data
    std::map<int, EdgeData> restEdgeLengths;

    // Helper methods
    void computeRestData(MFnMesh& meshFn, MObject& meshObj, const MString& uvSetName, std::map<int, EdgeData>& restEdgeLengths);

    void buildVertexToUVMap(MFnMesh& meshFn,
        const MString& uvSetName,
        std::map<int, int>& vertToUV);

    void findUVShellBorders(MFnMesh& meshFn,
        const MString& uvSetName,
        std::set<int>& borderUVs);

    bool relaxUVs(MFnMesh& meshFn,
        const MFloatArray& uArray,
        const MFloatArray& vArray,
        const MPointArray& currentPoints,
        const std::map<int, int>& vertToUV,
        const std::set<int>& borderUVs,
        float stiffness,
        float damping,
        int iterations,
        MFloatArray& newU,
        MFloatArray& newV,
        const bool& adaptiveStep);
};