// Copyright Joseph Chittock @2022

#include "node.h"

MStatus collisionDeformer::deform(MDataBlock& dataBlock,
                                MItGeometry& geomIter,
                                const MMatrix& matrix,
                                unsigned int multiIndex)
{
    MStatus status;
    MPointArray input_points;
    geomIter.allPositions(input_points, MSpace::kWorld);

    MArrayDataHandle hInput = dataBlock.outputArrayValue(input, &status);
    hInput.jumpToElement(multiIndex);
    MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
    _input_object = hInputGeom.asMesh();
    MFnMesh fnMesh(_input_object, &status);

    MDataHandle collider_mesh_handle = dataBlock.inputValue(collider_mesh);
    _collider_object = collider_mesh_handle.asMesh();
    
    if (_collider_object.isNull() != true)
    {
        std::map<int, std::vector<MVector>> colliderData = getMeshData(_collider_object);
        _collider_mesh.Set(colliderData);

        omp_set_nested(1);
        #pragma omp parallel for private(normal, rayCaster, point)
        for (int x = 0; x < input_points.length(); ++x)
        {
            MVector normal;
            PRCRayTracer rayCaster;
            const MVector point = input_points[x];
            fnMesh.getVertexNormal(x, true, normal, MSpace::kWorld);
            rayCaster.DetectIntersections(point, (const MVector)normal, 9999999.999, &_collider_mesh);
        }
    }

    geomIter.setAllPositions(input_points);
    return status;
}