#pragma once
#include "../Common/Structures.h"
#include "onnx.pb.h"
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <cmath>


class OnnxModelResource
{

public:
    OnnxModelResource::OnnxModelResource(
    Microsoft::WRL::ComPtr<ID3D12Device>& device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& gemmRootSig,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& reluRootSig,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& leakyReluRootSig,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& sigmoidRootSig,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& tanhRootSig,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& softmaxRootSig,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& gemmPSO,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& reluPSO,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& leakyReluPSO,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& sigmoidPSO,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& tanhPSO,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& softmaxPSO);

    OnnxModelResource::~OnnxModelResource();

    void Initialize(const std::string& path);

    void Evaluate(const std::vector<float>& inputData);

    void ReadBackOutput();

    std::vector<float> GetOutputData() const;

private:

    std::shared_ptr<BufferResource> CreateBufferResource(const std::string& name, const size_t& size);
    std::shared_ptr<BufferResource> GetBufferResource(const std::string& name);
    std::shared_ptr<BufferResource> GetInputBufferResource();
    std::vector<float> LoadTensor(const onnx::TensorProto& tensor);
    void UpdateInputBuffer(const std::vector<float>& inputData);

    void CreateGemmNodeBuffers(const Node& node);
    void CreateUnaryNodeBuffers(const Node& node);

    void OnnxModelResource::ChangeResourceState(std::shared_ptr<BufferResource>& resource, const D3D12_RESOURCE_STATES& state);

	Graph mGraph;

    Microsoft::WRL::ComPtr<ID3D12Resource> mReadbackBuffer;
    size_t mReadbackBytes = 0;

    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
    
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mGemmRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mReluRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mLeakyReluRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mSigmoidRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mTanhRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mSoftmaxRootSignature = nullptr;
	
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mGemmPSO = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mReluPSO = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mLeakyReluPSO = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mSigmoidPSO = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mTanhPSO = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mSoftmaxPSO = nullptr;


};