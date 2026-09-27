// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VXEnemyClasses.h"
#include "Enemy/VXElite.h"
#include "Enemy/VXRunner.h"
#include "Enemy/VXShooter.h"
#include "UObject/SoftObjectPtr.h"
#include "Voxelcaster.h"

namespace VXEnemyClasses
{
	TSubclassOf<AVXEnemyBase> Resolve(const FString& EnemyType)
	{
		TSubclassOf<AVXEnemyBase> NativeClass;
		FString Name;
		if (EnemyType.Equals(TEXT("Runner"), ESearchCase::IgnoreCase))
		{
			NativeClass = AVXRunner::StaticClass();
			Name = TEXT("Runner");
		}
		else if (EnemyType.Equals(TEXT("Shooter"), ESearchCase::IgnoreCase))
		{
			NativeClass = AVXShooter::StaticClass();
			Name = TEXT("Shooter");
		}
		else if (EnemyType.Equals(TEXT("Elite"), ESearchCase::IgnoreCase))
		{
			NativeClass = AVXElite::StaticClass();
			Name = TEXT("Elite");
		}
		else
		{
			return nullptr;
		}

		const FString Path = FString::Printf(TEXT("/Game/Voxelcaster/Enemy/BP_VX%s.BP_VX%s_C"), *Name, *Name);
		const TSoftClassPtr<AVXEnemyBase> Soft{ FSoftObjectPath(Path) };
		if (UClass* BlueprintClass = Soft.LoadSynchronous())
		{
			if (BlueprintClass->IsChildOf(NativeClass))
			{
				return BlueprintClass;
			}
			UE_LOG(LogVX, Warning, TEXT("%s is not a child of %s: using C++ class"), *Path, *NativeClass->GetName());
		}
		return NativeClass;
	}
}
