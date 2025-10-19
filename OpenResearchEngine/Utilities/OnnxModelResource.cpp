#include "OnnxModelResource.h"

std::vector<float> OnnxModelResource::LoadTensor(const onnx::TensorProto& tensor)
{
    std::vector<float> data;
    if (tensor.has_raw_data()) 
    {
        const std::string& raw = tensor.raw_data();
        size_t count = raw.size() / sizeof(float);
        data.resize(count);
        memcpy(data.data(), raw.data(), raw.size());
    }
    else 
    {
        data.assign(tensor.float_data().begin(), tensor.float_data().end());
    }
    return data;
}

std::shared_ptr<BufferResource> OnnxModelResource::GetInputBufferResource()
{
    const auto& inputName = mGraph.inputBuffer;
    return GetBufferResource(inputName);
}

std::shared_ptr<BufferResource> OnnxModelResource::GetBufferResource(const std::string& name)
{
    if (mGraph.buffers.count(name) == 0) {
        throw std::runtime_error("Input buffer not found: " + name);
    }

    return mGraph.buffers[name];
}

std::shared_ptr<BufferResource> OnnxModelResource::CreateBufferResource(const std::string& name, const size_t& size)
{
    std::shared_ptr<BufferResource> buf = std::make_shared<BufferResource>();
    
    // Check if we have initializer data for this buffer
    if (mGraph.initializers.find(name) != mGraph.initializers.end()) {
        // Use the initializer data
        const auto& initData = mGraph.initializers[name];
        buf->buffer = d3dUtil::CreateDefaultBuffer(
            md3dDevice.Get(),
            mCommandList.Get(),
            initData.data(),
            initData.size() * sizeof(float),
            buf->uploader
        );
    } 
    else {
        // No initializer - use zero initialization
        std::vector<float> zeroData(size, 0.0f);
        buf->buffer = d3dUtil::CreateDefaultBuffer(
            md3dDevice.Get(),
            mCommandList.Get(),
            zeroData.data(),
            size * sizeof(float),
            buf->uploader
        );
    }
    
    return std::move(buf);
}

void OnnxModelResource::CreateGemmNodeBuffers(const Node& node)
{
    // Input A
    const auto& inputAName = node.inputs[0];
    const auto& inputAInfo = mGraph.tensors[inputAName];
    size_t inputASize = inputAInfo.shape[0] * inputAInfo.shape[1];
    mGraph.buffers[inputAName] = CreateBufferResource(inputAName, inputASize);

    // Input B
    const auto& inputBName = node.inputs[1];
    const auto& inputBInfo = mGraph.tensors[inputBName];
    size_t inputBSize = inputBInfo.shape[0] * inputBInfo.shape[1];
    mGraph.buffers[inputBName] = CreateBufferResource(inputBName, inputBSize);

    // Bias
    if (node.inputs.size() > 2) {
        const auto& biasName = node.inputs[2];
        const auto& biasInfo = mGraph.tensors[biasName];
        size_t biasSize = biasInfo.shape[0];
        mGraph.buffers[biasName] = CreateBufferResource(biasName, biasSize);
    }

    // Output
    const auto& outputName = node.outputs[0];
    int64_t M = mGraph.tensors[node.inputs[0]].shape[0];
    int64_t N = mGraph.tensors[node.inputs[1]].shape[1];
    size_t outputSize = M * N;
    mGraph.buffers[outputName] = CreateBufferResource(outputName, outputSize);
}

void OnnxModelResource::CreateUnaryNodeBuffers(const Node& node)
{
	// Unary ops: input and output
    const auto& inputName = node.inputs[0];
    const auto& inputInfo = mGraph.tensors[inputName];
    size_t inputSize = 1;
	// Calculate total size given shape
    for (auto dim : inputInfo.shape) if (dim > 0) inputSize *= dim;
	// Create input buffer if it doesn't exist
    if (mGraph.buffers.count(inputName) == 0)
    {
        mGraph.buffers[inputName] = CreateBufferResource(inputName, inputSize);
    }
    const auto& outputName = node.outputs[0];
	// Output buffer same size as input
    if (mGraph.buffers.count(outputName) == 0)
    {
        mGraph.buffers[outputName] = CreateBufferResource(outputName, inputSize);
    }

}

