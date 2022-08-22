// Copyright 2022 Joseph Chittock. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "RenderResource.h"
#include "OptimusDeformer.h"
#include "Animation/MeshDeformer.h"
#include "RenderCommandFence.h"
#include "UObject/ObjectMacros.h"
#include "Components/ActorComponent.h"
#include "TensionComponent.generated.h"

// Declare buffer containing vertex adjancency information
class FVertexAdjacencyMapBuffer : public FVertexBufferWithSRV
{
public:
	void Init(const TArray<int>& InVertexAdjacencyMap)
	{
		VertexAdjacencyMap = InVertexAdjacencyMap;
	}

private:
	void InitRHI() override;
	TArray<int> VertexAdjacencyMap;
};

// Tension component base class to retrieve vertex information
UCLASS(Blueprintable, ClassGroup = Component, BlueprintType, meta = (BlueprintSpawnableComponent))
class UTensionComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	UTensionComponent();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Serialize(FArchive& Archive) override;
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual bool IsReadyForFinishDestroy() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	void InitVertexMap();
	void InitGPUData();
	void ReleaseGPUData();
	void GetSkeletalMeshData();
	void FindAdjacentVertices(int VertexCount, TArray<TArray<unsigned int>>& InputData, TArray<int>& OutputData);
	void GetTriangleIDVertexIDData(FSkeletalMeshRenderData* RenderData, TArray<TArray<unsigned int>>& OutputData);

public:
	FVertexAdjacencyMapBuffer VertexAdjacencyMapBuffer;

private:
	TArray<int> VertexAdjacencyMap;
	FRenderCommandFence RenderResourceDestroyFence;
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	UOptimusDeformer* SkeletalMeshDeformer = nullptr;
};
