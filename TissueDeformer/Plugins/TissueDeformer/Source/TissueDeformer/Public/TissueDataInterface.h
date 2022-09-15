// Copyright 2022 Joseph Chittock. All Rights Reserved.

#pragma once

#include "OptimusComputeDataInterface.h"
#include "ComputeFramework/ComputeDataProvider.h"
#include "TissueDataInterface.generated.h"

class FRHIShaderResourceView;
class FSkeletalMeshObject;
class UTissueComponent;
class USkeletalMeshComponent;

// Tissue data interface for optimus framework
UCLASS(Category = ComputeFramework)
class UTissueDataInterface : public UOptimusComputeDataInterface
{
	GENERATED_BODY()

public:
	FString GetDisplayName() const override;
	TArray<FOptimusCDIPinDefinition> GetPinDefinitions() const override;
	void GetSupportedInputs(TArray<FShaderFunctionDefinition>& OutFunctions) const override;
	void GetShaderParameters(TCHAR const* UID, FShaderParametersMetadataBuilder& OutBuilder) const override;
	void GetHLSL(FString& OutHLSL) const override;
	void GetSourceTypes(TArray<UClass*>& OutSourceTypes) const override;
	UComputeDataProvider* CreateDataProvider(TArrayView<TObjectPtr<UObject>> InSourceObjects, uint64 InInputMask, uint64 InOutputMask) const override;
};

// Tissue data provider for optimus framework
UCLASS(BlueprintType, editinlinenew, Category = ComputeFramework)
class UTissueDataProvider : public UComputeDataProvider
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Binding)
		TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	bool IsValid() const override;
	FComputeDataProviderRenderProxy* GetRenderProxy() override;
};

// Tissue data proxy for optimus framework
class FTissueDataProviderProxy : public FComputeDataProviderRenderProxy
{
public:
	FTissueDataProviderProxy(USkeletalMeshComponent* SkeletalMeshComponent, UTissueComponent* DeformerComponent);
	void GetBindings(int32 InvocationIndex, TCHAR const* UID, FBindings& OutBindings) const override;

private:
	FSkeletalMeshObject* SkeletalMeshObject;
	FRHIShaderResourceView* VertexAdjacencyMapBufferSRV = nullptr;
};
