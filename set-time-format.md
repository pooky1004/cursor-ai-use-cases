구현했습니다. `set-time`은 `YYYY-MM-DD:hh:mm:ss` 형식만 받고, 시간 조회용 `get-time` 명령을 추가했습니다.

## 시간 형식

| 형식 | 예시 | 용도 |
|------|------|------|
| **CTL 형식** | `2038-01-19:03:15:48` | `set-time` 입력, `get-time`/`show` 출력 |
| ISO-8601 | `2038-01-19T03:15:48Z` | `show`의 epoch 보조 출력 |
| epoch | `2147483748` | `calibrate` (기존 유지) |

구분자: 날짜와 시각 사이는 **`:`** (하이픈 아님)

---

## 새/변경 명령

### `set-time` — 시각 설정 (UTC)

```bash
# root 권한 필요
y2k38_offsetctl set-time 2038-01-19:03:15:48 \
  --file /etc/y2k38_offset \
  --notify
```

- `settimeofday(2)` → 실패 시 `date -s @<kernel_sec>`
- `/etc/y2k38_offset`에 OFFSET 저장
- `--notify` 시 데몬에 SIGHUP

### `get-time` — 현재 UTC 조회

```bash
y2k38_offsetctl get-time
# 출력: 2038-01-19:03:15:48

y2k38_offsetctl get-time --file /etc/y2k38_offset
```

한 줄로 `YYYY-MM-DD:hh:mm:ss`만 출력합니다 (스크립트 연동용).

### `show` — 상세 조회 (기존 + CTL 형식)

```
offset_file = /etc/y2k38_offset
offset      = 4294967296
kernel_raw  = -2147483548
utc_now     = 2038-01-19:03:15:48
epoch       = 2147483748 (2038-01-19T03:15:48Z)
```

---

## 라이브러리 API

```c
y2k38_format_datetime_ctl(t, buf, buflen);  // → "2038-01-19:03:15:48"
y2k38_parse_datetime_ctl("2038-01-19:03:15:48", &t);
```

2038 이후 날짜도 int64 epoch로 파싱·포맷됩니다.

---

## 사용 예

```bash
# 2038 이후 시각 설정
y2k38_offsetctl set-time 2100-01-01:00:00:00 --notify

# 조회
y2k38_offsetctl get-time

# 상세 확인
y2k38_offsetctl show
```

**참고:** 시간 문자열에 공백이 있으면 셸에서 따옴표로 감싸세요.

```bash
y2k38_offsetctl set-time "2038-01-19:03:15:48" --notify
```