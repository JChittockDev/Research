#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <DirectXCollision.h>
#include "../D3D12/D3Dx12.h"
#include "../D3D12/D3DUtil.h"
#include "Math.h"
#include "UploadBuffer.h"
#include "../Models/External/DDSTextureLoader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <math.h>
#include "../Objects/ViewFrustum.h"

#define MaxLights 16

extern const int gNumFrameResources;

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = Math::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = Math::Identity4x4();
    UINT     MaterialIndex;
    UINT     ObjPad0;
    UINT     ObjPad1;
    UINT     ObjPad2;
};

struct SkinnedConstants
{
    DirectX::XMFLOAT4X4 BoneTransforms[55];
};

struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.0f, 0.0f, 0.0f };
    float FalloffStart = 1.0f;
    DirectX::XMFLOAT3 Direction = { 0.0f, 0.0f, 0.0f };
    float FalloffEnd = 100.0f;                         
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
    float InnerConeAngle = 5.0f;
    float OuterConeAngle = 250.0f;
    float Pad1 = 0.0f;
    float Pad2 = 0.0f;
    float Pad3 = 0.0f;
};

struct LightTransform
{
    DirectX::XMFLOAT3 ReferencePosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT4X4 ViewMatrix = Math::Identity4x4();
    DirectX::XMFLOAT4X4 ProjectionMatrix = Math::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProjectionMatrix = Math::Identity4x4();
    float NearZ = 0.1f;
    float FarZ = 100.0f;
};

class DynamicLights
{
public:
    std::vector<Light> DirectionalLights;
    std::vector<Light> PointLights;
    std::vector<Light> SpotLights;
    std::vector<LightTransform> LightTransforms;
    int GetNumLights() { return (int)(DirectionalLights.size() + PointLights.size() + SpotLights.size()); }
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = Math::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = Math::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = Math::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = Math::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = Math::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = Math::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProjTex = Math::Identity4x4();
    DirectX::XMFLOAT4X4 LightTransforms[MaxLights];
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];
};

struct SsaoConstants
{
    DirectX::XMFLOAT4X4 Proj;
    DirectX::XMFLOAT4X4 InvProj;
    DirectX::XMFLOAT4X4 ProjTex;
    DirectX::XMFLOAT4   OffsetVectors[14];

    // For SsaoBlur.hlsl
    DirectX::XMFLOAT4 BlurWeights[3];

    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

    // Coordinates given in view space.
    float OcclusionRadius = 0.5f;
    float OcclusionFadeStart = 0.2f;
    float OcclusionFadeEnd = 2.0f;
    float SurfaceEpsilon = 0.05f;
};

struct MaterialData
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.5f;

    // Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = Math::Identity4x4();

    UINT DiffuseMapIndex = 0;
    UINT NormalMapIndex = 0;
    UINT MaterialPad1;
    UINT MaterialPad2;
};

struct Vertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT4 TangentU;

    Vertex()
    {
        Pos = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
        Normal = DirectX::XMFLOAT3(0.0, 1.0, 0.0);
        TexC = DirectX::XMFLOAT2(0.0, 0.0);
        TangentU = DirectX::XMFLOAT4(1.0, 0.0, 0.0, 0.0);
    }
};

struct SkinningInfo
{
    DirectX::XMFLOAT4 BoneWeights;
    BYTE BoneIndices[4];
};

struct Subset
{
    UINT Id = -1;
    UINT VertexStart = 0;
    UINT VertexCount = 0;
    UINT IndexStart = 0;
    UINT IndexCount = 0;
    UINT MaterialIndex = 0;
    std::string MeshName;
};

struct ModelMaterial
{
    std::string Name;

    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.8f;
    bool AlphaClip = false;

    std::string MaterialTypeName;
    std::string DiffuseMapName;
    std::string NormalMapName;
};

struct SkinWeight
{
    std::vector<float> BoneWeights;
    std::vector<int> BoneIndices;
};

