# 디렉토리 및 파일 분석 요청
_Exported on 9/17/2025 at 09:23:10 GMT+9 from Cursor (1.5.9)_

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

