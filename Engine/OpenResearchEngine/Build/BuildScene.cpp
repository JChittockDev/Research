#include "../EngineApp.h"

void EngineApp::BuildScene()
{
    SerializeLevel();
    PushLights();
    PushGenericMesh();
    PushMesh();
    ImportTextures();
    SetRootSignatures();
    CompileShaders();
    PushMaterials();
    PushRenderItems();
    SetFenceResources();
    SetPipelineStates();

    mOnnxModelResource = std::make_unique<OnnxModelResource>(
        md3dDevice, mCommandList, mGemmRootSignature, mReluRootSignature,
        mLeakyReluRootSignature, mSigmoidRootSignature, mTanhRootSignature,
        mSoftmaxRootSignature, mPSOs["Gemm"], mPSOs["Relu"], mPSOs["LeakyRelu"],
        mPSOs["Sigmoid"], mPSOs["Tanh"], mPSOs["Softmax"]
    );
    
    mOnnxModelResource->Initialize(GetFullPath("Onnx/test.onnx"));
}