// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "VXDataManager.generated.h"

class UDataTable;
struct FVXEnemyRow;
struct FVXModifierRow;
struct FVXSkillRow;

/** 게임 데이터 테이블 종류 */
UENUM()
enum class EVXDataTable : uint8
{
	Waves,
	Enemies,
	Skills,
	Modifiers,
	TextKor,
	TextEng,
	/** 입력 아이콘 (선택). 없으면 글자로 표시 */
	InputIcons,
	Count UMETA(Hidden)
};

/**
 * 데이터 매니저. 모든 DataTable을 한곳에서 읽고 검사한다.
 * - 테이블 위치는 UVXDataSettings (프로젝트 세팅 → Game → Voxelcaster Data)
 * - 게임 시작 시(AVXGameMode::InitGame) LoadAll로 모두 읽고, 빠진 테이블·행·잘못된 값을 로그에 남긴다.
 * - 테이블이나 행이 없으면 nullptr을 돌려준다. 쓰는 쪽은 코드 기본값을 쓴다.
 * - 에디터에서 테이블 값을 고치면 같은 객체라 바로 반영된다. 세팅의 경로를 바꿨다면 치트 DataReload.
 */
UCLASS()
class VOXELCASTER_API UVXDataManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	static UVXDataManager* Get();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 모든 테이블을 다시 읽고 검사 결과를 로그에 남긴다. 문제가 없으면 true */
	bool LoadAll();

	/** 테이블. 아직 읽지 않았으면 이때 읽는다. 없거나 행 구조체가 다르면 nullptr */
	const UDataTable* GetTable(EVXDataTable Type);

	const FVXEnemyRow* FindEnemy(FName RowName);
	const FVXSkillRow* FindSkill(FName RowName);
	const FVXModifierRow* FindModifier(FName RowName);
	const UDataTable* GetTextTable(bool bKorean);

	static const TCHAR* GetTableLabel(EVXDataTable Type);

	/** 없어도 되는 테이블 (없을 때 경고하지 않는다) */
	static bool IsOptional(EVXDataTable Type) { return EVXDataTable::InputIcons == Type; }

private:
	const UDataTable* LoadTable(EVXDataTable Type);

	/** 테이블 하나를 검사한다. 문제 수를 돌려준다 */
	int32 ValidateTable(EVXDataTable Type);

	/** EVXDataTable 순서. 없으면 nullptr */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> Tables;

	TArray<bool> bTried;
};
