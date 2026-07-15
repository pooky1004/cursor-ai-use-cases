# Y2K38 발표 자료 인덱스

프로젝트 루트의 PPT용 마크다운 모음.  
각 파일은 **슬라이드 단위(`## Slide N`)** 로 작성되어 PPT에 옮기기 쉽게 되어 있다.

| 파일 | 주제 | 권장 슬라이드 |
|------|------|---------------|
| [`PPT_01_liby2k38safe.md`](PPT_01_liby2k38safe.md) | 라이브러리 설명·기능·연동 | 3~4장 |
| [`PPT_02_daemon_y2k38_check.md`](PPT_02_daemon_y2k38_check.md) | Y2K38 Check 데몬 | 3~4장 |
| [`PPT_03_y2k38_offsetctl.md`](PPT_03_y2k38_offsetctl.md) | y2k38_offsetctl 도구 | 3~4장 |
| [`PPT_04_toolchain_update.md`](PPT_04_toolchain_update.md) | 개발환경(툴체인) 갱신 | 2~3장 |
| [`PPT_05_board_deploy.md`](PPT_05_board_deploy.md) | 보드 디렉터리·load·설정 | 2~3장 |

### 권장 발표 순서

1. 문제 배경 (Y2K38 / 32-bit `time_t`) — 기존 `Y2K38_issue.md` 참고  
2. **라이브러리** → **Check 데몬** → **offsetctl** (기능 축)  
3. **툴체인 갱신** → **보드 배포** (적용 축)

### 세 구성 요소 연동 한 장 (발표용)

```
┌──────────────────────────────────────────────────────────┐
│                    liby2k38safe (공통 ABI)                │
│         y2k38_time_t · OFFSET · session · SIGHUP         │
└────────────┬───────────────────┬─────────────────────────┘
             │                   │
   ┌─────────▼─────────┐ ┌───────▼────────┐ ┌──────────────▼─────────┐
   │ daemon_y2k38_check│ │ daemon A / B   │ │ y2k38_offsetctl        │
   │ wrap poll 1~60s   │ │ 앱 로직·로그   │ │ set-time / calibrate   │
   │ OFFSET+=2^32      │ │ SIGHUP 수신    │ │ notify / sync          │
   │ SIGHUP 송신       │ │                │ │                        │
   └─────────┬─────────┘ └───────▲────────┘ └──────────────┬─────────┘
             │                   │                         │
             └───────── /etc/y2k38_offset · sighup.list ───┘
```
