// Tension Graph - Joseph Chittock

#include "TensionDataInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "ComputeFramework/ComputeKernelPermutationSet.h"
#include "ComputeFramework/ShaderParamTypeDefinition.h"
#include "TensionComponent.h"
#include "OptimusDataDomain.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderGraphResources.h"
#include "ShaderParameterMetadataBuilder.h"
#include "SkeletalRenderPublic.h"

// Define display name in optimus graph
FString UTensionDataInterface::GetDisplayName() const
{
	return TEXT("Tension Info");
}

// Declare the node graph pin names
TArray<FOptimusCDIPinDefinition> UTensionDataInterface::GetPinDefinitions() const
{
	TArray<FOptimusCDIPinDefinition> Defs;
	Defs.Add({ "AdjacentIDs", "ReadAdjacentIDs", Optimus::DomainName::Vertex, "ReadNumVertices" });
	
	return Defs;
}

// Declare the node outputs in optimus graph
void UTensionDataInterface::GetSupportedInputs(TArray<FShaderFunctionDefinition>& OutFunctions) const
{
	{
		FShaderFunctionDefinition Fn;
		Fn.Name = TEXT("ReadNumVertices");
		Fn.bHasReturnType = true;
		FShaderParamTypeDefinition ReturnParam = {};
		ReturnParam.ValueType = FShaderValueType::Get(EShaderFundamentalType::Uint);
		Fn.ParamTypes.Add(ReturnParam);
		OutFunctions.Add(Fn);
	}
	
	{
		FShaderFunctionDefinition Fn;
		Fn.Name = TEXT("ReadAdjacentIDs");
		Fn.bHasReturnType = true;
		FShaderParamTypeDefinition ReturnParam = {};
		ReturnParam.ValueType = FShaderValueType::Get(EShaderFundamentalType::Uint);
		Fn.ParamTypes.Add(ReturnParam);
		FShaderParamTypeDefinition Param0 = {};
		Param0.ValueType = FShaderValueType::Get(EShaderFundamentalType::Uint);
		Fn.ParamTypes.Add(Param0);
		OutFunctions.Add(Fn);
	}
}

BEGIN_SHADER_PARAMETER_STRUCT(FTensionDataInterfaceParameters, )
SHADER_PARAMETER(uint32, NumVertices)
SHADER_PARAMETER(uint32, InputStreamStart)
SHADER_PARAMETER_SRV(Buffer<uint>, VertexAdjacencyMapBuffer)
END_SHADER_PARAMETER_STRUCT()

// Query shader parameters
void UTensionDataInterface::GetShaderParameters(TCHAR const* UID, FShaderParametersMetadataBuilder& OutBuilder) const
{
	OutBuilder.AddNestedStruct<FTensionDataInterfaceParameters>(UID);
}

// Path to data interface HLSL file
void UTensionDataInterface::GetHLSL(FString& OutHLSL) const
{
	OutHLSL += TEXT("#include \"/Plugin/TensionGraph/Private/TensionDataInterface.ush\"\n");
}

// Query source type
void UTensionDataInterface::GetSourceTypes(TArray<UClass*>& OutSourceTypes) const
{
	OutSourceTypes.Add(USkeletalMeshComponent::StaticClass());
}

// Generate data provider for optimus interface
UComputeDataProvider* UTensionDataInterface::CreateDataProvider(TArrayView<TObjectPtr<UObject>> InSourceObjects, uint64 InInputMask, uint64 InOutputMask) const
{
	UTensionDataProvider* Provider = NewObject<UTensionDataProvider>();
	
	if (InSourceObjects.Num() == 1)
	{
		Provider->SkeletalMeshComponent = Cast<USkeletalMeshComponent>(InSourceObjects[0]);
	}
	
	return Provider;
}

// Query if resources needed are being provided to the data interface
bool UTensionDataProvider::IsValid() const
{
	bool output = true;
	if (SkeletalMeshComponent == nullptr || SkeletalMeshComponent->MeshObject == nullptr)
	{
		output = false;
	}
	else
	{
		UTensionComponent* DeformerComponent = SkeletalMeshComponent->GetOwner()->FindComponentByClass<UTensionComponent>();
		if (DeformerComponent == nullptr)
		{
			output = false;
		}

	}
	
	return output;
}

// Construct and return a data provider proxy from the tension component
FComputeDataProviderRenderProxy* UTensionDataProvider::GetRenderProxy()
{
	UTensionComponent* DeformerComponent = SkeletalMeshComponent->GetOwner()->FindComponentByClass<UTensionComponent>();

	return new FTensionDataProviderProxy(SkeletalMeshComponent, DeformerComponent);
}

// Define the skeletal mesh object and the buffer that is passing the vertex adjancey data
FTensionDataProviderProxy::FTensionDataProviderProxy(USkeletalMeshComponent* SkeletalMeshComponent, UTensionComponent* DeformerComponent)
{
	SkeletalMeshObject = SkeletalMeshComponent->MeshObject;
	VertexAdjacencyMapBufferSRV = DeformerComponent->VertexAdjacencyMapBuffer.ShaderResourceViewRHI;
}

// Declare the vertex buffer bindings
void FTensionDataProviderProxy::GetBindings(int32 InvocationIndex, TCHAR const* UID, FBindings& OutBindings) const
{
	const int32 SectionIdx = InvocationIndex;
	FSkeletalMeshRenderData const& SkeletalMeshRenderData = SkeletalMeshObject->GetSkeletalMeshRenderData();
	FSkeletalMeshLODRenderData const* LodRenderData = SkeletalMeshRenderData.GetPendingFirstLOD(0);
	FSkelMeshRenderSection const& RenderSection = LodRenderData->RenderSections[SectionIdx];

	FTensionDataInterfaceParameters Parameters;
	FMemory::Memset(&Parameters, 0, sizeof(Parameters));
	Parameters.NumVertices = 0;
	Parameters.InputStreamStart = RenderSection.BaseVertexIndex;
	Parameters.VertexAdjacencyMapBuffer = VertexAdjacencyMapBufferSRV;

	TArray<uint8> ParamData;
	ParamData.SetNum(sizeof(Parameters));
	FMemory::Memcpy(ParamData.GetData(), &Parameters, sizeof(Parameters));
	OutBindings.Structs.Add(TTuple<FString, TArray<uint8>>(UID, MoveTemp(ParamData)));
}
