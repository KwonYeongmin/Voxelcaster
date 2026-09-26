// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GAS/VoxelHitContext.h"
#include "VXModifierComponent.generated.h"

class AVoxelProjectile;
class AVXCharacterBase;

/** 모디파이어 종류 (DES-MOD-001) */
UENUM(BlueprintType)
enum class EVXModifierType : uint8
{
	Pierce,
	Split,
	Explode,
	Chain,
	Haste
};

/** 스킬 하나에 장착된 모디파이어 슬롯. 슬롯 1칸 = 스택 1이라 같은 종류가 여러 번 들어갈 수 있다. */
USTRUCT(BlueprintType)
struct FVXModifierSlots
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<EVXModifierType> Slots;

	int32 GetStack(EVXModifierType Type) const;
};

DECLARE_MULTICAST_DELEGATE(FVXModifiersChangedSignature);

/**
 * 플레이어의 스킬별 모디파이어 장착 상태와 효과 실행을 담당한다. (SPC-MOD-001)
 * - 스킬은 쿨다운 태그(Cooldown.MagicBolt 등)로 식별한다. 명중 정보(FVoxelHitContext::SkillTag)와 같다.
 * - 스킬 명중(OnSkillHit)을 구독해 분열·폭발·연쇄를 실행한다. 파생 명중(bIsDerived)에는 반응하지 않는다.
 * - 관통은 투사체가, 가속은 스킬 어빌리티가 이 컴포넌트의 스택을 읽어 적용한다.
 * 확정 규칙: 슬롯 3칸, 슬롯 1칸 = 스택 1, 최대 스택 3, 분열 피해 40%.
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXELCASTER_API UVXModifierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVXModifierComponent();

	/** 장착할 수 있는지: 빈 슬롯이 있고, 최대 스택 미만이고, 스킬 형태에 효과가 있는지 */
	bool CanAddModifier(const FGameplayTag& SkillTag, EVXModifierType Type) const;

	/** 모디파이어를 1스택 장착한다. 실패하면 false */
	bool AddModifier(const FGameplayTag& SkillTag, EVXModifierType Type);

	int32 GetStack(const FGameplayTag& SkillTag, EVXModifierType Type) const;
	int32 GetUsedSlots(const FGameplayTag& SkillTag) const;
	const FVXModifierSlots* FindSlots(const FGameplayTag& SkillTag) const;

	/** 모든 장착을 지운다 (재시작) */
	void ResetModifiers();

	/** 가속을 반영한 쿨다운 */
	float GetModifiedCooldown(const FGameplayTag& SkillTag, float BaseCooldown) const;

	/** 효과가 있는 조합인지 (관통은 투사체 스킬에만) */
	static bool IsModifierValidForSkill(const FGameplayTag& SkillTag, EVXModifierType Type);

	/** 내부 이름 (치트·로그용, 영어 고정) */
	static FString GetModifierName(EVXModifierType Type);
	static FString GetSkillName(const FGameplayTag& SkillTag);

	/** 화면 표시 이름 (DT_UIText, 현재 언어) */
	static FString GetModifierDisplayName(EVXModifierType Type);
	static FString GetSkillDisplayName(const FGameplayTag& SkillTag);

	/** 장착이 바뀔 때 (HUD, 결과 화면) */
	FVXModifiersChangedSignature OnModifiersChanged;

	/** 스킬별 슬롯 수 */
	static constexpr int32 MaxSlots = 3;
	/** 모디파이어별 최대 스택 */
	static constexpr int32 MaxStack = 3;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---- 수치 (design 문서 기준, 플레이하며 조정) ----

	/** 분열 투사체 피해 비율 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Split")
	float SplitDamageRatio = 0.4f;

	/** 분열 투사체 속도 (cm/s). 15 m/s */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Split")
	float SplitSpeed = 1500.f;

	/** 분열 투사체 사거리 (cm). 6 m */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Split")
	float SplitRange = 600.f;

	/** 동시에 존재할 수 있는 분열 투사체 수 (성능 제한) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Split")
	int32 MaxSplitProjectiles = 60;

	/** 폭발 반경 (cm): 1스택 150, 스택당 +50 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Explode")
	float ExplodeBaseRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Explode")
	float ExplodeRadiusPerStack = 50.f;

	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Explode")
	float ExplodeDamageRatio = 0.5f;

	/** 연쇄 전이 최대 거리 (cm). 5 m */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Chain")
	float ChainRange = 500.f;

	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Chain")
	float ChainDamageRatio = 0.7f;

	/** 가속: 스택당 쿨다운 감소 비율 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Haste")
	float HasteReductionPerStack = 0.15f;

	/** 쿨다운 하한 (초) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Modifier|Haste")
	float MinCooldown = 0.1f;

	/** 화면 좌측 상단에 현재 빌드를 표시한다 (HUD가 생기면 끈다) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Debug")
	bool bShowDebugInfo = false;

private:
	void HandleSkillHit(const FVoxelHitContext& Context);

	void ApplySplit(const FVoxelHitContext& Context, int32 Stack);
	void ApplyExplode(const FVoxelHitContext& Context, int32 Stack);
	void ApplyChain(const FVoxelHitContext& Context, int32 Stack);

	/** 파생 피해를 준다 (bIsDerived = true, 모디파이어 재발동 없음) */
	void ApplyDerivedHit(const FVoxelHitContext& Source, AVXCharacterBase* Target, const FVector& Location, float Damage) const;

	void GatherHostiles(const FVector& Center, float Radius, TArray<AVXCharacterBase*>& OutTargets) const;
	AVXCharacterBase* GetOwnerCharacter() const;

	UPROPERTY(VisibleAnywhere, Category = "Voxel|Modifier")
	TMap<FGameplayTag, FVXModifierSlots> Equipped;

	TArray<TWeakObjectPtr<AVoxelProjectile>> LiveSplitProjectiles;
	bool bBoundToHits = false;
};
