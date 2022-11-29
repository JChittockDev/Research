// Copyright 2022 Joseph Chittock. All Rights Reserved.

#include "TissueDataInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "ComputeFramework/ComputeKernelPermutationSet.h"
#include "ComputeFramework/ShaderParamTypeDefinition.h"
#include "TissueComponent.h"
#include "OptimusDataDomain.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderGraphResources.h"
#include "ShaderParameterMetadataBuilder.h"
#include "SkeletalRenderPublic.h"

// Define display name in optimus graph
FString UTissueDataInterface::GetDisplayName() const
{
	return TEXT("Tissue Data");
}

// Declare the node graph pin names
TArray<FOptimusCDIPinDefinition> UTissueDataInterface::GetPinDefinitions() const
{
	TArray<FOptimusCDIPinDefinition> Defs;
	Defs.Add({ "TetVertexPositions", "ReadTetVertexPositions", Optimus::DomainName::Vertex, "ReadNumTetVertices" });
	return Defs;
}

// Declare the node outputs in optimus graph
void UTissueDataInterface::GetSupportedInputs(TArray<FShaderFunctionDefinition>& OutFunctions) const
{
	{
		FShaderFunctionDefinition Fn;
		Fn.Name = TEXT("ReadNumTetVertices");
		Fn.bHasReturnType = true;
		FShaderParamTypeDefinition ReturnParam = {};
		ReturnParam.ValueType = FShaderValueType::Get(EShaderFundamentalType::Uint);
		Fn.ParamTypes.Add(ReturnParam);
		OutFunctions.Add(Fn);
	}

	{
		FShaderFunctionDefinition Fn;
		Fn.Name = TEXT("ReadTetVertexPositions");
		Fn.bHasReturnType = true;
		FShaderParamTypeDefinition ReturnParam = {};
		ReturnParam.ValueType = FShaderValueType::Get(EShaderFundamentalType::Float, 3);
		Fn.ParamTypes.Add(ReturnParam);
		FShaderParamTypeDefinition Param0 = {};
		Param0.ValueType = FShaderValueType::Get(EShaderFundamentalType::Uint);
		Fn.ParamTypes.Add(Param0);
		OutFunctions.Add(Fn);
	}
}

BEGIN_SHADER_PARAMETER_STRUCT(FTissueDataInterfaceParameters, )
SHADER_PARAMETER(uint32, InputStreamStart)
SHADER_PARAMETER(uint32, NumInputVertices)
SHADER_PARAMETER(uint32, NumTetVertices)
SHADER_PARAMETER_SRV(Buffer<float3>, InputVertexPositionBuffer)
SHADER_PARAMETER_SRV(Buffer<float3>, TetVertexPositionBuffer)
END_SHADER_PARAMETER_STRUCT()

// Query shader parameters
void UTissueDataInterface::GetShaderParameters(TCHAR const* UID, FShaderParametersMetadataBuilder& OutBuilder) const
{
	OutBuilder.AddNestedStruct<FTissueDataInterfaceParameters>(UID);
}

// Path to data interface HLSL file
void UTissueDataInterface::GetHLSL(FString& OutHLSL) const
{
	OutHLSL += TEXT("#include \"/Plugin/TissueGraph/Private/TissueDataInterface.ush\"\n");
}

// Query source type
void UTissueDataInterface::GetSourceTypes(TArray<UClass*>& OutSourceTypes) const
{
	OutSourceTypes.Add(USkeletalMeshComponent::StaticClass());
}

// Generate data provider for optimus interface
UComputeDataProvider* UTissueDataInterface::CreateDataProvider(TArrayView<TObjectPtr<UObject>> InSourceObjects, uint64 InInputMask, uint64 InOutputMask) const
{
	UTissueDataProvider* Provider = NewObject<UTissueDataProvider>();
	
	if (InSourceObjects.Num() == 1)
	{
		Provider->SkeletalMeshComponent = Cast<USkeletalMeshComponent>(InSourceObjects[0]);
	}
	
	return Provider;
}

// Query if resources needed are being provided to the data interface
bool UTissueDataProvider::IsValid() const
{
	bool output = true;
	if (SkeletalMeshComponent == nullptr || SkeletalMeshComponent->MeshObject == nullptr)
	{
		output = false;
	}
	else
	{
		UTissueComponent* DeformerComponent = SkeletalMeshComponent->GetOwner()->FindComponentByClass<UTissueComponent>();
		if (DeformerComponent == nullptr)
		{
			output = false;
		}

	}
	
	return output;
}

// Construct and return a data provider proxy from the Tissue component
FComputeDataProviderRenderProxy* UTissueDataProvider::GetRenderProxy()
{
	UTissueComponent* DeformerComponent = SkeletalMeshComponent->GetOwner()->FindComponentByClass<UTissueComponent>();

	return new FTissueDataProviderProxy(SkeletalMeshComponent, DeformerComponent);
}

// Define the skeletal mesh object and the buffer that is passing the vertex adjancey data
FTissueDataProviderProxy::FTissueDataProviderProxy(USkeletalMeshComponent* SkeletalMeshComponent, UTissueComponent* DeformerComponent)
{
	SkeletalMeshObject = SkeletalMeshComponent->MeshObject;
	TetVertexCountRef = DeformerComponent->TetVertexCount;
	TetVertexPositionsBufferSRV = DeformerComponent->TetVertexPositionsBuffer.ShaderResourceViewRHI;
}

// Declare the vertex buffer bindings
void FTissueDataProviderProxy::GetBindings(int32 InvocationIndex, TCHAR const* UID, FBindings& OutBindings) const
{
	const int32 SectionIdx = InvocationIndex;
	FSkeletalMeshRenderData const& SkeletalMeshRenderData = SkeletalMeshObject->GetSkeletalMeshRenderData();
	FSkeletalMeshLODRenderData const* LodRenderData = SkeletalMeshRenderData.GetPendingFirstLOD(0);
	FSkelMeshRenderSection const& RenderSection = LodRenderData->RenderSections[SectionIdx];

	FTissueDataInterfaceParameters Parameters;
	FMemory::Memset(&Parameters, 0, sizeof(Parameters));
	Parameters.InputStreamStart = RenderSection.BaseVertexIndex;
	Parameters.NumTetVertices = 0;
	Parameters.TetVertexPositionBuffer = TetVertexPositionsBufferSRV;

	TArray<uint8> ParamData;
	ParamData.SetNum(sizeof(Parameters));
	FMemory::Memcpy(ParamData.GetData(), &Parameters, sizeof(Parameters));
	OutBindings.Structs.Add(TTuple<FString, TArray<uint8>>(UID, MoveTemp(ParamData)));
}
