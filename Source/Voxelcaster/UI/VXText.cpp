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

	const TCHAR* KoreanTablePath = TEXT("/Game/Voxelcaster/Data/DS_UIText_Kor.DS_UIText_Kor");
	const TCHAR* EnglishTablePath = TEXT("/Game/Voxelcaster/Data/DS_UIText_Eng.DS_UIText_Eng");

	/** 테이블을 못 읽을 때 쓰는 기본 문구. Data/DS_UIText_Kor.csv, DS_UIText_Eng.csv와 같은 내용이다. */
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

		{ TEXT("UI.Wave"),        TEXT("웨이브 {0} / {1}"),   TEXT("WAVE {0} / {1}") },
		{ TEXT("UI.Remaining"),   TEXT("남은 적 {0}"),        TEXT("Enemies {0}") },
		{ TEXT("UI.Kills"),       TEXT("처치 {0}"),           TEXT("Kills {0}") },
		{ TEXT("UI.Dash"),        TEXT("대시"),               TEXT("Dash") },
		{ TEXT("UI.Paused"),      TEXT("일시정지"),           TEXT("PAUSED") },
		{ TEXT("UI.Resume"),      TEXT("재개"),               TEXT("Resume") },
		{ TEXT("UI.Restart"),     TEXT("재시작"),             TEXT("Restart") },
		{ TEXT("UI.Quit"),        TEXT("종료"),               TEXT("Quit") },
		{ TEXT("UI.Victory"),     TEXT("승리"),               TEXT("VICTORY") },
		{ TEXT("UI.Defeat"),      TEXT("패배"),               TEXT("DEFEAT") },
		{ TEXT("UI.ReachedWave"), TEXT("도달 웨이브 {0} / {1}"), TEXT("Reached wave {0} / {1}") },
		{ TEXT("UI.PlayTime"),    TEXT("플레이 시간 {0}"),    TEXT("Play time {0}") },
		{ TEXT("UI.FinalBuild"),  TEXT("최종 빌드"),          TEXT("FINAL BUILD") },
	};

	/** 언어별 테이블을 한 번만 읽는다. 없으면 nullptr (기본 문구 사용) */
	const UDataTable* LoadTable(bool bKorean)
	{
		static TWeakObjectPtr<const UDataTable> Cached[2];
		static bool bTried[2] = { false, false };

		const int32 Index = bKorean ? 0 : 1;
		if (false == bTried[Index])
		{
			bTried[Index] = true;
			const TSoftObjectPtr<UDataTable> Soft{ FSoftObjectPath(bKorean ? KoreanTablePath : EnglishTablePath) };
			const UDataTable* Table = Soft.LoadSynchronous();
			if (Table && Table->GetRowStruct() == FVXTextRow::StaticStruct())
			{
				Cached[Index] = Table;
			}
			else
			{
				UE_LOG(LogVX, Log, TEXT("%s not found or wrong row struct: using built-in texts"),
					bKorean ? TEXT("DS_UIText_Kor") : TEXT("DS_UIText_Eng"));
			}
		}
		return Cached[Index].Get();
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

		if (const UDataTable* Table = LoadTable(bKorean))
		{
			if (const FVXTextRow* Row = Table->FindRow<FVXTextRow>(Key, TEXT("VXText"), false))
			{
				if (false == Row->Text.IsEmpty())
				{
					// CSV에는 줄바꿈을 \n 두 글자로 적는다.
					return Row->Text.Replace(TEXT("\\n"), TEXT("\n"));
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
