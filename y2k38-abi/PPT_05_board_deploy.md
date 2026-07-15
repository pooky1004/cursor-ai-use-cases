# 발표 자료 — 보드 배포 · 디렉터리 · 설정

**대상:** 타깃 보드 런타임 레이아웃  
**분량:** PPT 2~3장

---

## Slide 1 — 어디에 무엇을 올리는가

### 원칙

| 구분 | 호스트 (sysroot) | 보드 |
|------|------------------|------|
| 헤더 | `$SYSROOT/usr/include/y2k38/` | 보통 **불필요** |
| `.a` / 링크용 `.so` | `$SYSROOT/usr/lib/` | 동적 링크 앱일 때만 `.so` |
| 실행 파일 | (빌드 산출물) | **`/usr/bin/`** |
| OFFSET | — | **`/etc/y2k38_offset`** |
| SIGHUP 리스트 | — | **`/var/run/y2k38_sighup.list`** (런타임 생성) |

### 권장 보드 트리

```
/usr/bin/
├── daemon_y2k38_check
├── daemon_a
├── daemon_b
└── y2k38_offsetctl

/usr/lib/                          # 동적 링크 시에만
├── liby2k38safe.so.1.0.0
└── liby2k38safe.so.1 → …

/etc/
└── y2k38_offset                   # OFFSET <int64>

/var/run/
└── y2k38_sighup.list              # session_init 시 자동 갱신

/usr/share/y2k38/                  # 선택
└── board-init-y2k38.sh
```

> 본 프로젝트 기본 `LINK_STATIC=1`이면 데몬/툴은 **`.so` 없이** `/usr/bin`만으로 동작 가능.

---

## Slide 2 — 어떻게 load / 배포하는가

### A. staging → 보드 (권장)

```bash
# 호스트
./scripts/cross-build-eldk.sh stage
./scripts/deploy-board.sh root@BOARD:/

# staging 내용 요약
#   staging/usr/bin/…
#   staging/usr/lib/liby2k38safe*
#   staging/etc/y2k38_offset
#   staging/usr/share/y2k38/board-init-y2k38.sh
```

### B. 수동 scp 예

```bash
scp staging/usr/bin/daemon_* staging/usr/bin/y2k38_offsetctl root@BOARD:/usr/bin/
scp staging/etc/y2k38_offset root@BOARD:/etc/
# 동적 링크 시
scp staging/usr/lib/liby2k38safe.so.1.0.0 root@BOARD:/usr/lib/
ssh root@BOARD 'cd /usr/lib && ln -sf liby2k38safe.so.1.0.0 liby2k38safe.so.1'
```

### C. 보드에서 해야 할 설정

**1) OFFSET 파일**

```bash
# 최초 (pre-wrap, 시계 정상)
cat /etc/y2k38_offset
# OFFSET 0

# 시각 맞춤 (현장)
y2k38_offsetctl set-time YYYY-MM-DD:hh:mm:ss
# 또는
y2k38_offsetctl calibrate <true_utc_epoch> --notify
```

**2) 데몬 기동 순서 (권장)**

```bash
daemon_y2k38_check --offset-file /etc/y2k38_offset &
daemon_a /var/log/y2k38_events.log --offset-file /etc/y2k38_offset &
daemon_b /var/log/y2k38_events.log /var/log/y2k38_deltas.out \
  --offset-file /etc/y2k38_offset &
```

**3) 부팅 훅 (선택)**

`board-init-y2k38.sh` 또는 `/etc/rc.local`:

```bash
export Y2K38_START_DAEMONS=1
/usr/share/y2k38/board-init-y2k38.sh
```

**4) 환경 변수 (필요 시)**

| 변수 | 기본 |
|------|------|
| `Y2K38_KERNEL_OFFSET_FILE` | `/etc/y2k38_offset` |
| `Y2K38_SIGHUP_LIST_FILE` | `/var/run/y2k38_sighup.list` |

`/var/run`은 재부팅 시 비울 수 있음 → 프로세스 기동 시 `session_init`이 리스트를 다시 채움.

---

## Slide 3 — 점검 · 발표 요약 (선택)

### 보드 점검 체크리스트

- [ ] `which daemon_y2k38_check y2k38_offsetctl`
- [ ] `y2k38_offsetctl show` → `utc_now`가 신뢰 시계와 일치
- [ ] `cat /etc/y2k38_offset`
- [ ] 데몬 기동 후 `cat /var/run/y2k38_sighup.list`에 각 pid 존재
- [ ] (동적) `ldd $(which daemon_a) | grep y2k38`
- [ ] wrap/SIGHUP 시 stderr에 메시지 (debug 없이도)

### 디렉터리 ↔ 역할 한눈에

| 경로 | 역할 |
|------|------|
| `/usr/bin/*` | Check · A · B · offsetctl 실행 |
| `/usr/lib/liby2k38safe.so*` | 동적 링크 런타임 |
| `/etc/y2k38_offset` | 전 프로세스 공유 OFFSET |
| `/var/run/y2k38_sighup.list` | SIGHUP 구독자 |

### 발표 요약

1. **호스트**에 헤더·lib 설치, **보드**에 바이너리·OFFSET·(선택) `.so`
2. 배포는 **`stage` + `deploy-board.sh`**
3. 설정 핵심은 **`/etc/y2k38_offset` + Check 먼저 기동 + offsetctl로 시각 교정**
