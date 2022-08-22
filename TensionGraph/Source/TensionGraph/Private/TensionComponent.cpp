// Tension Graph - Joseph Chittock

#include "TensionComponent.h"
#include "Components/SkeletalMeshComponent.h"

UTensionComponent::UTensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Function to initiliaze vertex data on serialization
void UTensionComponent::Serialize(FArchive& Archive)
{
#if WITH_EDITOR
	if (Archive.IsSaving() && Archive.IsPersistent())
	{
		InitVertexMap();
		InitGPUData();
	}
#endif

	Super::Serialize(Archive);
}

// Function to again initiliaze vertex data after post load
void UTensionComponent::PostLoad()
{
	Super::PostLoad();
	InitVertexMap();
	InitGPUData();
}

// Destructor to release gpu data to avoid memory leaks
void UTensionComponent::BeginDestroy()
{
	Super::BeginDestroy();
	ReleaseGPUData();
}

// Attribute function to initialize vertex data if the skeletal mesh or skeletal mesh deformer have changed
void UTensionComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName SkeletalMeshName = GET_MEMBER_NAME_CHECKED(UTensionComponent, SkeletalMeshComponent);
	static const FName SkeletalDeformerName = GET_MEMBER_NAME_CHECKED(UTensionComponent, SkeletalMeshDeformer);
	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	
	if (PropertyName == SkeletalMeshName || PropertyName == SkeletalDeformerName)
	{
		InitVertexMap();
		InitGPUData();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

// Query if class is ready for destruction
bool UTensionComponent::IsReadyForFinishDestroy()
{
	return Super::IsReadyForFinishDestroy() && RenderResourceDestroyFence.IsFenceComplete();
}

// Function to gather query the current skeletal mesh component and retrieve the mesh deformer
void UTensionComponent::GetSkeletalMeshData()
{
	AActor* Actor = Cast<AActor>(GetOuter());
	SkeletalMeshComponent = Actor->FindComponentByClass<USkeletalMeshComponent>();
	UMeshDeformer* DeformerComponent = SkeletalMeshComponent->MeshDeformer;
	SkeletalMeshDeformer = Cast<UOptimusDeformer>(DeformerComponent);
}

void UTensionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Initialize the vertex adjacnecy information
void UTensionComponent::InitVertexMap()
{
	VertexAdjacencyMap.Empty();
	GetSkeletalMeshData();
	
	if (SkeletalMeshComponent)
	{
		TArray<TArray<unsigned int>> TriangleIds;
		FSkeletalMeshRenderData* RenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
		int VertexCount = RenderData->LODRenderData[0].GetNumVertices();
		GetTriangleIDVertexIDData(RenderData, TriangleIds);
		FindAdjacentVertices(VertexCount, TriangleIds, VertexAdjacencyMap);
	}
}

// Store the vertex indicies for each triangle in an array
void UTensionComponent::GetTriangleIDVertexIDData(FSkeletalMeshRenderData* RenderData, TArray<TArray<unsigned int>>& OutputData)
{
	FRawStaticIndexBuffer16or32Interface& IndexBuffer = *(RenderData->LODRenderData[0].MultiSizeIndexContainer.GetIndexBuffer());
	int triangles = IndexBuffer.Num() / 3;
	
	for (int x = 0; x < triangles; x++)
	{
		OutputData.Add({ IndexBuffer.Get(x * 3), IndexBuffer.Get(x * 3 + 1), IndexBuffer.Get(x * 3 + 2) });
	}
}

// Find the adjacent vertices for each vertex on mesh
void UTensionComponent::FindAdjacentVertices(int VertexCount, TArray<TArray<unsigned int>>& InputData, TArray<int>& OutputData)
{
	for (int a = 0; a < VertexCount; a++)
	{
		TArray<int> AdjacentVertices;
		for (int b = 0; b < InputData.Num(); b++)
		{
			bool check = false;
			int TriangleVertexCount = InputData[b].Num();
			for (int c = 0; c < TriangleVertexCount; c++)
			{
				if (a == InputData[b][c])
				{
					check = true;
					break;
				}
			}
			
			if (check)
			{
				for (int c = 0; c < TriangleVertexCount; c++)
				{
					if (a != InputData[b][c])
					{
						if (!AdjacentVertices.Contains(InputData[b][c]))
						{
							if (AdjacentVertices.Num() != 4)
							{
								AdjacentVertices.Add(InputData[b][c]);
							}
						}
					}
				}
			}
		}
		
		int CurrentCount = AdjacentVertices.Num();
		if (CurrentCount != 4)
		{
			int remainder = 4 - CurrentCount;
			for (int e = 0; e < remainder; e++)
			{
				AdjacentVertices.Add(a);
			}
		}
		OutputData.Append(AdjacentVertices);
	}
}

// Intialize & Create the a vertex buffer to pass adjacency data
void FVertexAdjacencyMapBuffer::InitRHI()
{
	if (VertexAdjacencyMap.Num() > 0)
	{
		FRHIResourceCreateInfo CreateInfo(TEXT("FVertexAdjacencyMapBuffer"));
		VertexBufferRHI = RHICreateVertexBuffer(VertexAdjacencyMap.Num() * sizeof(uint32), BUF_Static | BUF_ShaderResource, CreateInfo);
		uint32* Data = reinterpret_cast<uint32*>(RHILockBuffer(VertexBufferRHI, 0, VertexAdjacencyMap.Num() * sizeof(uint32), RLM_WriteOnly));
		
		for (int32 Index = 0; Index < VertexAdjacencyMap.Num(); ++Index)
		{
			Data[Index] = static_cast<uint32>(VertexAdjacencyMap[Index]);
		}
		
		RHIUnlockBuffer(VertexBufferRHI);
		VertexAdjacencyMap.Empty();
		ShaderResourceViewRHI = RHICreateShaderResourceView(VertexBufferRHI, 4, PF_R32_UINT);
	}
	else
	{
		VertexBufferRHI = nullptr;
		ShaderResourceViewRHI = nullptr;
	}
}

// Functions to concatenate the process of initiliazing & destorying resources
void UTensionComponent::InitGPUData()
{
	BeginReleaseResource(&VertexAdjacencyMapBuffer);
	VertexAdjacencyMapBuffer.Init(VertexAdjacencyMap);
	BeginInitResource(&VertexAdjacencyMapBuffer);
}


void UTensionComponent::ReleaseGPUData()
{
	BeginReleaseResource(&VertexAdjacencyMapBuffer);
	RenderResourceDestroyFence.BeginFence();
}