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

## VR에서 쓸 때

**실내와 실외는 다른 소리다.** 운전석에서 듣는 엔진과 트랙사이드에서 듣는 엔진은 같지
않고, 양쪽을 담은 그래프는 지금 어느 쪽인지 알려주지 않으면 자기 기본값에 머문다. 이
플러그인은 청취자와 차의 거리로 판정해 `InCar`를 보낸다. 경계는
`InteriorListenerRadius`(기본 250cm)다.

거리를 소유 여부가 아니라 **카메라 위치**로 재는 것이 중요하다. 콕핏 시점은 실내,
추격 시점은 실외가 되어야 하는데, 그 둘을 가르는 것은 누가 그 차를 조종하는지가 아니라
귀가 어디에 있는지다.

**감쇠 거리와 LOD 거리를 함께 본다.** 감쇠를 늘려도 LOD가 먼저 레이어를 꺼버리면 소용이
없다. 기본값은 25m에서 타이어가, 60m에서 엔진 외 전부가, 150m에서 전부가 꺼진다.
레이싱 트랙에는 짧으니 트랙 크기에 맞춰 둘 다 올린다.

## 진단

소리가 이상할 때 어느 레이어인지부터 가른다. 모든 레이어가 같은 액터에서 같은 믹스로
들어가므로 귀로는 구별되지 않는다.

    vs.DumpState                  차량별 속도·회전수·슬립·실내여부와 레이어별 실제 볼륨
    vs.LayerVolume tire 0         한 레이어만 끈다. 하나씩 꺼보면 범인이 나온다
    vs.ForceSlip 1                슬립을 고정한다. 그래프가 슬립에 반응하는지 확인
    vs.ForceSlip -1               고정 해제
    vs.SlipThreshold 0.10         슬립 문턱을 즉시 바꾼다

`vs.DumpState`는 고정이 걸려 있으면 그 사실을 함께 찍는다. 고정된 슬립은 조정되지 않는
슬립과 겉보기가 같아서, 한 번 잊으면 문턱만 계속 쫓게 된다.

## MetaSound를 붙일 때 (읽지 않으면 반드시 틀린다)

**입력 이름을 추측하지 말고 그래프를 열어 Members 목록을 본다.** 이 플러그인을 붙이면서
가장 오래 헤맨 두 건이 모두 여기서 나왔다.

- MS_WheelSounds는 입력이 일곱 개인데 흔한 이름 세 개만 찾아 보내고 있었다. 나머지 넷을
  기본값에 방치한 결과, 멀쩡한 차가 세션 내내 림으로 갈리는 소리를 냈다
- `AnyWheelHasTire`는 이름과 반대로 동작한다. 슬립 블록의 게인이 `(1 - AnyWheelHasTire)`
  이므로 1을 넣으면 슬립 소리가 정확히 0으로 곱해진다. 멀쩡한 차는 0이어야 한다

**이름뿐 아니라 의미도 그래프에서 확인한다.** 값이 크기인지 스위치인지도 마찬가지다.
그래프에 `UpdateSound` 같은 트리거가 있다면 소리를 섞는 것이 아니라 갈아끼우는 구조이고,
그때 0.15 같은 중간값은 어느 갈래도 선택하지 못한다. `bSlipParameterIsSwitch`로 어느
쪽인지 지정한다.

## 알려진 미완성 (2026-09-08)

- **스크레이프가 동작하지 않는다.** 프로젝트가 `bSubsteppingAsync=True`면 `OnActorHit`이
  게임 스레드로 오지 않아 접촉 정보가 없다. 충돌음은 속도 변화로 대신 잡지만, 긁는 소리는
  접촉 지점과 방향이 필요해서 대체할 수 없다
- `MS_ScrapeSound`의 `Gain` 입력을 보내지 않는다. 기본값 0.15가 있어 무해하다
- `EngineSamples` / `ExhaustSamples` 배열은 MetaSound 그래프가 쓰라고 둔 것이고
  코드는 읽지 않는다. 단일 샘플 경로에서는 필요 없다
- 노면별 소리는 `SurfaceTypeMapping`을 채워야 동작한다. 비어 있으면 어디를 달려도
  아스팔트로 나가고, 그 사실이 겉으로 드러나지 않는다
