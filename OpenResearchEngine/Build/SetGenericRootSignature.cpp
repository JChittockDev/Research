#include "../EngineApp.h"

void EngineApp::SetGenericRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 18, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 18, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[5];

    // Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    
    ThrowIfFailed(hr);
    ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void EngineApp::SetSkinnedRootSignature()
{
    // Define root signature parameters
    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Setup root parameters
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsShaderResourceView(1);
    rootParameters[3].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();

    // Create the root signature description
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }

    ThrowIfFailed(hr);
    ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSkinnedRootSignature.GetAddressOf())));
}

void EngineApp::SetVerletSolverRootSignature()
{
    // Define root signature parameters
    CD3DX12_ROOT_PARAMETER rootParameters[5];

    // Setup root parameters
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsShaderResourceView(2);
    rootParameters[3].InitAsUnorderedAccessView(0);
    rootParameters[4].InitAsUnorderedAccessView(1);

    auto staticSamplers = GetStaticSamplers();

    // Create the root signature description
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(5, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }

    ThrowIfFailed(hr);
    ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mVerletSolverRootSignature.GetAddressOf())));
}

void EngineApp::SetTriangleNormalRootSignature()
{
    // Define root signature parameters
    CD3DX12_ROOT_PARAMETER rootParameters[3];

    // Setup root parameters
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();

    // Create the root signature description
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }

    ThrowIfFailed(hr);
    ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mTriangleNormalRootSignature.GetAddressOf())));
}

void EngineApp::SetVertexNormalRootSignature()
{
    // Define root signature parameters
    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Setup root parameters
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsShaderResourceView(2);
    rootParameters[3].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();

    // Create the root signature description
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }

    ThrowIfFailed(hr);
    ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mVertexNormalRootSignature.GetAddressOf())));
}