// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXText.h"
#include "HAL/IConsoleManager.h"
#include "UObject/SoftObjectPtr.h"
#include "Voxelcaster.h"

namespace
{
	TAutoConsoleVariable<FString> CVarLanguage(
		TEXT("Voxel.Language"), TEXT("ko"),
		TEXT("화면 문구 언어: ko(한국어) / en(영어)"));

	const TCHAR* TablePath = TEXT("/Game/Voxelcaster/Data/DT_UIText.DT_UIText");

	/** 테이블을 못 읽을 때 쓰는 기본 문구. Data/DT_UIText.json과 같은 내용이다. */
	struct FDefaultText
	{
		const TCHAR* Key;
		const TCHAR* Ko;
		const TCHAR* En;
	};

	const FDefaultText DefaultTexts[] =
	{
		{ TEXT("Skill.MagicBolt"),  TEXT("매직 볼트"),     TEXT("Magic Bolt") },
		{ TEXT("Skill.Nova"),       TEXT("노바"),          TEXT("Nova") },
		{ TEXT("Skill.BladeSweep"), TEXT("블레이드 스윕"), TEXT("Blade Sweep") },

		{ TEXT("Mod.Pierce"),  TEXT("관통"), TEXT("Pierce") },
		{ TEXT("Mod.Split"),   TEXT("분열"), TEXT("Split") },
		{ TEXT("Mod.Explode"), TEXT("폭발"), TEXT("Explode") },
		{ TEXT("Mod.Chain"),   TEXT("연쇄"), TEXT("Chain") },
		{ TEXT("Mod.Haste"),   TEXT("가속"), TEXT("Haste") },

		{ TEXT("Desc.Pierce"),  TEXT("적 {0}명을 관통"),                         TEXT("Pierces {0} enemies") },
		{ TEXT("Desc.Split"),   TEXT("명중 시 작은 투사체 {0}발\n(피해 40%)"),   TEXT("Splits into {0} bolts on hit\n(40% damage)") },
		{ TEXT("Desc.Explode"), TEXT("명중 지점에 반경 {0}m 폭발\n(피해 50%)"),  TEXT("Explodes in a {0}m radius on hit\n(50% damage)") },
		{ TEXT("Desc.Chain"),   TEXT("가까운 적 {0}명에게 번개 전이\n(피해 70%)"), TEXT("Lightning chains to {0} enemies\n(70% damage)") },
		{ TEXT("Desc.Haste"),   TEXT("쿨다운 -{0}%"),                             TEXT("Cooldown -{0}%") },

		{ TEXT("UI.Level"),       TEXT("Lv.{0}"),                                         TEXT("Lv.{0}") },
		{ TEXT("UI.New"),         TEXT("신규"),                                           TEXT("NEW") },
		{ TEXT("UI.Upgrade"),     TEXT("강화"),                                           TEXT("UPGRADE") },
		{ TEXT("UI.RewardTitle"), TEXT("웨이브 {0} 클리어  -  강화를 선택하세요"),       TEXT("WAVE {0} CLEAR  -  Choose an upgrade") },
		{ TEXT("UI.RewardHint"),  TEXT("좌우로 선택   ·   Enter / 클릭 / (A) 로 확정"),  TEXT("Left / Right to choose   ·   Enter / Click / (A) to confirm") },
		{ TEXT("UI.BuildOf"),     TEXT("{0} 빌드:"),                                      TEXT("{0} build:") },
		{ TEXT("UI.Build"),       TEXT("빌드"),                                           TEXT("BUILD") },
	};

	const UDataTable* LoadTable()
	{
		// 한 번만 시도한다. 테이블이 없으면 기본 문구를 쓴다.
		static TWeakObjectPtr<const UDataTable> Cached;
		static bool bTried = false;
		if (false == bTried)
		{
			bTried = true;
			const TSoftObjectPtr<UDataTable> Soft{ FSoftObjectPath(TablePath) };
			const UDataTable* Table = Soft.LoadSynchronous();
			if (Table && Table->GetRowStruct() == FVXTextRow::StaticStruct())
			{
				Cached = Table;
			}
			else
			{
				UE_LOG(LogVoxel, Log, TEXT("DT_UIText not found or wrong row struct: using built-in texts"));
			}
		}
		return Cached.Get();
	}
}

namespace VXText
{
	bool IsKorean()
	{
		return false == CVarLanguage.GetValueOnGameThread().Equals(TEXT("en"), ESearchCase::IgnoreCase);
	}

	FString Get(const FName& Key)
	{
		const bool bKorean = IsKorean();

		if (const UDataTable* Table = LoadTable())
		{
			if (const FVXTextRow* Row = Table->FindRow<FVXTextRow>(Key, TEXT("VXText"), false))
			{
				const FString& Text = bKorean ? Row->Ko : Row->En;
				if (false == Text.IsEmpty())
				{
					return Text.Replace(TEXT("\\n"), TEXT("\n"));
				}
			}
		}

		for (const FDefaultText& Default : DefaultTexts)
		{
			if (Key == Default.Key)
			{
				return bKorean ? Default.Ko : Default.En;
			}
		}
		return Key.ToString();
	}

	FString Format(const FName& Key, const FStringFormatOrderedArguments& Args)
	{
		return FString::Format(*Get(Key), Args);
	}
}