// Defines a subrange of geometry in a MeshGeometry.This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
    UINT VertexCount = 0;
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    INT BaseVertexLocation = 0;
    UINT MaterialIndex = 0;

    // Bounding box of the geometry defined by this submesh. 
    // This is used in later chapters of the book.
    DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
    // Give it a name so we can look it up by name.
    std::string Name;

    // System memory copies.  Use Blobs because the vertex/index format can be generic.
    // It is up to the client to cast appropriately.  
    Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SkinningBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SkinnedVertexBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinnedVertexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinnedVertexBufferUploader = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;
    UINT SkinningByteStride = 0;
    UINT SkinningBufferByteSize = 0;

    // A MeshGeometry may store multiple geometries in one vertex/index buffer.
    // Use this container to define the Submesh geometries so we can draw
    // the Submeshes individually.
    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = VertexByteStride;
        vbv.SizeInBytes = VertexBufferByteSize;

        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = IndexFormat;
        ibv.SizeInBytes = IndexBufferByteSize;

        return ibv;
    }

    D3D12_VERTEX_BUFFER_VIEW SkinningBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW svbv;
        svbv.BufferLocation = SkinningBufferGPU->GetGPUVirtualAddress();
        svbv.StrideInBytes = SkinningByteStride;
        svbv.SizeInBytes = SkinningBufferByteSize;

        return svbv;
    }

    D3D12_VERTEX_BUFFER_VIEW SkinnedVertexBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW svbv;
        svbv.BufferLocation = SkinnedVertexBufferGPU->GetGPUVirtualAddress();
        svbv.StrideInBytes = VertexByteStride;
        svbv.SizeInBytes = VertexBufferByteSize;

        return svbv;
    }

    // We can free this memory after we finish upload to the GPU.
    void DisposeUploaders()
    {
        VertexBufferUploader = nullptr;
        IndexBufferUploader = nullptr;
        SkinningBufferUploader = nullptr;
        SkinnedVertexBufferUploader = nullptr;
    }
};

struct MaterialConstants
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.25f;

    // Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = Math::Identity4x4();
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
    // Unique material name for lookup.
    std::string Name;

    // Index into constant buffer corresponding to this material.
    int MatCBIndex = -1;

    // Index into SRV heap for diffuse texture.
    int DiffuseSrvHeapIndex = -1;

    // Index into SRV heap for normal texture.
    int NormalSrvHeapIndex = -1;

    // Dirty flag indicating the material has changed and we need to update the constant buffer.
    // Because we have a material constant buffer for each FrameResource, we have to apply the
    // update to each FrameResource.  Thus, when we modify a material we should set 
    // NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
    int NumFramesDirty = gNumFrameResources;

    // Material constant buffer data used for shading.
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = .25f;
    DirectX::XMFLOAT4X4 MatTransform = Math::Identity4x4();
};

struct Texture
{
    // Unique material name for lookup.
    std::string Name;

    std::wstring Filename;

    Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

struct AnimationNode
{
    std::string name;
    std::vector<std::unique_ptr<aiVectorKey>> positionKeys;
    std::vector<std::unique_ptr<aiQuatKey>> rotationKeys;
    std::vector<std::unique_ptr<aiVectorKey>> scalingKeys;
};

struct Animation
{
    std::string name;
    std::unordered_map<std::string, std::unique_ptr<AnimationNode>> animationNodes;
    float duration;
};

struct ItemData
{
    std::string item_name;
    std::string geometry;
    std::string mesh;
    std::string animation;
    std::string material;
    std::vector<double> position;
    std::vector<double> rotation;
    std::vector<double> scale;
    std::string render_layer;
};

struct LevelItemData
{
    std::unique_ptr<std::unordered_map<std::string, ItemData>> renderItemDataDict;
};

struct LevelAssetData
{
    std::unique_ptr<LevelItemData> renderItemData;
};

struct LevelData
{
    std::string name;
    std::unique_ptr<LevelAssetData> data;
};