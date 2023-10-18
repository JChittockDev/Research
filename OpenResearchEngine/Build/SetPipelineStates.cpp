#include "../EngineApp.h"

void EngineApp::SetPipelineStates()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC skinnedComputePSO = {};
    skinnedComputePSO.pRootSignature = mSkinnedRootSignature.Get();
    skinnedComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["skinnedCS"]->GetBufferPointer()), mShaders["skinnedCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&skinnedComputePSO, IID_PPV_ARGS(&mPSOs["skinned"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC triangleNormalComputePSO = {};
    triangleNormalComputePSO.pRootSignature = mTriangleNormalRootSignature.Get();
    triangleNormalComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["triangleNormalCS"]->GetBufferPointer()), mShaders["triangleNormalCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&triangleNormalComputePSO, IID_PPV_ARGS(&mPSOs["triangleNormal"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC vertexNormalComputePSO = {};
    vertexNormalComputePSO.pRootSignature = mVertexNormalRootSignature.Get();
    vertexNormalComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["vertexNormalCS"]->GetBufferPointer()), mShaders["vertexNormalCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&vertexNormalComputePSO, IID_PPV_ARGS(&mPSOs["vertexNormal"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC meshTransferComputePSO = {};
    meshTransferComputePSO.pRootSignature = mMeshTransferRootSignature.Get();
    meshTransferComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["meshTransferCS"]->GetBufferPointer()), mShaders["meshTransferCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&meshTransferComputePSO, IID_PPV_ARGS(&mPSOs["meshTransfer"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC simMeshTransferComputePSO = {};
    simMeshTransferComputePSO.pRootSignature = mSimMeshTransferRootSignature.Get();
    simMeshTransferComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["simMeshTransferCS"]->GetBufferPointer()), mShaders["simMeshTransferCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&simMeshTransferComputePSO, IID_PPV_ARGS(&mPSOs["simMeshTransfer"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC forceComputePSO = {};
    forceComputePSO.pRootSignature = mForceRootSignature.Get();
    forceComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["forceCS"]->GetBufferPointer()), mShaders["forceCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&forceComputePSO, IID_PPV_ARGS(&mPSOs["force"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC preSolveComputePSO = {};
    preSolveComputePSO.pRootSignature = mPreSolveRootSignature.Get();
    preSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["preSolveCS"]->GetBufferPointer()), mShaders["preSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&preSolveComputePSO, IID_PPV_ARGS(&mPSOs["preSolve"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC postSolveComputePSO = {};
    postSolveComputePSO.pRootSignature = mPostSolveRootSignature.Get();
    postSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["postSolveCS"]->GetBufferPointer()), mShaders["postSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&postSolveComputePSO, IID_PPV_ARGS(&mPSOs["postSolve"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC stretchConstraintSolveComputePSO = {};
    stretchConstraintSolveComputePSO.pRootSignature = mStretchConstraintSolveRootSignature.Get();
    stretchConstraintSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["stretchConstraintSolveCS"]->GetBufferPointer()), mShaders["stretchConstraintSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&stretchConstraintSolveComputePSO, IID_PPV_ARGS(&mPSOs["stretchConstraintSolve"])));


    //
    // PSO for opaque objects.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaquePsoDesc.pRootSignature = mRootSignature.Get();
    opaquePsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), mShaders["standardVS"]->GetBufferSize()};
    opaquePsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()), mShaders["opaquePS"]->GetBufferSize()};
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
    opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

    //
    // PSO for shadow map pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
    smapPsoDesc.RasterizerState.DepthBias = 100000;
    smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    smapPsoDesc.pRootSignature = mRootSignature.Get();
    smapPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["shadowVS"]->GetBufferPointer()), mShaders["shadowVS"]->GetBufferSize()};
    smapPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["shadowOpaquePS"]->GetBufferPointer()), mShaders["shadowOpaquePS"]->GetBufferSize()};
    smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets = 0;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_opaque"])));

    //
    // PSO for debug layer.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
    debugPsoDesc.pRootSignature = mRootSignature.Get();
    debugPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["debugVS"]->GetBufferPointer()), mShaders["debugVS"]->GetBufferSize()};
    debugPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["debugPS"]->GetBufferPointer()), mShaders["debugPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["debug"])));

    //
    // PSO for drawing normals.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = opaquePsoDesc;
    drawNormalsPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["drawNormalsVS"]->GetBufferPointer()), mShaders["drawNormalsVS"]->GetBufferSize()};
    drawNormalsPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["drawNormalsPS"]->GetBufferPointer()), mShaders["drawNormalsPS"]->GetBufferSize()};
    drawNormalsPsoDesc.RTVFormats[0] = SsaoMap::NormalMapFormat;
    drawNormalsPsoDesc.SampleDesc.Count = 1;
    drawNormalsPsoDesc.SampleDesc.Quality = 0;
    drawNormalsPsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&drawNormalsPsoDesc, IID_PPV_ARGS(&mPSOs["drawNormals"])));

    //
    // PSO for SSAO.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = opaquePsoDesc;
    ssaoPsoDesc.InputLayout = { nullptr, 0 };
    ssaoPsoDesc.pRootSignature = mSsaoRootSignature.Get();
    ssaoPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["ssaoVS"]->GetBufferPointer()), mShaders["ssaoVS"]->GetBufferSize()};
    ssaoPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["ssaoPS"]->GetBufferPointer()), mShaders["ssaoPS"]->GetBufferSize()};

    // SSAO effect does not need the depth buffer.
    ssaoPsoDesc.DepthStencilState.DepthEnable = false;
    ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    ssaoPsoDesc.RTVFormats[0] = SsaoMap::AmbientMapFormat;
    ssaoPsoDesc.SampleDesc.Count = 1;
    ssaoPsoDesc.SampleDesc.Quality = 0;
    ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&mPSOs["ssao"])));

    //
    // PSO for SSAO blur.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPsoDesc = ssaoPsoDesc;
    ssaoBlurPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["ssaoBlurVS"]->GetBufferPointer()), mShaders["ssaoBlurVS"]->GetBufferSize()};
    ssaoBlurPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["ssaoBlurPS"]->GetBufferPointer()), mShaders["ssaoBlurPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ssaoBlurPsoDesc, IID_PPV_ARGS(&mPSOs["ssaoBlur"])));

    //
    // PSO for sky.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;
    skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    skyPsoDesc.pRootSignature = mRootSignature.Get();
    skyPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()), mShaders["skyVS"]->GetBufferSize()};
    skyPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()), mShaders["skyPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));
}