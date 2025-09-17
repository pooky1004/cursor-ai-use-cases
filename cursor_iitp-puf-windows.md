# 디렉토리 및 파일 분석 요청
_Exported on 9/17/2025 at 13:22:22 GMT+9 from Cursor (1.5.9)_

---

**User**

이 코드들은 C#으로 개발된 Windows 프로그램이다.
디렉토리 별, 파일 별, 기능별 분석을 상세히 해서 알려 달라

---

**Cursor**

이 C# Windows 프로그램을 상세히 분석해드리겠습니다. 먼저 주요 파일들을 읽어서 구조와 기능을 파악해보겠습니다.


이제 추가 파일들도 읽어보겠습니다.


이제 C# Windows 프로그램에 대한 상세한 분석을 제공하겠습니다.

## 📁 디렉토리별 분석

### 1. **루트 디렉토리** (`/home/pooky/WorkSpace/2023-IITP-PUF-LK1/TestProgram/`)
- **프로젝트 메인 파일들**
  - `PFT.csproj` - Visual Studio 프로젝트 파일 (.NET Framework 4.7.2)
  - `PFT.sln` - Visual Studio 솔루션 파일
  - `App.config` - 애플리케이션 설정 파일
  - `README.md` - 프로젝트 설명서

### 2. **bin/Debug/** 디렉토리
- **컴파일된 실행 파일들**
  - `PFT.exe` - 메인 실행 파일
  - `PFT.exe.config` - 실행 시 설정 파일
  - `PFT.pdb` - 디버그 심볼 파일
  - `FTD2XX_NET.dll` - FTDI USB 통신 라이브러리
  - `FTD2XX_NET.xml` - 라이브러리 문서 파일

### 3. **obj/Debug/** 디렉토리
- **빌드 중간 파일들**
  - 컴파일 캐시 파일들
  - 리소스 파일들
  - 임시 빌드 파일들

### 4. **Properties/** 디렉토리
- **애플리케이션 속성 파일들**
  - `AssemblyInfo.cs` - 어셈블리 메타데이터
  - `Resources.Designer.cs` - 리소스 관리자
  - `Resources.resx` - 리소스 파일
  - `Settings.Designer.cs` - 설정 관리자
  - `Settings.settings` - 사용자 설정

### 5. **Resources/** 디렉토리
- **리소스 파일들**
  - `FTD2XX_NET.dll` - FTDI 라이브러리 원본
  - `FTD2XX_NET.XML` - 라이브러리 문서
  - `eft.ico` - 아이콘 파일

## 📄 파일별 상세 분석

### 1. **Program.cs** - 애플리케이션 진입점
```csharp
static class Program
{
    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new MainForm());
    }
}
```
- **기능**: Windows Forms 애플리케이션의 메인 진입점
- **특징**: STA 스레드 모델 사용, MainForm 인스턴스 생성

### 2. **MainForm.cs** - 메인 폼 로직
- **클래스**: `MainForm : Form`
- **주요 기능**:
  - **PUF (Physically Unclonable Function) 테스트**
  - **FTDI USB 통신**
  - **레지스터 읽기/쓰기**
  - **암호화/복호화 테스트**
  - **ECC (Error Correction Code) 테스트**

#### 핵심 메서드들:
- `PufEncTest()` - PUF 암호화 테스트
- `PufEccTest()` - PUF ECC 인코딩/디코딩 테스트
- `WriteRegister()` / `ReadRegister()` - 레지스터 접근
- `Encrypt()` / `EccWrite()` / `EccRead()` - 암호화 및 ECC 기능

### 3. **MainForm.Designer.cs** - UI 디자인
- **UI 구성요소**:
  - **Connection 그룹**: 디바이스 선택, 연결/해제
  - **Access Test 그룹**: 레지스터 읽기/쓰기 테스트
  - **PUF Test 그룹**: PUF 기능 테스트
  - **Message 그룹**: 로그 출력

