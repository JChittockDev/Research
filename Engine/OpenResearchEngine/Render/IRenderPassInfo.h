#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "../Common/Structures.h"

class IRenderPassInfo
{
public:
    virtual ~IRenderPassInfo() = default;
    IRenderPassInfo(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);

    UINT Width()const;
    UINT Height()const;

    void OnResize(UINT newWidth, UINT newHeight);

    void Build();

private:

};
