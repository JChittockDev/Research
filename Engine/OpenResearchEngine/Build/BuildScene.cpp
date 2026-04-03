#include "../EngineApp.h"

void EngineApp::BuildScene()
{
    SerializeLevel();
    PushLights();

    mAssets = std::make_unique<AssetManager>(
        md3dDevice, mCommandList,
        std::filesystem::path(GetFullPath("")),
        m4xMsaaState, m4xMsaaQuality,
        mBackBufferFormat, mDepthStencilFormat
    );
    mAssets->Build(dynamicLights, mLevelRenderItems, mLevelMaterials, mLevelLights);

    PushRenderItems();
    SetFenceResources();

    mOnnxModelResource = std::make_unique<OnnxModelResource>(
        md3dDevice, mCommandList,
        mAssets->mGemmRootSignature,    mAssets->mReluRootSignature,
        mAssets->mLeakyReluRootSignature, mAssets->mSigmoidRootSignature,
        mAssets->mTanhRootSignature,    mAssets->mSoftmaxRootSignature,
        mAssets->mPSOs["Gemm"],  mAssets->mPSOs["Relu"],      mAssets->mPSOs["LeakyRelu"],
        mAssets->mPSOs["Sigmoid"], mAssets->mPSOs["Tanh"],    mAssets->mPSOs["Softmax"]
    );
    mOnnxModelResource->Initialize(GetFullPath("Onnx/test.onnx"));
}