### 4. **ftd2xx.cs** - FTDI USB 통신
- **클래스**: `MainForm`의 partial class
- **주요 기능**:
  - FTDI 디바이스 열거 및 연결
  - USB 통신 설정 (타임아웃, 플로우 컨트롤 등)
  - 데이터 송수신
  - 버퍼 관리

## �� 기능별 상세 분석

### 1. **PUF (Physically Unclonable Function) 테스트**
- **목적**: 하드웨어 기반 보안 기능 테스트
- **테스트 데이터**:
  - `encrypted[]` - 암호화된 참조 데이터 (1024바이트)
  - `masterkey[]` - 마스터 키 (1024바이트)
  - `plaintext[]` - 평문 데이터 (1024바이트)

#### 암호화 테스트 (`PufEncTest`)
- 100회 반복 테스트
- 16바이트 평문 + 16바이트 마스터키 → 18바이트 암호문
- 참조 데이터와 비교하여 정확성 검증

#### ECC 테스트 (`PufEccTest`)
- 100회 반복 테스트
- 16바이트 데이터 + 2바이트 ECC 코드
- 의도적 2비트 오류 삽입 후 복구 테스트

### 2. **통신 프로토콜**
- **프레임 구조**: `0xCA 0xBA` 헤더
- **레지스터 접근**: 주소 기반 읽기/쓰기
- **암호화 명령**: `0x02` (패킷 모드)
- **ECC 명령**: `0x04` (읽기), `0x06` (쓰기)

### 3. **UI 기능**
- **디바이스 관리**: FTDI 디바이스 자동 감지 및 선택
- **실시간 테스트**: 레지스터 접근 및 PUF 기능 테스트
- **로그 시스템**: 상세한 테스트 결과 출력
- **설정 저장**: 사용자 설정 자동 저장/복원

### 4. **하드웨어 인터페이스**
- **FTDI FT2232H**: USB-to-FIFO 변환 칩
- **동기식 통신**: 16ms 지연시간 설정
- **플로우 컨트롤**: RTS/CTS 사용
- **타임아웃**: 5초 송수신 타임아웃

## 🎯 프로그램 목적

이 프로그램은 **PCC (Physical Cryptography Chip) FPGA 보드**를 테스트하기 위한 Windows 애플리케이션입니다:

1. **PUF 기능 검증**: 하드웨어 기반 보안 기능 테스트
2. **통신 검증**: USB를 통한 FPGA와의 데이터 교환
3. **암호화 성능**: 암호화/복호화 정확성 검증
4. **오류 복구**: ECC를 통한 데이터 무결성 검증

프로그램은 연구/개발 목적으로 제작된 것으로 보이며, LK1 Inc.에서 개발한 PCC 프로젝트의 일부입니다.

```csharp
static class Program
{
    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new MainForm());
    }
}
```

---

**User**

레지스터 읽기/쓰기, 암호화/복호화 테스트, ECC (Error Correction Code) 테스트 등을 하기 위해
메시지를 구성하는 부분을 메시지 포맷 등 상세히 설명해 줘

---

**Cursor**

메시지 포맷과 통신 프로토콜을 상세히 분석해드리겠습니다.


이제 메시지 포맷과 통신 프로토콜을 상세히 분석해드리겠습니다.

## 📡 통신 프로토콜 메시지 포맷

### 1. **기본 프레임 구조**

모든 메시지는 다음과 같은 공통 헤더를 가집니다:

```
[0xCA][0xBA][Length_Low][Length_High][Address/Command][Data...]
```

#### **헤더 필드 상세 분석**

| 바이트 | 필드명 | 설명 | 비트 구성 |
|--------|--------|------|-----------|
| 0 | Header1 | 프레임 시작 식별자 | `0xCA` |
| 1 | Header2 | 프레임 시작 식별자 | `0xBA` |
| 2 | Length_Low | 하위 4비트: 길이의 하위 4비트<br/>상위 4비트: 명령 타입 | `((len & 0xF) << 4) \| cmd_type` |
| 3 | Length_High | 길이의 상위 8비트 | `len >> 4` |
| 4 | Address/Command | 주소 또는 명령어 | 레지스터 접근 시 주소, 기능 명령 시 명령어 |
| 5+ | Data | 실제 데이터 | 페이로드 |

