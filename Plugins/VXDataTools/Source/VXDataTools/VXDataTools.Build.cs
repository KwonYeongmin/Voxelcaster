// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VXDataTools : ModuleRules
{
	public VXDataTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"Slate", "SlateCore", "ToolMenus", "UnrealEd", "AssetRegistry", "MaterialEditor", "RHI", "RenderCore",
			"UMG", "UMGEditor", "CommonUI", "ModelViewViewModel", "ModelViewViewModelBlueprint", "ModelViewViewModelEditor", "FieldNotification", "Kismet"
		});
	}
}
