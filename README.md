# Vehicle Sound System

Chaos Vehicle 기반 차량 사운드 플러그인. 엔진/타이어/바람 등 주행 사운드, 조작음,
인포테인먼트 오디오를 다룬다. UE 5.7.

**이 저장소가 상류(upstream)다.** 프로젝트에 붙은 사본을 직접 고치지 말고 여기서 고친다.

## 구조

```
UVehicleSoundComponent      차량 액터에 붙이는 컴포넌트
  DynamicSoundLayer         주행음 베이스
    Engine / EVMotor / Exhaust / Tire / Wind / Transmission
  ImpactSoundHandler        충돌음과 긁힘. 오너의 히트 이벤트를 직접 받는다
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

## 예제 데이터 에셋

`Content/Examples/DA_ExampleCar_Dynamic`은 실제로 튜닝을 마친 스포츠카 설정을 그대로 담은
출발점이다. 복제해서 소리만 꽂으면 된다.

소리도 함께 들어 있다. `Content/Examples/SoundAsset`에 이 예제가 실제로 참조하는 것만
담았다. 원본 팩은 332개 142MB지만 여기 있는 것은 64개 17MB다 - 엔진과 타이어 그래프, 감쇠
셋, 충돌음 네 개, 그리고 그것들이 물고 있는 웨이브와 사운드 클래스뿐이다.

차종별 엔진 그래프는 넣지 않았다. 예제 하나에 팩 전체를 딸려 보낼 이유가 없다.

들어 있는 값 가운데 직접 고르기 어려운 것들:

| 값 | 예제 | 어떻게 나온 값인가 |
|---|---|---|
| `TireSlipThreshold` | 0.10 | 실측에서 정속 코너링이 0.03, 실제 슬라이드가 0.27로 나왔다 |
| `RollingVolumeScale` | 0.35 | 구름소리가 슬라이드 아래에 앉도록. 1.0이면 슬라이드가 묻힌다 |
| `MinGatherAcceleration` | 3000 | 접지력이 낼 수 있는 것보다 크고 충돌보다 작은 값. 약 3g |
| `ImpactGatherWindow` | 0.06 | 밀려나는 차와의 충돌을 모으기에 충분하고, 소리가 늦었다고 느낄 만큼 길지는 않다 |
| `ParameterNameOverrides` | Slip→OnSlip 등 | 붙일 MetaSound의 입력 이름에 맞춰 바꾼다 |

## 데이터 에셋 채우기

칸이 많지만 대부분 비워도 된다. 각 레이어는 **둘 중 하나**만 고르면 된다.

| | MetaSound를 쓸 때 | 일반 SoundWave를 쓸 때 |
|---|---|---|
| `MetaSoundSource` | 그래프 지정 | 루프 wav 지정 |
| `RPMToPitchCurve` / `PitchAtMaxRPM` | **비움** (그래프가 함) | 선택. 없으면 자동 보간 |
| `SurfaceSounds` | **비움** (그래프가 함) | 노면별 루프 |
| `EngineSamples` / `ExhaustSamples` | 비워도 됨 (코드가 안 읽음) | 비움 |

에디터에서 `MetaSoundSource`를 채우면 필요 없는 칸은 자동으로 비활성화된다.

**최소 구성 — 이 세 칸만 채우면 소리가 난다.**

1. `Engine Config > MetaSoundSource` — 엔진 루프 하나
2. `Tire Config > MetaSoundSource` 또는 `SurfaceSounds > Asphalt`
3. `Impact Config > ImpactSounds` — 약한 것부터 강한 것 순으로

**감쇠는 반드시 채운다.** 비워두면 3D 감쇠가 걸리지 않아 거리도 방향도 도플러도
없이 들린다. 여러 차량이 나오는 장면에서는 이것 하나로 체감이 갈린다. 레이어마다
`AttenuationOverride`로 따로 줄 수 있고, 비우면 에셋 공통값을 쓴다. 충돌음은 트랙
반대편까지 가야 하지만 타이어 구름소리는 몇 미터면 되므로 같은 값을 쓰기 어렵다.

나머지 숫자 값들은 기본값이 들어 있고, 차량에 맞춰 조정하는 용도다. 엔진의
`IdleRPM`/`MaxRPM`/`RedlineRPM`은 **차량의 실제 설정과 맞춰야** 피치가 회전수와
따로 놀지 않는다.

## 노면 종류

타이어 소리는 아스팔트냐 자갈이냐에 따라 달라야 한다. Chaos가 바퀴 접지면의
피지컬 머티리얼을 알려주므로 그것으로 판정하는데, `EPhysicalSurface` 값은 프로젝트마다
의미가 다르므로 매핑을 직접 채워야 한다.

`VehicleSoundComponent > SurfaceTypeMapping`에 프로젝트의 피지컬 서피스를
`ETireSurfaceType`에 대응시킨다. 예: `SurfaceType1(자갈) -> Gravel`.

**비워두면 전부 아스팔트로 처리된다.** 포장도로만 달리는 프로젝트라면 그대로 두어도
된다. 가장 심하게 미끄러지는 바퀴의 노면을 채택한다.

## 파라미터 이름이 다른 MetaSound 붙이기

레이어는 항상 자기 이름으로 값을 보낸다.

- 엔진: `RPM`, `NormalizedRPM`, `Speed`, `Throttle`, `EngineLoad`, `Redline`
- 타이어: `Speed`, `SurfaceType`, `Slip`, `Skidding`
- 바람: `Speed`
- 변속기: `RPM`, `Gear`

그래프가 다른 이름을 듣는다면 데이터 에셋의 `ParameterNameOverrides`에 적는다.
예를 들어 `Slip -> OnSlip`, `SurfaceType -> Surface`.

## 충돌 감지

접촉 콜백(`OnActorHit`)과 속도 변화, 두 경로를 함께 쓴다.

접촉 콜백은 접촉점과 표면을 알려주지만 **믿을 수 없다.** Chaos 비동기 물리
(`bSubsteppingAsync=True`)에서는 솔버가 게임 스레드 밖에서 돌아 게임 스레드
델리게이트로 올라오지 않는다. 실측에서 12m 낙하에도 이벤트가 한 건도 오지 않았다.

그래서 기본 경로는 **속도 변화**다. 프레임 간 속도가 급격히 꺾이면 충돌로 본다.
급브레이크는 프레임당 수십 cm/s, 충돌은 수백 cm/s라 `MinImpactSpeed` 하나로
구분된다. 리셋이나 리스폰도 속도를 0으로 만들어 충돌처럼 보이므로, 이동 거리가
속도로 설명되지 않으면 순간이동으로 보고 무시한다.

대가는 접촉점과 표면을 모른다는 것이다. 소리가 충돌 지점이 아니라 차량 위치에서
난다. **긁힘은 접촉 방향이 필요하므로 접촉 콜백이 오는 환경에서만 동작한다.**

## 거리 LOD

차량 대수가 늘면 한 대당 오디오 컴포넌트 여섯 개가 전부 돌기 때문에, 리스너와의
거리로 레이어를 끈다. 거리는 폰이 아니라 카메라 기준으로 잰다.

| 거리 | 도는 레이어 |
|---|---|
| ~ ReducedDistance (25m) | 전부 |
| ~ EngineOnlyDistance (60m) | 엔진·배기·타이어 |
| ~ CullDistance (150m) | 엔진만 |
| 그 이상 | 없음 |

## 알려진 미완성 (2026-09-05)

- `EngineSamples` / `ExhaustSamples` 배열은 MetaSound 그래프가 쓰라고 둔 것이고
  코드는 읽지 않는다. 단일 샘플 경로에서는 필요 없다
- 이 저장소에 MetaSound 그래프도 커브도 음원도 들어 있지 않다
