# Vehicle Sound System

Chaos Vehicle 기반 차량 사운드 플러그인. 엔진/타이어/바람 등 주행 사운드, 조작음,
인포테인먼트 오디오를 다룬다. UE 5.7.

**이 저장소가 상류(upstream)다.** 프로젝트에 붙은 사본을 직접 고치지 말고 여기서 고친다.

## 구조

```
UVehicleSoundComponent      차량 액터에 붙이는 컴포넌트
  DynamicSoundLayer         주행음 베이스
    Engine / EVMotor / Exhaust / Tire / Wind / Transmission
  InteractionSoundHandler   도어·경적·방향지시등 등 원샷
  InfotainmentSoundHandler  UI음, 음악 플레이어
UVehicleSoundSubsystem      전역 볼륨, 차량 등록
```

차량 상태는 두 경로로 들어온다.

- `bAutoDetectFromChaosVehicle = true` — `UChaosWheeledVehicleMovementComponent`에서
  RPM/속도/기어를 직접 읽는다
- `false` — `SetVehicleSoundState()`로 외부가 밀어넣는다. AI 차량이나 커스텀 물리에 쓴다

## 프로젝트에 붙이기

프로젝트의 `Plugins/` 밑에 이 저장소를 디렉터리 정션으로 연결한다. 사본을 두면
갈라지므로 권장하지 않는다.

```powershell
New-Item -ItemType Junction -Path "<Project>\Plugins\VehicleSoundSystem" `
         -Target "C:\Git\VehicleSoundSystem"
```

그리고 프로젝트의 `.gitignore`에 `Plugins/VehicleSoundSystem/`을 넣어 사본이
프로젝트 저장소에 딸려 들어가지 않게 한다.

## 알려진 미완성 (2026-09-05)

플러그인 자체의 결함이며 프로젝트와 무관하다.

- `RPMToPitchCurve`, `EngineSamples`, `ExhaustSamples`, `SurfaceSounds`,
  `IdleRPM`/`MaxRPM`/`RedlineRPM` — 데이터 에셋에 선언만 되어 있고 코드가 읽지 않는다
- `UVehicleSoundSubsystem` 헤더는 거리 기반 LOD를 제공한다고 적어두었으나 구현이 없다
- 타이어 슬립을 `조향입력 x 속도`로 추정한다. 실제 휠 슬립을 읽어야 한다
- 충돌/충격 사운드 레이어가 없다
- **MetaSound 소스가 없으면 무음이다.** 레이어는 `RPM` 등 이름 붙은 파라미터를
  MetaSound 그래프로 넘길 뿐이고, 샘플 크로스페이드와 피치 시프트는 전부 그래프가 한다.
  이 저장소에는 그래프도 커브도 음원도 들어 있지 않다
