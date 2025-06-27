#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT blendObjectCount, UINT skinnedObjectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));
    PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    SsaoCB = std::make_unique<UploadBuffer<SsaoConstants>>(device, 1, true);
    SsgiCB = std::make_unique<UploadBuffer<SsgiConstants>>(device, 1, true);
    SssCB = std::make_unique<UploadBuffer<SssConstants>>(device, 1, true);
    SsaoVerticalBlurCB = std::make_unique<UploadBuffer<SsaoBlurConstants>>(device, 1, true);
    SsaoHorizontalBlurCB = std::make_unique<UploadBuffer<SsaoBlurConstants>>(device, 1, true);
    SsgiVerticalBlurCB = std::make_unique<UploadBuffer<SsgiBlurConstants>>(device, 1, true);
    SsgiHorizontalBlurCB = std::make_unique<UploadBuffer<SsgiBlurConstants>>(device, 1, true);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, false);
    ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
    BlendCB = std::make_unique<UploadBuffer<BlendConstants>>(device, blendObjectCount, true);
    SkinnedCB = std::make_unique<UploadBuffer<SkinnedConstants>>(device, skinnedObjectCount, true);
}

FrameResource::~FrameResource()
{

}