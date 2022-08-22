// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "TensionGraph/Public/TensionDataInterface.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeTensionDataInterface() {}
// Cross Module References
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionDataInterface_NoRegister();
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionDataInterface();
	OPTIMUSCORE_API UClass* Z_Construct_UClass_UOptimusComputeDataInterface();
	UPackage* Z_Construct_UPackage__Script_TensionGraph();
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionDataProvider_NoRegister();
	TENSIONGRAPH_API UClass* Z_Construct_UClass_UTensionDataProvider();
	COMPUTEFRAMEWORK_API UClass* Z_Construct_UClass_UComputeDataProvider();
	ENGINE_API UClass* Z_Construct_UClass_USkeletalMeshComponent_NoRegister();
// End Cross Module References
	void UTensionDataInterface::StaticRegisterNativesUTensionDataInterface()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UTensionDataInterface);
	UClass* Z_Construct_UClass_UTensionDataInterface_NoRegister()
	{
		return UTensionDataInterface::StaticClass();
	}
	struct Z_Construct_UClass_UTensionDataInterface_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UTensionDataInterface_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UOptimusComputeDataInterface,
		(UObject* (*)())Z_Construct_UPackage__Script_TensionGraph,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UTensionDataInterface_Statics::Class_MetaDataParams[] = {
		{ "Category", "ComputeFramework" },
		{ "Comment", "// Tension data interface for optimus framework\n" },
		{ "IncludePath", "TensionDataInterface.h" },
		{ "ModuleRelativePath", "Public/TensionDataInterface.h" },
		{ "ToolTip", "Tension data interface for optimus framework" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UTensionDataInterface_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UTensionDataInterface>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UTensionDataInterface_Statics::ClassParams = {
		&UTensionDataInterface::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x000100A0u,
		METADATA_PARAMS(Z_Construct_UClass_UTensionDataInterface_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UTensionDataInterface_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UTensionDataInterface()
	{
		if (!Z_Registration_Info_UClass_UTensionDataInterface.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UTensionDataInterface.OuterSingleton, Z_Construct_UClass_UTensionDataInterface_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UTensionDataInterface.OuterSingleton;
	}
	template<> TENSIONGRAPH_API UClass* StaticClass<UTensionDataInterface>()
	{
		return UTensionDataInterface::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UTensionDataInterface);
	void UTensionDataProvider::StaticRegisterNativesUTensionDataProvider()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UTensionDataProvider);
	UClass* Z_Construct_UClass_UTensionDataProvider_NoRegister()
	{
		return UTensionDataProvider::StaticClass();
	}
	struct Z_Construct_UClass_UTensionDataProvider_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SkeletalMeshComponent_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPtrPropertyParams NewProp_SkeletalMeshComponent;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UTensionDataProvider_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UComputeDataProvider,
		(UObject* (*)())Z_Construct_UPackage__Script_TensionGraph,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UTensionDataProvider_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Category", "ComputeFramework" },
		{ "Comment", "// Tension data provider for optimus framework\n" },
		{ "IncludePath", "TensionDataInterface.h" },
		{ "ModuleRelativePath", "Public/TensionDataInterface.h" },
		{ "ToolTip", "Tension data provider for optimus framework" },
	};
#endif
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UTensionDataProvider_Statics::NewProp_SkeletalMeshComponent_MetaData[] = {
		{ "Category", "Binding" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/TensionDataInterface.h" },
	};
#endif
	const UECodeGen_Private::FObjectPtrPropertyParams Z_Construct_UClass_UTensionDataProvider_Statics::NewProp_SkeletalMeshComponent = { "SkeletalMeshComponent", nullptr, (EPropertyFlags)0x001400000008000d, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UTensionDataProvider, SkeletalMeshComponent), Z_Construct_UClass_USkeletalMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_UTensionDataProvider_Statics::NewProp_SkeletalMeshComponent_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_UTensionDataProvider_Statics::NewProp_SkeletalMeshComponent_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UTensionDataProvider_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UTensionDataProvider_Statics::NewProp_SkeletalMeshComponent,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UTensionDataProvider_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UTensionDataProvider>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_UTensionDataProvider_Statics::ClassParams = {
		&UTensionDataProvider::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UTensionDataProvider_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_UTensionDataProvider_Statics::PropPointers),
		0,
		0x008010A0u,
		METADATA_PARAMS(Z_Construct_UClass_UTensionDataProvider_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_UTensionDataProvider_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UTensionDataProvider()
	{
		if (!Z_Registration_Info_UClass_UTensionDataProvider.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UTensionDataProvider.OuterSingleton, Z_Construct_UClass_UTensionDataProvider_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_UTensionDataProvider.OuterSingleton;
	}
	template<> TENSIONGRAPH_API UClass* StaticClass<UTensionDataProvider>()
	{
		return UTensionDataProvider::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(UTensionDataProvider);
	struct Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionDataInterface_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionDataInterface_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_UTensionDataInterface, UTensionDataInterface::StaticClass, TEXT("UTensionDataInterface"), &Z_Registration_Info_UClass_UTensionDataInterface, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UTensionDataInterface), 2162027289U) },
		{ Z_Construct_UClass_UTensionDataProvider, UTensionDataProvider::StaticClass, TEXT("UTensionDataProvider"), &Z_Registration_Info_UClass_UTensionDataProvider, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UTensionDataProvider), 3957989039U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionDataInterface_h_1592125376(TEXT("/Script/TensionGraph"),
		Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionDataInterface_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_TensionMapping_Plugins_TensionGraph_Source_TensionGraph_Public_TensionDataInterface_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
