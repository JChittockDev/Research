#pragma once

#include "../Render/Resources/FrameResource.h"
#include "../Render/Resources/Mesh.h"
#include "../Render/Passes/ShadowMap.h"
#include "../Render/Passes/Ssao.h"
#include "../Render/Resources/RenderItem.h"
#include "../Render/Resources/Animation.h"
#include "../Common/Structures.h"
#include <DirectXColors.h>

using Microsoft::WRL::ComPtr;

class RenderCore
{

public:
	void RenderCore::Initialize(
		ID3D12Resource* inCBB,
		const D3D12_CPU_DESCRIPTOR_HANDLE& inCbbv,
		const D3D12_CPU_DESCRIPTOR_HANDLE& inDsv,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE& inNullSrv,
		ID3D12GraphicsCommandList* inCommandList,
		ID3D12RootSignature* inRootSignature,
		ID3D12RootSignature* inSsaoRootSignature,
		ID3D12DescriptorHeap* inSrvDescriptorHeap,
		const std::unordered_map<std::string, std::pair<INT, UINT>>& inLayoutIndicies,
		const std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>& inPsos,
		const std::vector<std::shared_ptr<RenderItem>>& renderItems,
		const std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemLayers,
		const DynamicLights& inLights
	);

	void Render(FrameResource* currentFrameResource);
	void SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource);
	void ShadowPass(const std::shared_ptr<DynamicLights>& lights, FrameResource* currentFrameResource);
	void DepthPass(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, FrameResource* currentFrameResource);
	void SsaoPass(int blurCount, FrameResource* currentFrameResource);
	void DiffusePass(const std::shared_ptr<std::unordered_map<std::string, std::pair<INT, UINT>>>& layoutIndexMap, FrameResource* currentFrameResource);

	D3D12_VIEWPORT screenViewport;
	D3D12_RECT screenScissorRect;
	
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12RootSignature* rootSignature = nullptr;
	ID3D12RootSignature* ssaoRootSignature = nullptr;
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;
	
	CD3DX12_GPU_DESCRIPTOR_HANDLE nullSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv;
	D3D12_CPU_DESCRIPTOR_HANDLE cbbv;
	ID3D12Resource* cBB = nullptr;

	std::unique_ptr<Ssao> ssao;
	std::vector<std::unique_ptr<ShadowMap>> shadowMaps;

	std::shared_ptr<DynamicLights> lights;
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> renderItems;
	std::shared_ptr<std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>> renderItemLayers;
	std::shared_ptr<std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>> psos;
	std::shared_ptr<std::unordered_map<std::string, std::pair<INT, UINT>>> layoutIndicies;
};