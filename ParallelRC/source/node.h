// Copyright Joseph Chittock @2022

#include "common.h"
#include "raycast.h"

class collisionDeformer : public MPxDeformerNode
{
public:
    static  void* creator();
    static  MStatus initialize();

    MStatus deform(MDataBlock& block,
                    MItGeometry& iterator,
                    const MMatrix& matrix,
                    unsigned int multiIndex) override;

    static const MTypeId id;
    static MObject collider_mesh;

private:
    bool init;
    PRCMesh _input_mesh;
    PRCMesh _collider_mesh;
    MObject _input_object;
    MObject _collider_object;
    std::map<int, std::vector<MVector>> getMeshData(MObject& mesh);
};
