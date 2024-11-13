#pragma once

#include <map>
#include <set>
#include <utility>
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
#include <algorithm>
#include "../ImGui/imgui_internal.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../ImGui/imgui_impl_dx12.h"


#define MaxLights 16

extern const int gNumFrameResources;


struct BlendshapeVertex
{
    UINT index = 0;
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 tangent = { 0.0f, 0.0f, 0.0f };
    float padding1 = 0.0;
    float padding2 = 0.0;
};

struct Blendshape
{
    UINT index = 0;
    std::vector<BlendshapeVertex> vertexData;
};

struct Edge
{
    UINT vertexA;
    UINT vertexB;

    Edge() {}

    Edge(UINT vertexA, UINT vertexB)
    {
        this->vertexA = vertexA;
        this->vertexB = vertexB;
    }

    bool operator==(Edge edge) const
    {
        return (vertexA == edge.vertexA && vertexB == edge.vertexB) || (vertexA == edge.vertexB && vertexB == edge.vertexA);
    }

    bool operator<(const Edge& edge) const
    {
        return (vertexA < edge.vertexA) || (vertexA == edge.vertexA && vertexB < edge.vertexB);
    }
};

struct EdgeHash
{
    std::size_t operator()(const Edge& edge) const
    {
        return std::hash<UINT>()(edge.vertexA) ^ std::hash<UINT>()(edge.vertexB);
    }
};


struct TangentNormals
{
    DirectX::XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 tangent = { 0.0f, 0.0f, 0.0f };
};

struct Neighbours
{
    UINT index[8];
};

struct VertexNeighbours
{
    UINT index[8];
    float length[8];
};

struct UINT3
{
    UINT x = 0;
    UINT y = 0;
    UINT z = 0;

    UINT3() {};

    UINT3(UINT inX, UINT inY, UINT inZ)
    {
        x = inX;
        y = inY;
        z = inZ;
    }
};

struct Vector3
{
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;

    Vector3() {};

    Vector3(float inX, float inY, float inZ)
    {
        x = inX;
        y = inY;
        z = inZ;
    }
};

struct Vector4
{
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float w = 0.0;

    Vector4() {};

    Vector4(float inX, float inY, float inZ, float inW)
    {
        x = inX;
        y = inY;
        z = inZ;
        w = inW;
    }
};

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

struct BlendConstants
{
    Vector4 Weights[64];
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
    UINT RoughnessMapIndex = 0;
    UINT MetalnesssMapIndex = 0;
    UINT SpecularsMapIndex = 0;
    UINT HeightsMapIndex = 0;
    UINT OpacitysMapIndex = 0;
    UINT OcclusionsMapIndex = 0;
    UINT RefractionsMapIndex = 0;
    UINT EmissivesMapIndex = 0;
    UINT SubsurfacesMapIndex = 0;
    UINT MaterialPad1 = 0;
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
    DirectX::XMFLOAT4 BoneIndices;
};

struct BlendshapeSubset
{
    UINT VertexStart = 0;
    UINT VertexCount = 0;
};


struct Subset
{
    UINT VertexStart = 0;
    UINT VertexCount = 0;
    UINT IndexStart = 0;
    UINT IndexCount = 0;
    UINT TriangleStart = 0;
    UINT TriangleCount = 0;

    UINT SimMeshVertexStart = 0;
    UINT SimMeshVertexCount = 0;
    UINT SimMeshIndexStart = 0;
    UINT SimMeshIndexCount = 0;

    UINT SimMeshConstraintStart = 0;
    UINT SimMeshConstraintCount = 0;

    UINT SimMeshTriangleStart = 0;
    UINT SimMeshTriangleCount = 0;

    std::string Name;

    UINT BlendshapeVertexStart = 0;
    UINT BlendshapeVertexCount = 0;
    UINT BlendshapeCount = 0;
    UINT BlendshapeStart = 0;
    std::vector<BlendshapeSubset> BlendshapeSubsets;
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
    std::string Name;

    UINT VertexCount = 0;
    UINT IndexCount = 0;
    UINT TriangleCount = 0;
    UINT IndexStart = 0;
    UINT VertexStart = 0;
    UINT TriangleStart = 0;
    UINT BlendshapeVertexStart = 0;
    UINT BlendshapeVertexCount = 0;
    UINT BlendshapeCount = 0;
    UINT BlendshapeStart = 0;
    std::vector<BlendshapeSubset> BlendshapeSubsets;

    UINT SimMeshVertexCount = 0;
    UINT SimMeshIndexCount = 0;

    UINT SimMeshConstraintStart = 0;
    UINT SimMeshConstraintCount = 0;

    UINT SimMeshTriangleCount = 0;
    UINT SimMeshIndexStart = 0;
    UINT SimMeshVertexStart = 0;
    UINT SimMeshTriangleStart = 0;

    DirectX::BoundingBox Bounds;
};

struct MeshDeformationData
{
    std::vector<Vertex> vertices;
    std::vector<TangentNormals> normals;
    std::vector<Vertex> simMeshVertices;
    std::vector<Vector3> simMeshForce;
    std::vector<Vector3> simMeshSolverAccumulation;
    std::vector<UINT> simMeshSolverCount;
    std::vector<Vector4> simMeshTension;
};

struct MeshGeometry
{
    // Give it a name so we can look it up by name.
    std::string Name;

    MeshDeformationData DeformationData;

