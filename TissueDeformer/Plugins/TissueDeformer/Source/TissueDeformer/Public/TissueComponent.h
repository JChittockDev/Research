// Copyright 2022 Joseph Chittock. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "RenderResource.h"
#include "OptimusDeformer.h"
#include "Animation/MeshDeformer.h"
#include "RenderCommandFence.h"
#include "UObject/ObjectMacros.h"
#include "TetgenInterface.h"
#include "TissueBuffer.h"
#include "Components/ActorComponent.h"
#include "TissueComponent.generated.h"

// Tissue component base class to retrieve vertex information
UCLASS(Blueprintable, ClassGroup = Component, BlueprintType, meta = (BlueprintSpawnableComponent))
class UTissueComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere)
	float gravity;
	UPROPERTY(EditAnywhere)
	float drag;
	UPROPERTY(EditAnywhere)
	int substeps;
	UPROPERTY(EditAnywhere)
	float volumeCompliance;
	UPROPERTY(EditAnywhere)
	float edgeCompliance;
	UPROPERTY(EditAnywhere)
	float timeStep;
	UPROPERTY(EditAnywhere)
	float radiusEdgeRatio = 1.5;
	UPROPERTY(EditAnywhere)
	float volumeConstraint = 2.0;
	UPROPERTY(EditAnywhere)
	FVector volumeCavityLocation;


public:
	UTissueComponent();
	TMap<const int, float> ReturnFlatVertexPositions(TMap<const int, FVector>& input);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Serialize(FArchive& Archive) override;
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual bool IsReadyForFinishDestroy() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	void InitGPUData();
	void ReleaseGPUData();
	void GetSkeletalMeshData();

public:
	int TetVertexCount;
	
	TissueFloatVertexBuffer TetVertexPositionsBuffer;
	
	
	TissueFloatVertexBuffer PreviousTetVertexPositionsBuffer;
	TissueFloatVertexBuffer TetVertexVelocityBuffer;
	TissueFloatVertexBuffer InverseMassBuffer;
	TissueFloatVertexBuffer RestVolumeBuffer;
	TissueFloatVertexBuffer EdgeLengthsBuffer;
	TissueFloatVertexBuffer GradientsBuffer;
	TissueFloatVertexBuffer TempBuffer;
	TissueIntegerVertexBuffer TetIDsBuffer;
	TissueIntegerVertexBuffer TetEdgeIDsBuffer;


private:
	FRenderCommandFence RenderResourceDestroyFence;
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	UOptimusDeformer* SkeletalMeshDeformer = nullptr;
	TetgenInterface TetInterface;
};
