﻿#include "../EngineApp.h"

void EngineApp::SetPipelineStates()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC blendComputePSO = {};
    blendComputePSO.pRootSignature = mBlendRootSignature.Get();
    blendComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["blendCS"]->GetBufferPointer()), mShaders["blendCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&blendComputePSO, IID_PPV_ARGS(&mPSOs["blend"])));

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

    D3D12_COMPUTE_PIPELINE_STATE_DESC tensionComputePSO = {};
    tensionComputePSO.pRootSignature = mTensionRootSignature.Get();
    tensionComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["tensionCS"]->GetBufferPointer()), mShaders["tensionCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&tensionComputePSO, IID_PPV_ARGS(&mPSOs["tension"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC preSolveComputePSO = {};
    preSolveComputePSO.pRootSignature = mPreSolveRootSignature.Get();
    preSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["preSolveCS"]->GetBufferPointer()), mShaders["preSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&preSolveComputePSO, IID_PPV_ARGS(&mPSOs["preSolve"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC postSolveComputePSO = {};
    postSolveComputePSO.pRootSignature = mPostSolveRootSignature.Get();
    postSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["postSolveCS"]->GetBufferPointer()), mShaders["postSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&postSolveComputePSO, IID_PPV_ARGS(&mPSOs["postSolve"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC constraintSolveComputePSO = {};
    constraintSolveComputePSO.pRootSignature = mConstraintSolveRootSignature.Get();
    constraintSolveComputePSO.CS = { reinterpret_cast<BYTE*>(mShaders["constraintSolveCS"]->GetBufferPointer()), mShaders["constraintSolveCS"]->GetBufferSize() };
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&constraintSolveComputePSO, IID_PPV_ARGS(&mPSOs["constraintSolve"])));


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
    // PSO for sky.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;
    skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    skyPsoDesc.pRootSignature = mRootSignature.Get();
    skyPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()), mShaders["skyVS"]->GetBufferSize()};
    skyPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()), mShaders["skyPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));

    // **5️⃣ Configure the Graphics Pipeline State**
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gBufferPsoDesc = opaquePsoDesc;
    gBufferPsoDesc.pRootSignature = mGBufferRootSignature.Get();
    gBufferPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["GBufferVS"]->GetBufferPointer()), mShaders["GBufferVS"]->GetBufferSize() };
    gBufferPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["GBufferPS"]->GetBufferPointer()), mShaders["GBufferPS"]->GetBufferSize() };
    gBufferPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT; // gPosition
    gBufferPsoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT; // gNormal
    gBufferPsoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;     // gAlbedoSpec
    gBufferPsoDesc.NumRenderTargets = 3;
    md3dDevice->CreateGraphicsPipelineState(&gBufferPsoDesc, IID_PPV_ARGS(&mPSOs["GBuffer"]));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightingPsoDesc = opaquePsoDesc;
    lightingPsoDesc.InputLayout = { nullptr, 0 };
    lightingPsoDesc.pRootSignature = mLightingRootSignature.Get();
    lightingPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["LightingVS"]->GetBufferPointer()), mShaders["LightingVS"]->GetBufferSize() };
    lightingPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["LightingPS"]->GetBufferPointer()), mShaders["LightingPS"]->GetBufferSize() };
    lightingPsoDesc.DepthStencilState.DepthEnable = false;
    lightingPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    md3dDevice->CreateGraphicsPipelineState(&lightingPsoDesc, IID_PPV_ARGS(&mPSOs["Lighting"]));
}