    // Default Buffers
    Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SkinningBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> BlendshapeBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendshapeBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendshapeBufferUploader = nullptr;

    // Simulation Buffers

    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshVertexColorBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshVertexNeighbourBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshConstraintsBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshConstraintIDsBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshNullSolverAccumulationBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshNullSolverCountBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshTransferBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> MeshTransferBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexColorBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexNeighbourBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintIDsBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverAccumulationBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverCountBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshTransferBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> MeshTransferBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexColorBufferUploader = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexNeighbourBufferUploader = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintIDsBufferUploader = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverAccumulationBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverCountBufferUploader = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshTransferBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> MeshTransferBufferUploader = nullptr;
    
    // Normal & Tangent Calculation Buffers
    Microsoft::WRL::ComPtr<ID3DBlob> TriangleAdjacencyBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleAdjacencyBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleAdjacencyBufferUploader = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;

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

    // We can free this memory after we finish upload to the GPU.
    void DisposeUploaders()
    {
        VertexBufferUploader = nullptr;
        IndexBufferUploader = nullptr;
        SimMeshVertexColorBufferUploader = nullptr;
        SimMeshVertexBufferUploader = nullptr;
        SimMeshVertexNeighbourBufferUploader = nullptr;
        SimMeshConstraintsBufferUploader = nullptr;
        SimMeshConstraintIDsBufferUploader = nullptr;
        SimMeshNullSolverAccumulationBufferUploader = nullptr;
        SimMeshNullSolverCountBufferUploader = nullptr;
        SimMeshTransferBufferUploader = nullptr;
        MeshTransferBufferUploader = nullptr;
        TriangleAdjacencyBufferUploader = nullptr;
    }
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
    // Unique material name for lookup.
    std::string Name;

    // Index into constant buffer corresponding to this material.
    int MaterialIndex = -1;

    // Index into SRV heap for diffuse texture.
    int DiffuseSrvHeapIndex = -1;
    int NormalSrvHeapIndex = -1;
    int RoughnessSrvHeapIndex = -1;
    int MetalnessSrvHeapIndex = -1;
    int SpecularSrvHeapIndex = -1;
    int HeightSrvHeapIndex = -1;
    int OpacitySrvHeapIndex = -1;
    int OcclusionSrvHeapIndex = -1;
    int RefractionSrvHeapIndex = -1;
    int EmissiveSrvHeapIndex = -1;
    int SubsurfaceSrvHeapIndex = -1;

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

    std::string Type;
};

struct TransformAnimNode
{
    std::string name;
    std::vector<std::unique_ptr<aiVectorKey>> positionKeys;
    std::vector<std::unique_ptr<aiQuatKey>> rotationKeys;
    std::vector<std::unique_ptr<aiVectorKey>> scalingKeys;
};


struct MorphKey
{
    double mTime;
    std::vector<UINT> mValues;
    std::vector<double> mWeights;
    UINT mNumValuesAndWeights;

    MorphKey(const aiMeshMorphKey& input)
    {
        mTime = input.mTime;

        for (UINT z = 0; z < input.mNumValuesAndWeights; ++z)
        {
            mValues.push_back(input.mValues[z]);
            mWeights.push_back(input.mWeights[z]);
        }

        mNumValuesAndWeights = input.mNumValuesAndWeights;

    }
};

struct BlendAnimNode
{
    std::string name;
    UINT blendsStart;
    std::vector<MorphKey> weightKeys;
};

struct Animation
{
    std::string name;
    std::map<std::string, std::unique_ptr<TransformAnimNode>> TransformAnimNodes;
    std::map<std::string, std::unique_ptr<BlendAnimNode>> BlendAnimNodes;
    float duration;
};

struct RenderItemSettings
{
    bool Simulation = false;
    std::string SimulationMask = "none";
    std::string Material = "default";
};

struct ItemData
{
    std::string item_name;
    std::string geometry;
    std::string animation;
    bool deformable;
    std::vector<double> position;
    std::vector<double> rotation;
    std::vector<double> scale;
    std::string render_layer;
    std::unordered_map<std::string, RenderItemSettings> settings;
};

struct LevelItemData
{
    std::unique_ptr<std::unordered_map<std::string, ItemData>> renderItemDataDict;
};

struct PBRMaterialData
{
    std::string name;
    std::string diffuse_tex_path;
    std::string normal_tex_path;
    std::string roughness_tex_path;
    std::string metalness_tex_path;
    std::string specular_tex_path;
    std::string height_tex_path;
    std::string opacity_tex_path;
    std::string occlusion_tex_path;
    std::string refraction_tex_path;
    std::string emissive_tex_path;
    std::string subsurface_tex_path;
};

struct LightData
{
    std::string type;
    std::vector<double> position;
    std::vector<double> direction;
    std::vector<double> strength;
    double falloff_start;
    double falloff_end;
    double inner_cone_angle;
    double outer_cone_angle;
};

struct LevelMaterialData
{
    std::unique_ptr<std::unordered_map<std::string, PBRMaterialData>> pbrMaterialDataDict;
};

struct LevelLightData
{
    std::unique_ptr<std::unordered_map<std::string, LightData>> lightDataDict;
};

struct LevelAssetData
{
    std::unique_ptr<LevelItemData> renderItemData;
};

struct LevelData
{
    std::string name;
    std::unique_ptr<LevelAssetData> data;
    std::unique_ptr<LevelMaterialData> materialData;
    std::unique_ptr<LevelLightData> lightData;
};