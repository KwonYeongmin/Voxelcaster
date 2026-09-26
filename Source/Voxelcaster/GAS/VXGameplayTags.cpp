// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VXGameplayTags.h"

namespace VXTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill1, "Input.Skill1", "스킬 1 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill2, "Input.Skill2", "스킬 2 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill3, "Input.Skill3", "스킬 3 입력");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Dash, "Input.Dash", "대시 입력");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "사망 상태");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dashing, "State.Dashing", "대시 중 (스킬 사용 불가)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Invincible, "State.Invincible", "무적 (피해 무시)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_God, "State.God", "치트 무적 (God 명령)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Dash, "Cooldown.Dash", "대시 쿨다운");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_MagicBolt, "Cooldown.MagicBolt", "매직 볼트 쿨다운");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Nova, "Cooldown.Nova", "노바 쿨다운");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_BladeSweep, "Cooldown.BladeSweep", "블레이드 스윕 쿨다운");
}
