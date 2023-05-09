#include "../EngineApp.h"

void EngineApp::SetPipelineStates()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    //
    // PSO for opaque objects.
    //
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
    // PSO for skinned pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
    skinnedOpaquePsoDesc.InputLayout = { mSkinnedInputLayout.data(), (UINT)mSkinnedInputLayout.size() };
    skinnedOpaquePsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skinnedVS"]->GetBufferPointer()), mShaders["skinnedVS"]->GetBufferSize()};
    skinnedOpaquePsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()), mShaders["opaquePS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skinnedOpaquePsoDesc, IID_PPV_ARGS(&mPSOs["skinnedOpaque"])));

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

    // Shadow map pass does not have a render target.
    smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets = 0;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_opaque"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedSmapPsoDesc = smapPsoDesc;
    skinnedSmapPsoDesc.InputLayout = { mSkinnedInputLayout.data(), (UINT)mSkinnedInputLayout.size() };
    skinnedSmapPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skinnedShadowVS"]->GetBufferPointer()), mShaders["skinnedShadowVS"]->GetBufferSize()};
    skinnedSmapPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["shadowOpaquePS"]->GetBufferPointer()),mShaders["shadowOpaquePS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skinnedSmapPsoDesc, IID_PPV_ARGS(&mPSOs["skinnedShadow_opaque"])));

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

    D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedDrawNormalsPsoDesc = drawNormalsPsoDesc;
    skinnedDrawNormalsPsoDesc.InputLayout = { mSkinnedInputLayout.data(), (UINT)mSkinnedInputLayout.size() };
    skinnedDrawNormalsPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skinnedDrawNormalsVS"]->GetBufferPointer()), mShaders["skinnedDrawNormalsVS"]->GetBufferSize()};
    skinnedDrawNormalsPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["drawNormalsPS"]->GetBufferPointer()), mShaders["drawNormalsPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skinnedDrawNormalsPsoDesc, IID_PPV_ARGS(&mPSOs["skinnedDrawNormals"])));

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

    // The camera is inside the sky sphere, so just turn off culling.
    skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    // Make sure the depth function is LESS_EQUAL and not just LESS.  
    // Otherwise, the normalized depth values at z = 1 (NDC) will 
    // fail the depth test if the depth buffer was cleared to 1.
    skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    skyPsoDesc.pRootSignature = mRootSignature.Get();
    skyPsoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()), mShaders["skyVS"]->GetBufferSize()};
    skyPsoDesc.PS = {reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()), mShaders["skyPS"]->GetBufferSize()};
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));

}