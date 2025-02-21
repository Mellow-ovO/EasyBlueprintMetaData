// Copyright Epic Games, Inc. All Rights Reserved.

#include "EasyBlueprintMetaData.h"

#include "BlueprintEditorModule.h"
#include "Customization/EasyBPMetaDataDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FEasyBlueprintMetaDataModule"

void FEasyBlueprintMetaDataModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// Register Blueprint editor variable customization
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintVariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&EasyBPMetaDataDetailCustomization::MakeInstance));
}

void FEasyBlueprintMetaDataModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unregister Blueprint editor variable customization
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintEditorModule.UnregisterVariableCustomization(FProperty::StaticClass(), BlueprintVariableCustomizationHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEasyBlueprintMetaDataModule, EasyBlueprintMetaData)