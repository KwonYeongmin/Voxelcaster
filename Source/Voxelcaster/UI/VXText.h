// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VXText.generated.h"

/**
 * 화면 문구 한 줄. 언어별 테이블(DS_UIText_Kor, DS_UIText_Eng)의 행 구조체다. 행 이름이 키다. (예: Mod.Explode)
 * 문구 안의 {0}, {1}은 숫자 등으로 치환된다. \n 은 줄바꿈이 된다.
 */
USTRUCT(BlueprintType)
struct FVXTextRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;
};

/**
 * 화면 문구 조회. 현재 언어의 테이블에서 문구를 찾는다.
 * - 한국어: /Game/Voxelcaster/Data/DS_UIText_Kor
 * - 영어:   /Game/Voxelcaster/Data/DS_UIText_Eng
 * 테이블이 없거나 키가 없으면 코드에 넣어 둔 기본 문구를 쓴다.
 * 언어는 콘솔 변수 Voxel.Language (ko / en)로 바꾼다. 기본 ko.
 */
namespace VXText
{
	/** 키에 해당하는 문구 */
	VOXELCASTER_API FString Get(const FName& Key);

	/** 키의 문구에서 {0}, {1} ...을 인자로 치환한다 */
	VOXELCASTER_API FString Format(const FName& Key, const FStringFormatOrderedArguments& Args);

	/** 현재 언어가 한국어인지 */
	VOXELCASTER_API bool IsKorean();
}
