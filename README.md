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

## MetaSound 없이 쓰기

MetaSound 그래프가 없어도 동작한다. 레이어에 일반 `SoundWave` 루프를 넣으면
코드가 직접 피치와 볼륨을 조절한다. 그래프는 나중에 품질을 올릴 때 얹는다.

- 엔진: `RPMToPitchCurve`가 있으면 그걸 쓰고, 없으면 `PitchAtMaxRPM`까지 선형 보간한다.
  루프 하나로도 RPM에 따라 변하는 엔진 소리가 난다
- 타이어: `MetaSoundSource`가 비어 있으면 `SurfaceSounds`에서 노면에 맞는 샘플을 골라 튼다

## 알려진 미완성 (2026-09-05)

- `UVehicleSoundSubsystem` 헤더는 거리 기반 LOD를 제공한다고 적어두었으나 구현이 없다
- 충돌/충격 사운드 레이어가 없다
- `EngineSamples` / `ExhaustSamples` 배열은 MetaSound 그래프가 쓰라고 둔 것이고
  코드는 읽지 않는다. 단일 샘플 경로에서는 필요 없다
- 이 저장소에 MetaSound 그래프도 커브도 음원도 들어 있지 않다