OnnxModelResource::~OnnxModelResource()
{
    // Release all buffer resources in mGraph.buffers
    for (auto& kv : mGraph.buffers) 
    {
        kv.second.reset();
    }

    mGraph.buffers.clear();
    mGraph.tensors.clear();
    mGraph.nodes.clear();
    mGraph.initializers.clear();
}

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
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& softmaxPSO)
    : md3dDevice(device),
      mCommandList(commandList),
      mGemmRootSignature(gemmRootSig),
      mReluRootSignature(reluRootSig),
      mLeakyReluRootSignature(leakyReluRootSig),
      mSigmoidRootSignature(sigmoidRootSig),
      mTanhRootSignature(tanhRootSig),
      mSoftmaxRootSignature(softmaxRootSig),
      mGemmPSO(gemmPSO),
      mReluPSO(reluPSO),
      mLeakyReluPSO(leakyReluPSO),
      mSigmoidPSO(sigmoidPSO),
      mTanhPSO(tanhPSO),
      mSoftmaxPSO(softmaxPSO)
{
};

void OnnxModelResource::Initialize(const std::string& path)
{
    onnx::ModelProto model;
    std::ifstream in(path, std::ios::binary);
    if (!in.good()) {
        throw std::runtime_error("Failed to open ONNX file: " + path);
    }

    if (!model.ParseFromIstream(&in)) {
        throw std::runtime_error("Failed to parse ONNX model.");
    }

    const auto& graph = model.graph();
    mGraph.buffers.clear();
    mGraph.tensors.clear();
    mGraph.nodes.clear();

    // Debug print for graph inputs
    std::cout << "\nGraph inputs:" << std::endl;
    for (const auto& input : graph.input()) {
        std::cout << "Input: " << input.name() << std::endl;
    }

    // First, collect all initializers and their shapes
    for (const auto& init : graph.initializer()) {
        std::cout << "Processing initializer: " << init.name() << std::endl;
        
        // Store the data
        mGraph.initializers[init.name()] = LoadTensor(init);
        
        // Create tensor info
        TensorInfo ti;
        ti.elemType = init.data_type();
        for (const auto& dim : init.dims()) {
            ti.shape.push_back(dim);
        }
        mGraph.tensors[init.name()] = std::move(ti);

        std::cout << "Initializer " << init.name() << " shape: [";
        for (size_t i = 0; i < mGraph.tensors[init.name()].shape.size(); ++i) {
            std::cout << mGraph.tensors[init.name()].shape[i];
            if (i < mGraph.tensors[init.name()].shape.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

    // Collect input tensor info
    auto collectTensorInfo = [&](const onnx::ValueInfoProto& v) {
        if (mGraph.tensors.find(v.name()) != mGraph.tensors.end()) {
            return; // Already have info from initializers
        }

        TensorInfo ti;
        if (v.has_type() && v.type().has_tensor_type()) {
            const auto& tt = v.type().tensor_type();
            ti.elemType = tt.elem_type();
            if (tt.has_shape()) {
                for (auto& dim : tt.shape().dim()) {
                    if (dim.has_dim_value()) {
                        ti.shape.push_back(dim.dim_value());
                    }
                    else {
                        ti.shape.push_back(-1); // unknown
                    }
                }
            }
        }
        mGraph.tensors[v.name()] = std::move(ti);

        // Debug print
        std::cout << "Tensor " << v.name() << " shape: [";
        for (size_t i = 0; i < mGraph.tensors[v.name()].shape.size(); ++i) {
            std::cout << mGraph.tensors[v.name()].shape[i];
            if (i < mGraph.tensors[v.name()].shape.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    };

    // Process inputs first
    for (const auto& input : graph.input()) {
        collectTensorInfo(input);
    }

    // Set input buffer name
    if (!graph.input().empty()) {
        mGraph.inputBuffer = graph.input(0).name();
        std::cout << "Input buffer set to: " << mGraph.inputBuffer << std::endl;
    }

    // Extract and validate nodes
    std::set<std::string> supported = { "Gemm", "Relu", "LeakyRelu", "Sigmoid", "Softmax", "Tanh" };
    
    for (const auto& n : graph.node()) {
        if (supported.count(n.op_type()) == 0) {
            std::cout << "Skipping unsupported op: " << n.op_type() << "\n";
            continue;
        }

        std::cout << "\nProcessing node: " << n.op_type() << std::endl;
        
        Node node;
        node.opType = n.op_type();
        for (auto& in : n.input()) {
            node.inputs.push_back(in);
            std::cout << "Input: " << in << std::endl;
        }
        for (auto& out : n.output()) {
            node.outputs.push_back(out);
            std::cout << "Output: " << out << std::endl;
        }

        // Infer and store output shape
        if (node.opType == "Gemm") {
            const auto& inputA = mGraph.tensors[node.inputs[0]];
            const auto& inputB = mGraph.tensors[node.inputs[1]];
            
            if (inputA.shape.empty() || inputB.shape.empty()) {
                throw std::runtime_error("Missing shape information for Gemm inputs");
            }

            TensorInfo outputInfo;
            outputInfo.elemType = inputA.elemType;
            outputInfo.shape = { inputA.shape[0], inputB.shape[1] };
            mGraph.tensors[node.outputs[0]] = std::move(outputInfo);
            
            std::cout << "Gemm output shape: [" << inputA.shape[0] << ", " << inputB.shape[1] << "]" << std::endl;
        }
        else {
            // For unary ops, output shape matches input
            const auto& input = mGraph.tensors[node.inputs[0]];
            if (input.shape.empty()) {
                throw std::runtime_error("Missing shape information for input to " + node.opType);
            }

            TensorInfo outputInfo;
            outputInfo.elemType = input.elemType;
            outputInfo.shape = input.shape;
            mGraph.tensors[node.outputs[0]] = std::move(outputInfo);

            std::cout << "Unary op output shape: [";
            for (size_t i = 0; i < input.shape.size(); ++i) {
                std::cout << input.shape[i];
                if (i < input.shape.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }

        for (auto& attr : n.attribute()) {
            if (attr.has_f()) {
                node.fAttrs[attr.name()] = attr.f();
            }
        }

        mGraph.nodes.push_back(std::move(node));
    }

    // Create buffers now that all shapes are known
    for (const auto& node : mGraph.nodes) {
        std::cout << "\nCreating buffers for node: " << node.opType << std::endl;
        if (node.opType == "Gemm") {
            CreateGemmNodeBuffers(node);
        }
        else {
            CreateUnaryNodeBuffers(node);
        }
    }

    std::cout << "Loaded ONNX graph: " << mGraph.nodes.size() << " supported nodes.\n";
}

// New method to update the first input buffer before inference
void OnnxModelResource::UpdateInputBuffer(const std::vector<float>& inputData)
{
	auto buf = GetInputBufferResource();
    // Update buffer with new input data
    d3dUtil::UpdateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), buf->buffer.Get(), inputData.data(), inputData.size() * sizeof(float), buf->uploader);
}

// Get output data from the last node's output buffer
std::vector<float> OnnxModelResource::GetOutputData()
{
    if (mGraph.nodes.empty()) {
        throw std::runtime_error("No nodes in graph");
    }

    // Get the last node's output buffer
    const auto& lastNode = mGraph.nodes.back();
    const auto& outputName = lastNode.outputs[0];
    auto& output = mGraph.buffers[outputName];

    ChangeResourceState(output, D3D12_RESOURCE_STATE_COPY_SOURCE);

    // Calculate output size
    const auto& outputInfo = mGraph.tensors[outputName];
    size_t outputSize = 1;
    for (auto dim : outputInfo.shape) if (dim > 0) outputSize *= dim;

    // Create readback buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer;
    CD3DX12_HEAP_PROPERTIES readbackHeapProps(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(outputSize * sizeof(float));
    
    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &readbackHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&readbackBuffer)));

    // Copy data to readback buffer
    mCommandList->CopyResource(readbackBuffer.Get(), output->buffer.Get());

    // Wait for copy to complete and map buffer
    void* mappedData;
    ThrowIfFailed(readbackBuffer->Map(0, nullptr, &mappedData));
    
    // Copy to vector
    std::vector<float> outputData(outputSize);
    memcpy(outputData.data(), mappedData, outputSize * sizeof(float));
    
    // Cleanup
    readbackBuffer->Unmap(0, nullptr);

    return outputData;
}

// function to change resource state and set the current state
void OnnxModelResource::ChangeResourceState(std::shared_ptr<BufferResource>& resource, const D3D12_RESOURCE_STATES& state)
{
    if (resource->currentState != state)
    {
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource->buffer.Get(), resource->currentState, state));
        resource->currentState = state;
    }
}

void OnnxModelResource::Evaluate(const std::vector<float>& inputData)
{
    UpdateInputBuffer(inputData);

    // Process each node in order
    for (size_t nodeIdx = 0; nodeIdx < mGraph.nodes.size(); nodeIdx++) 
    {
        auto& node = mGraph.nodes[nodeIdx];
        auto& input0 = mGraph.buffers[node.inputs[0]];
        auto& output = mGraph.buffers[node.outputs[0]];

        ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);


        if (node.opType == "Gemm")
        {
            auto& input1 = mGraph.buffers[node.inputs[1]];
            ChangeResourceState(input1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            int64_t M = mGraph.tensors[node.inputs[0]].shape[0];
            int64_t K = mGraph.tensors[node.inputs[0]].shape[1];
            int64_t N = mGraph.tensors[node.inputs[1]].shape[1];

            mCommandList->SetComputeRootSignature(mGemmRootSignature.Get());
            mCommandList->SetPipelineState(mGemmPSO.Get());

            struct GemmCBData {
                UINT M;
                UINT N;
                UINT K;
                UINT hasBias;
            } gemmCB = { (UINT)M, (UINT)N, (UINT)K, node.inputs.size() > 2 ? 1u : 0u };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(GemmCBData) / 4, &gemmCB, 0);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());
            mCommandList->SetComputeRootShaderResourceView(2, input1->buffer->GetGPUVirtualAddress());
            
            if (node.inputs.size() > 2)
            {
                auto& inputBias = mGraph.buffers[node.inputs[2]];
                ChangeResourceState(inputBias, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                mCommandList->SetComputeRootShaderResourceView(3, inputBias->buffer->GetGPUVirtualAddress());
            }

            mCommandList->SetComputeRootUnorderedAccessView(4, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSizeX = 16;
            const UINT threadGroupSizeY = 16;
            mCommandList->Dispatch((M + threadGroupSizeX - 1) / threadGroupSizeX,
                (N + threadGroupSizeY - 1) / threadGroupSizeY, 1);
        }
        else if (node.opType == "Relu")
        {
            size_t count = mGraph.tensors[node.inputs[0]].shape[0];
            if (mGraph.tensors[node.inputs[0]].shape.size() > 1)
            {
                count *= mGraph.tensors[node.inputs[0]].shape[1];
            }

            mCommandList->SetComputeRootSignature(mReluRootSignature.Get());
            mCommandList->SetPipelineState(mReluPSO.Get());

            struct UnaryCB {
                UINT Count;
                float pad[3];
            } cb = { (UINT)count };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(UnaryCB) / 4, &cb, 0);

            ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());
            
            ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->SetComputeRootUnorderedAccessView(2, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSize = 64;
            mCommandList->Dispatch((count + threadGroupSize - 1) / threadGroupSize, 1, 1);
        }
        else if (node.opType == "LeakyRelu")
        {
            float alpha = 0.01f;
            if (node.fAttrs.count("alpha"))
            {
                alpha = node.fAttrs.at("alpha");
            }
            
            size_t count = mGraph.tensors[node.inputs[0]].shape[0];
            if (mGraph.tensors[node.inputs[0]].shape.size() > 1)
            {
                count *= mGraph.tensors[node.inputs[0]].shape[1];
            }

            mCommandList->SetComputeRootSignature(mLeakyReluRootSignature.Get());
            mCommandList->SetPipelineState(mLeakyReluPSO.Get());

            struct LeakyReluCB {
                float alpha;
                UINT Count;
                float pad[2];
            } cb = { alpha, (UINT)count };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(LeakyReluCB) / 4, &cb, 0);

            ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());

            ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->SetComputeRootUnorderedAccessView(2, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSize = 64;
            mCommandList->Dispatch((count + threadGroupSize - 1) / threadGroupSize, 1, 1);
        }
        else if (node.opType == "Sigmoid")
        {
            size_t count = mGraph.tensors[node.inputs[0]].shape[0];
            if (mGraph.tensors[node.inputs[0]].shape.size() > 1)
                count *= mGraph.tensors[node.inputs[0]].shape[1];

            mCommandList->SetComputeRootSignature(mSigmoidRootSignature.Get());
            mCommandList->SetPipelineState(mSigmoidPSO.Get());

            struct UnaryCB {
                UINT Count;
                float pad[3];
            } cb = { (UINT)count };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(UnaryCB) / 4, &cb, 0);

            ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());

            ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->SetComputeRootUnorderedAccessView(2, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSize = 64;
            mCommandList->Dispatch((count + threadGroupSize - 1) / threadGroupSize, 1, 1);
        }
        else if (node.opType == "Tanh")
        {
            size_t count = mGraph.tensors[node.inputs[0]].shape[0];
            if (mGraph.tensors[node.inputs[0]].shape.size() > 1)
                count *= mGraph.tensors[node.inputs[0]].shape[1];

            mCommandList->SetComputeRootSignature(mTanhRootSignature.Get());
            mCommandList->SetPipelineState(mTanhPSO.Get());

            struct UnaryCB {
                UINT Count;
                float pad[3];
            } cb = { (UINT)count };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(UnaryCB) / 4, &cb, 0);

            ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());

            ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->SetComputeRootUnorderedAccessView(2, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSize = 64;
            mCommandList->Dispatch((count + threadGroupSize - 1) / threadGroupSize, 1, 1);
        }
        else if (node.opType == "Softmax")
        {
            int M = (int)mGraph.tensors[node.inputs[0]].shape[0];
            int N = (int)mGraph.tensors[node.inputs[0]].shape[1];

            mCommandList->SetComputeRootSignature(mSoftmaxRootSignature.Get());
            mCommandList->SetPipelineState(mSoftmaxPSO.Get());

            struct SoftmaxCB {
                UINT M;
                UINT N;
                float pad[2];
            } cb = { (UINT)M, (UINT)N };

            mCommandList->SetComputeRoot32BitConstants(0, sizeof(SoftmaxCB) / 4, &cb, 0);
            
            ChangeResourceState(input0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->SetComputeRootShaderResourceView(1, input0->buffer->GetGPUVirtualAddress());

            ChangeResourceState(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->SetComputeRootUnorderedAccessView(2, output->buffer->GetGPUVirtualAddress());

            const UINT threadGroupSize = 64;
            mCommandList->Dispatch((M + threadGroupSize - 1) / threadGroupSize, 1, 1);
        }
    }
}