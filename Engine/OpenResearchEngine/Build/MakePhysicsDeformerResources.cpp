#include "../EngineApp.h"

PhysicsDeformerResources EngineApp::MakePhysicsDeformerResources()
{
    PhysicsDeformerResources res{};
    res.meshTransfer       = mMeshTransferRootSignature.Get();
    res.psoMeshTransfer    = mPSOs.at("meshTransfer").Get();
    res.tension            = mTensionRootSignature.Get();
    res.psoTension         = mPSOs.at("tension").Get();
    res.force              = mForceRootSignature.Get();
    res.psoForce           = mPSOs.at("force").Get();
    res.preSolve           = mPreSolveRootSignature.Get();
    res.psoPreSolve        = mPSOs.at("preSolve").Get();
    res.constraintSolve    = mConstraintSolveRootSignature.Get();
    res.psoConstraintSolve = mPSOs.at("constraintSolve").Get();
    res.postSolve          = mPostSolveRootSignature.Get();
    res.psoPostSolve       = mPSOs.at("postSolve").Get();
    res.simMeshTransfer    = mSimMeshTransferRootSignature.Get();
    res.psoSimMeshTransfer = mPSOs.at("simMeshTransfer").Get();
    res.triangleNormal     = mTriangleNormalRootSignature.Get();
    res.psoTriangleNormal  = mPSOs.at("triangleNormal").Get();
    res.vertexNormal       = mVertexNormalRootSignature.Get();
    res.psoVertexNormal    = mPSOs.at("vertexNormal").Get();
    return res;
}