### 2. **레지스터 읽기/쓰기 메시지**

#### **레지스터 쓰기 (WriteRegister)**

**요청 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Data...]
```

**구체적 구성:**
- `wbuf[0] = 0xCA` - 헤더1
- `wbuf[1] = 0xBA` - 헤더2  
- `wbuf[2] = (byte)(((wlen & 0xF) << 4) | 0x0)` - 길이 하위4비트 + 명령타입(0=쓰기)
- `wbuf[3] = (byte)(wlen >> 4)` - 길이 상위8비트
- `wbuf[4] = (byte)(((addr & 0x3F) << 2) | 0x1)` - 주소 하위6비트 + 쓰기플래그(1)
- `wbuf[5] = (byte)(addr >> 6)` - 주소 상위8비트
- `wbuf[6+i] = buf[i]` - 실제 데이터

**응답 메시지:**
```
[0xCA][0xBA][0x60][0x00][Address_Info]
```
- `0x60 0x00` = 길이 6바이트 (고정)
- 주소 정보는 요청과 동일해야 함

#### **레지스터 읽기 (ReadRegister)**

**요청 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Read_Length]
```

**구체적 구성:**
- `wbuf[0] = 0xCA` - 헤더1
- `wbuf[1] = 0xBA` - 헤더2
- `wbuf[2] = (byte)(((wlen & 0xF) << 4) | 0x0)` - 길이 하위4비트 + 명령타입(0=읽기)
- `wbuf[3] = (byte)(wlen >> 4)` - 길이 상위8비트
- `wbuf[4] = (byte)(((addr & 0x3F) << 2) | 0x0)` - 주소 하위6비트 + 읽기플래그(0)
- `wbuf[5] = (byte)(addr >> 6)` - 주소 상위8비트
- `wbuf[6] = (byte)len` - 읽을 바이트 수

**응답 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Data...]
```
- 길이는 `6 + 읽은데이터길이`
- 주소 정보는 요청과 동일해야 함

### 3. **암호화/복호화 메시지**

#### **암호화 (Encrypt) - 패킷 모드**

**요청 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][0x02][32바이트_데이터]
```

**구체적 구성:**
- `wbuf[0] = 0xCA` - 헤더1
- `wbuf[1] = 0xBA` - 헤더2
- `wbuf[2] = (byte)(((wlen & 0xF) << 4) | 0x1)` - 길이 하위4비트 + 명령타입(1=기능명령)
- `wbuf[3] = (byte)(wlen >> 4)` - 길이 상위8비트 (37바이트)
- `wbuf[4] = 0x02` - 암호화 명령
- `wbuf[5~36]` - 32바이트 입력 데이터 (16바이트 평문 + 16바이트 키)

**응답 메시지:**
```
[0xCA][0xBA][0x71][0x01][0x02][18바이트_암호문]
```
- `0x71 0x01` = 길이 23바이트 (4+1+18)
- `0x02` = 암호화 명령 확인
- 18바이트 암호문 (16바이트 암호화된 데이터 + 2바이트 추가 정보)

#### **레지스터 모드 암호화**

1. **데이터 입력**: `WriteRegister(0x10, buf, 32)` - 32바이트 데이터를 주소 0x10에 쓰기
2. **명령 실행**: `WriteRegister(0x30, {0x82}, 1)` - 암호화 명령(0x82)을 주소 0x30에 쓰기
3. **완료 대기**: `ReadRegister(0x30, cmd, 1)` - 비트7이 0이 될 때까지 대기
4. **결과 읽기**: `ReadRegister(0x40, buf, 18)` - 18바이트 결과를 주소 0x40에서 읽기

### 4. **ECC (Error Correction Code) 메시지**

#### **ECC 쓰기 (EccWrite) - 패킷 모드**

