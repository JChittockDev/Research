// Tension Graph - Joseph Chittock

#pragma once

#include "OptimusComputeDataInterface.h"
#include "ComputeFramework/ComputeDataProvider.h"
#include "TensionDataInterface.generated.h"

class FRHIShaderResourceView;
class FSkeletalMeshObject;
class UTensionComponent;
class USkeletalMeshComponent;

// Tension data interface for optimus framework
UCLASS(Category = ComputeFramework)
class UTensionDataInterface : public UOptimusComputeDataInterface
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

// Tension data provider for optimus framework
UCLASS(BlueprintType, editinlinenew, Category = ComputeFramework)
class UTensionDataProvider : public UComputeDataProvider
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Binding)
		TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	bool IsValid() const override;
	FComputeDataProviderRenderProxy* GetRenderProxy() override;
};

// Tension data proxy for optimus framework
class FTensionDataProviderProxy : public FComputeDataProviderRenderProxy
{
public:
	FTensionDataProviderProxy(USkeletalMeshComponent* SkeletalMeshComponent, UTensionComponent* DeformerComponent);
	void GetBindings(int32 InvocationIndex, TCHAR const* UID, FBindings& OutBindings) const override;

private:
	FSkeletalMeshObject* SkeletalMeshObject;
	FRHIShaderResourceView* VertexAdjacencyMapBufferSRV = nullptr;
};
