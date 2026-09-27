// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "VXCharacterBase.generated.h"

class UVXAbilitySystemComponent;
class UVXAttributeSet;
class UGameplayAbility;

// 팀 구분
UENUM(BlueprintType)
enum class EVXTeam : uint8
{
	Player,
	Enemy
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FVXHealthChangedSignature, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FVXDamagedSignature, float /*Amount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FVXDeathSignature, class AVXCharacterBase* /*Character*/);

/**
 * 플레이어·적의 공통 베이스. ASC를 캐릭터가 직접 소유한다 (싱글플레이).
 * 체력·이동 속도는 어트리뷰트로 관리하고, 사망 처리를 제공한다.
 */
UCLASS(Abstract)
class VOXELCASTER_API AVXCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AVXCharacterBase();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UVXAbilitySystemComponent* GetVoxelAbilitySystemComponent() const { return AbilitySystemComponent; }
	const UVXAttributeSet* GetAttributeSet() const { return AttributeSet; }

	float GetHealth() const;
	float GetMaxHealth() const;
	bool IsDead() const { return bIsDead; }

	EVXTeam GetTeam() const { return Team; }
	bool IsHostileTo(const AVXCharacterBase* Other) const { return Other && Other->Team != Team; }

	/** 조준 방향 (월드 XY 평면의 단위 벡터). 컨트롤러가 매 프레임 갱신한다. */
	void SetAimDirection(const FVector& NewAimDirection);
	FVector GetAimDirection() const { return AimDirection; }

	/** 현재 이동 입력의 월드 방향. 입력이 없으면 Zero. */
	void SetMoveInputDirection(const FVector& NewDirection) { MoveInputDirection = NewDirection; }
	FVector GetMoveInputDirection() const { return MoveInputDirection; }

	/** 대시 방향: 이동 입력 방향, 없으면 조준 방향 */
	FVector GetDashDirection() const;

	/** 공격 순간의 "띠용" 출렁임 (스쿼시 앤 스트레치). Strength는 스킬별 배율. 기본은 아무것도 하지 않는다 */
	virtual void PlayAttackBounce(float Strength) {}

	/** 체력이 바뀔 때 (HUD 바인딩용) */
	FVXHealthChangedSignature OnHealthChanged;
	/** 사망 시 한 번만 호출 */
	FVXDeathSignature OnDeath;
	/** 체력이 줄었을 때 (피격 연출용). 회복에는 호출되지 않는다 */
	FVXDamagedSignature OnDamaged;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	/** 시작 어빌리티 부여. 자식 클래스에서 재정의한다. */
	virtual void GrantStartupAbilities() {}

	virtual void HandleDeath();

	/** 피해를 받았을 때 (체력 감소량). 사망하는 피격에도 호출된다. 자식 클래스가 피격 연출을 넣는다 */
	virtual void HandleDamaged(float Amount) {}

	UPROPERTY(EditDefaultsOnly, Category = "Voxel")
	EVXTeam Team = EVXTeam::Player;

	/** 시작 능력치 (수치 원본은 design 문서, 추후 데이터 테이블로 이동) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Stats")
	float DefaultMaxHealth = 100.f;

	/** cm/s. 600 = 6 m/s */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Stats")
	float DefaultMoveSpeed = 600.f;

private:
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(VisibleAnywhere, Category = "Voxel|GAS")
	TObjectPtr<UVXAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UVXAttributeSet> AttributeSet;

	FVector AimDirection = FVector::ForwardVector;
	FVector MoveInputDirection = FVector::ZeroVector;
	bool bIsDead = false;
};
