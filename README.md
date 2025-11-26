# Potenup_NextIdol

<div align="center">

| **항목** | **내용** |
|:---:|:---:|
| **프로젝트명** | Potenup_NextIdol (AI 기반 버추얼 아이돌 인터랙티브 엔터테인먼트 플랫폼) |
| **개발 기간** | 2025.09 |
| **팀 구성** | 3인 (기획 1, 프로그래머 2) |
| **엔진** | Unreal Engine 5.6 |
| **언어** | C++, Blueprint |
| **버전 관리** | Git & GitHub |
| **목적** | AI 에이전트 기반 인터랙티브 아이돌 퍼포먼스 시스템 구현 |

</div>

---

## ✓ 프로젝트 개요

**Potenup_NextIdol**은 Unreal Engine 5.6 기반의 혁신적인 AI 기반 버추얼 아이돌 엔터테인먼트 플랫폼입니다.
AI 에이전트와의 실시간 대화, 퍼포먼스 인터랙션, 그리고 몰입형 버추얼 공연 경험을 제공합니다.

### 핵심 특징
- 🤖 **AI 에이전트 대화 시스템**: DialogueNpcComponent를 통한 자연스러운 AI 에이전트와의 상호작용
- 🎤 **인터랙티브 퍼포먼스**: 실시간 점수 시스템 및 관객 참여형 공연
- 🎭 **DMX 조명 제어**: 전문적인 무대 조명 시스템 통합
- 🎬 **시네마틱 시퀀스**: 고품질 영상 제작 파이프라인
- 🔊 **TTS(Text-to-Speech)**: AI 에이전트의 음성 출력 기능
- 🌐 **네트워크 시스템**: HTTP 기반 멀티플레이어 지원

---

## ✓ 기술 스택 & 도구

### 게임 개발
![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6-313131?style=for-the-badge&logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Blueprint](https://img.shields.io/badge/Blueprint-0E1128?style=for-the-badge&logo=unrealengine&logoColor=white)

### AI & 네트워킹
![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)
![HTTP](https://img.shields.io/badge/HTTP-005571?style=for-the-badge&logo=http&logoColor=white)
![JSON](https://img.shields.io/badge/JSON-000000?style=for-the-badge&logo=json&logoColor=white)

### 버전 관리
![Git](https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white)
![GitHub](http일차)
- ✅ **시스템 통합**: 모든 컴포넌트 연동
- ✅ **UI/UX 개선**: 사용자 경험 향상
- ✅ **성능 최적화**: 프레임레이트 안정화
- ✅ **최종 빌드**: Windows 플랫폼 배포

---

## ✓ 시작하기

### 필수 요구사항

- **Unreal Engine**: 5.6 이상
- **Visual Studio**: 2022 (C++ 개발 도구)
- **운영체제**: Windows 10/11
- **Python**: 3.9+ (AI 서버 연동 시)

### 설치 방법

1. **리포지토리 클론**
   ```bash
   git clone https://github.com/KImnayeongEE/AI_NewIdol.git
   cd AI_NewIdol
   ```

2. **프로젝트 파일 생성**
   - `Potenup_NextIdol.uproject` 우클릭
   - "Generate Visual Studio project files" 선택

3. **프로젝트 빌드**
   - `Potenup_NextIdol.sln` 열기
   - `Development Editor` 구성 선택
   - 빌드 (F7)

4. **에디터 실행**
   - Visual Studio에서 F5 또는
   - `.uproject` 파일 더블클릭

### 의존성 플러그인 활성화

프로젝트 최초 실행 시 다음 플러그인들이 자동으로 활성화됩니다:
- TextToSpeech
- DMX Engine/Protocol/Fixtures/PixelMapping
- PythonScriptPlugin
- MovieRenderPipeline
- HDRIBackdrop
- ModelingToolsEditorMode
- DatasmithImporter

---

## ✓ 사용 방법

### AI 에이전트와 대화하기

1. **DialogueNpcComponent 설정**
   - AI 서버 URL 설정
   - TTS 음성 파라미터 조정

2. **대화 시작**
   - 플레이어가 NPC에 접근
   - 상호작용 키 입력
   - AI 응답 대기 및 음성 출력

### 퍼포먼스 관람

1. **메인 맵 로드**: `Content/Maps/MainStage`
2. **플레이 시작**: 시네마틱 자동 재생
3. **인터랙션**: 투표 버튼 클릭으로 참여

---

<div align="center">

**Potenup_NextIdol** - AI 기반 버추얼 아이돌 플랫폼

Made with ❤️ by the Potenup Team

</div>
