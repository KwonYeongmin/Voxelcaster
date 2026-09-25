# Voxelcaster

기본 스킬 3개에 웨이브마다 **모디파이어**(관통·분열·폭발·연쇄·가속)를 붙여 빌드가 갈리는 탑다운 로그라이크 액션 게임입니다. "선택이 조합되어 플레이가 달라진다"를 보여주는 것이 목표입니다.

## 게임 소개

- **장르**: 탑다운 로그라이크 핵앤슬래시 (웨이브 서바이벌)
- **한 판**: 5웨이브, 약 8~10분. 웨이브를 클리어할 때마다 카드 3장 중 1장을 골라 스킬에 모디파이어를 붙입니다.
- **스킬**: 매직 볼트(투사체), 노바(원형 범위), 블레이드 스윕(부채꼴 근접). 같은 모디파이어도 스킬 형태에 따라 결과가 달라집니다.
- **적**: 러너(돌진), 슈터(거리 유지 사격), 엘리트(3방향 사격, 체력 50%에서 러너 소환)
- **조작**: 키보드·마우스와 게임패드(Xbox·PlayStation) 모두 지원합니다.
- **아트**: 스타일라이즈드 판타지 던전 (탑다운)

## 기술 스택

| 항목 | 내용 |
| --- | --- |
| 엔진 | Unreal Engine 5.8, C++ |
| 플랫폼 | PC (Windows) |
| 스킬·전투 | Gameplay Ability System (GAS) |
| 입력 | Enhanced Input |
| UI | CommonUI + MVVM (예정) |
| 적 AI | StateTree |

## 개발일지

개발 과정은 Notion에 기록하고 있습니다.

**[VoxelCaster 개발일지](https://amazing-river-dff.notion.site/VoxelCaster-3e6a839162f6807a9c31fe89e836e10c?source=copy_link)**

## 개발 환경

1. Unreal Engine 5.8을 설치합니다.
2. `Voxelcaster.uproject`를 우클릭해 **Generate Visual Studio project files**를 실행합니다.
3. 생성된 솔루션을 열어 `VoxelcasterEditor`를 빌드하거나, `Voxelcaster.uproject`를 더블클릭해 에디터를 실행합니다.
