// Copyright 2022 Joseph Chittock. All Rights Reserved.

#include "TissueComponent.h"
#include "Components/SkeletalMeshComponent.h"

UTissueComponent::UTissueComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Function to initiliaze vertex data on serialization
void UTissueComponent::Serialize(FArchive& Archive)
{
#if WITH_EDITOR
	if (Archive.IsSaving() && Archive.IsPersistent())
	{
		InitGPUData();
	}
#endif

	Super::Serialize(Archive);
}

// Function to again initiliaze vertex data after post load
void UTissueComponent::PostLoad()
{
	Super::PostLoad();
	InitGPUData();
}

// Destructor to release gpu data to avoid memory leaks
void UTissueComponent::BeginDestroy()
{
	Super::BeginDestroy();
	ReleaseGPUData();
}

#if WITH_EDITOR
// Attribute function to initialize vertex data if the skeletal mesh or skeletal mesh deformer have changed
void UTissueComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName SkeletalMeshName = GET_MEMBER_NAME_CHECKED(UTissueComponent, SkeletalMeshComponent);
	static const FName SkeletalDeformerName = GET_MEMBER_NAME_CHECKED(UTissueComponent, SkeletalMeshDeformer);
	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	
	if (PropertyName == SkeletalMeshName || PropertyName == SkeletalDeformerName)
	{
		InitGPUData();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// Query if class is ready for destruction
bool UTissueComponent::IsReadyForFinishDestroy()
{
	return Super::IsReadyForFinishDestroy() && RenderResourceDestroyFence.IsFenceComplete();
}

// Function to gather query the current skeletal mesh component and retrieve the mesh deformer
void UTissueComponent::GetSkeletalMeshData()
{
	AActor* Actor = Cast<AActor>(GetOuter());
	SkeletalMeshComponent = Actor->FindComponentByClass<USkeletalMeshComponent>();
	UMeshDeformer* DeformerComponent = SkeletalMeshComponent->MeshDeformer;
	SkeletalMeshDeformer = Cast<UOptimusDeformer>(DeformerComponent);
}

void UTissueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TMap<const int, float> UTissueComponent::ReturnFlatVertexPositions(TMap<const int, FVector>& input)
{
	TMap<const int, float> output;
	for (int i = 0; i < input.Num(); i++)
	{
		output[i * 3] = input[i].X;
		output[i * 3 + 1] = input[i].Y;
		output[i * 3 + 2] = input[i].Z;
	}
	return output;
}



// Functions to concatenate the process of initiliazing & destorying resources
void UTissueComponent::InitGPUData()
{
	GetSkeletalMeshData();
	if (SkeletalMeshComponent)
	{
		TArray<TArray<unsigned int>> TriangleIds;
		FSkeletalMeshRenderData* RenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
		TetInterface.GenerateTetMesh(radius_edge_ratio, volume_constraint, RenderData, { {} });
		
		BeginReleaseResource(&InputVertexPositionsBuffer);
		BeginReleaseResource(&TetVertexPositionsBuffer);

		InputVertexPositionsBuffer.Init(ReturnFlatVertexPositions(*TetInterface.GetInputVertexPositions()), "inputVertexPositionBuffer");
		InputVertexPositionsBuffer.Init(ReturnFlatVertexPositions(*TetInterface.GetTetVertexPositions()), "tetVertexPositionBuffer");
		
		BeginInitResource(&InputVertexPositionsBuffer);
		BeginInitResource(&TetVertexPositionsBuffer);
	}
}


void UTissueComponent::ReleaseGPUData()
{
	BeginReleaseResource(&InputVertexPositionsBuffer);
	BeginReleaseResource(&TetVertexPositionsBuffer);
	RenderResourceDestroyFence.BeginFence();
}