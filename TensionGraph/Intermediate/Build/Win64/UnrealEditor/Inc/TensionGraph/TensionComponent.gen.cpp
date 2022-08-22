// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "TensionGraph/Public/TensionComponent.h"
#include "Serialization/ArchiveUObjectFromStructuredArchive.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeTensionComponent() {}
// Cross Module References
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionComponent_NoRegister();
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionComponent();
	ENGINE_API UClass* Z_Construct_UClass_UActorComponent();
	UPackage* Z_Construct_UPackage__Script_TensionGraph();
// End Cross Module References
	void UTensionComponent::StaticRegisterNativesUTensionComponent()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UTensionComponent);
	UClass* Z_Construct_UClass_UTensionComponent_NoRegister()
	{
		return UTensionComponent::StaticClass();
	}
	struct Z_Construct_UClass_UTensionComponent_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UTensionComponent_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UActorComponent,
		(UObject* (*)())Z_Construct_UPackage__Script_TensionGraph,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UTensionComponent_Statics::Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "BlueprintType", "true" },
		{ "ClassGroupNames", "Component" },
		{ "Comment", "// Tension component base class to retrieve vertex information\n" },
		{ "IncludePath", "TensionComponent.h" },
		{ "IsBlueprintBase", "true" },
		{ "ModuleRelativePath", "Public/TensionComponent.h" },
		{ "ToolTip", "Tension component base class to retrieve vertex information" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UTensionComponent_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UTensionComponent>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UTensionComponent_Statics::ClassParams = {
		&UTensionComponent::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x00A000A4u,
		METADATA_PARAMS(Z_Construct_UClass_UTensionComponent_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UTensionComponent_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UTensionComponent()
	{
		if (!Z_Registration_Info_UClass_UTensionComponent.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UTensionComponent.OuterSingleton, Z_Construct_UClass_UTensionComponent_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UTensionComponent.OuterSingleton;
	}
	template<> TENSIONGRAPH_API UClass* StaticClass<UTensionComponent>()
	{
		return UTensionComponent::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UTensionComponent);
	IMPLEMENT_FSTRUCTUREDARCHIVE_SERIALIZER(UTensionComponent)
	struct Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionComponent_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionComponent_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UTensionComponent, UTensionComponent::StaticClass, TEXT("UTensionComponent"), &Z_Registration_Info_UClass_UTensionComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UTensionComponent), 3136814644U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionComponent_h_276958359(TEXT("/Script/TensionGraph"),
		Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionComponent_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