**요청 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][0x06][18바이트_데이터]
```

**구체적 구성:**
- `wbuf[0] = 0xCA` - 헤더1
- `wbuf[1] = 0xBA` - 헤더2
- `wbuf[2] = (byte)(((wlen & 0xF) << 4) | 0x1)` - 길이 하위4비트 + 명령타입(1=기능명령)
- `wbuf[3] = (byte)(wlen >> 4)` - 길이 상위8비트 (23바이트)
- `wbuf[4] = 0x06` - ECC 쓰기 명령
- `wbuf[5~22]` - 18바이트 데이터 (16바이트 원본 + 2바이트 ECC)

**응답 메시지:**
```
[0xCA][0xBA][0x71][0x01][0x06][18바이트_처리된_데이터]
```

#### **ECC 읽기 (EccRead) - 패킷 모드**

**요청 메시지:**
```
[0xCA][0xBA][Length_Low][Length_High][0x04][18바이트_데이터]
```

**구체적 구성:**
- `wbuf[0] = 0xCA` - 헤더1
- `wbuf[1] = 0xBA` - 헤더2
- `wbuf[2] = (byte)(((wlen & 0xF) << 4) | 0x1)` - 길이 하위4비트 + 명령타입(1=기능명령)
- `wbuf[3] = (byte)(wlen >> 4)` - 길이 상위8비트 (23바이트)
- `wbuf[4] = 0x04` - ECC 읽기 명령
- `wbuf[5~22]` - 18바이트 오류가 포함된 데이터

**응답 메시지:**
```
[0xCA][0xBA][0x71][0x01][0x04][18바이트_복구된_데이터]
```

### 5. **명령어 코드 매핑**

| 명령어 | 코드 | 설명 | 모드 |
|--------|------|------|------|
| Register Write | `0x00` | 레지스터 쓰기 | 레지스터 |
| Register Read | `0x00` | 레지스터 읽기 | 레지스터 |
| Encryption | `0x02` | 암호화 | 패킷/레지스터 |
| ECC Read | `0x04` | ECC 읽기/복구 | 패킷/레지스터 |
| ECC Write | `0x06` | ECC 쓰기/인코딩 | 패킷/레지스터 |
| Encryption Cmd | `0x82` | 암호화 명령 | 레지스터 |
| ECC Read Cmd | `0x84` | ECC 읽기 명령 | 레지스터 |
| ECC Write Cmd | `0x86` | ECC 쓰기 명령 | 레지스터 |

### 6. **주소 매핑 (레지스터 모드)**

| 주소 | 용도 | 설명 |
|------|------|------|
| `0x10` | 데이터 입력 | 32바이트 입력 데이터 |
| `0x30` | 명령 레지스터 | 명령 실행 및 상태 확인 |
| `0x40` | 결과 출력 | 18바이트 처리 결과 |

### 7. **응답 검증**

모든 응답은 다음을 검증합니다:
- 헤더: `0xCA 0xBA`
- 길이: 예상 길이와 일치
- 명령/주소: 요청과 일치
- 상태: 오류 없음

이 프로토콜은 FPGA와의 안정적인 통신을 위해 설계되었으며, 레지스터 기반의 직접 접근과 패킷 기반의 고수준 명령을 모두 지원합니다.

```plaintext
[0xCA][0xBA][Length_Low][Length_High][Address/Command][Data...]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Data...]
```

```plaintext
[0xCA][0xBA][0x60][0x00][Address_Info]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Read_Length]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][Address_Info][Data...]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][0x02][32바이트_데이터]
```

```plaintext
[0xCA][0xBA][0x71][0x01][0x02][18바이트_암호문]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][0x06][18바이트_데이터]
```

```plaintext
[0xCA][0xBA][0x71][0x01][0x06][18바이트_처리된_데이터]
```

```plaintext
[0xCA][0xBA][Length_Low][Length_High][0x04][18바이트_데이터]
```

```plaintext
[0xCA][0xBA][0x71][0x01][0x04][18바이트_복구된_데이터]
```

