// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Voxelcaster : ModuleRules
{
	public Voxelcaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 하위 폴더(GAS/, Character/ 등) 헤더를 "GAS/Foo.h" 형태로 포함하기 위해 모듈 루트를 추가한다.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"GameplayAbilities", "GameplayTags", "GameplayTasks",
			"CommonUI", "CommonInput", "UMG", "Slate", "SlateCore", "AIModule", "StateTreeModule", "GameplayStateTreeModule", "ModelViewViewModel", "FieldNotification", "DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
