// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/ProjectZComponents/CombatComponent.h"
#include "ProjectZ/ProjectZ.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZ/GameMode/ProjectZMultiGameMode.h"
#include "ProjectZAnimInstance.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
// Sets default values
AProjectZCharacter::AProjectZCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	//액터 컴포넌트는 사전 함수재정의나 컨밴션없이 내장함수로 복제등록가능
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 800.f);
	GetCharacterMovement()->MaxWalkSpeed = 900.f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	TurnInPlace = ETurnInPlace::ETIP_NotTurn;

	//네트워크 업데이트 빈도 설정
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	bIsCrouchPressed = false;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}
void AProjectZCharacter::PostInitializeComponents()//이 클래스를 필요로하는 다른 클래스가 최대한 빨리 초기화를 진행하고싶을 때 사용
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}
// Called when the game starts or when spawned
void AProjectZCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ProjectZMappingContext, 0);
		}
	}

	if (HasAuthority())
	{
		//ApplyDamage 함수가 위임자에의해 호출 이는 다시 ReceiveDamage 호출 (서버만 호출) //this는 ApplyDamage에서 넘어온 OtherActor
		OnTakeAnyDamage.AddDynamic(this,&AProjectZCharacter::ReceiveDamage);
	}
}
void AProjectZCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AProjectZCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}
void AProjectZCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}
void AProjectZCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AProjectZCharacter::ElimTimerFinished,
		ElimDelay
	);
}
void AProjectZCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}
void AProjectZCharacter::MulticastElim_Implementation()
{

	bElimmed = true;
	PlayElimMontage();

	// dissolve시작
	if (DissolveMaterialInstnace1&& DissolveMaterialInstnace2&& DissolveMaterialInstnace3)
	{
		DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstnace1,this);
		DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstnace2, this);
		DynamicDissolveMaterialInstance3 = UMaterialInstanceDynamic::Create(DissolveMaterialInstnace3, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance1);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"), 200.f);
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance2);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow"), 200.f);
		GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstance3);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();
	// 캐릭터 무브먼트 제거
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (ProjectZPlayerController)
	{
		DisableInput(ProjectZPlayerController);
	}
	// 충돌 제거
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//spawn Elimbot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z+200.f);
		ElimBotComponent=UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ElimBotEffect,ElimBotSpawnPoint,GetActorRotation());
	}
}
void AProjectZCharacter::ElimTimerFinished()
{
	AProjectZMultiGameMode* ProjectZMultiGameMode = GetWorld()->GetAuthGameMode<AProjectZMultiGameMode>();
	if (ProjectZMultiGameMode)
	{
		ProjectZMultiGameMode->RequestRespawn(this,Controller);
	}
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}
void AProjectZCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("HitFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AProjectZCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	//클라나 서버의 Health 변경 후 서버에 의해 복제
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
	if (Health == 0.f)
	{
		AProjectZMultiGameMode* ProjectZMultiGameMode = GetWorld()->GetAuthGameMode<AProjectZMultiGameMode>();
		if (ProjectZMultiGameMode)
		{
			ProjectZPlayerController = ProjectZPlayerController == nullptr ? Cast<AProjectZPlayerController>(Controller) : ProjectZPlayerController;
			AProjectZPlayerController* AttackerController = Cast<AProjectZPlayerController>(InstigatorController);
			ProjectZMultiGameMode->PlayerEliminated(this, ProjectZPlayerController, AttackerController);
		}
	}
}
void AProjectZCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AProjectZCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AProjectZCharacter, Health);
}
// Called every frame
void AProjectZCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy&& IsLocallyControlled())//Local or Server
	{
		CalculateAimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedBasedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraCollisionToCharacter();
}
void AProjectZCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue= Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MoveValue.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MoveValue.X);
}

void AProjectZCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue=Value.Get<FVector2D>();
	if (GetController())
	{
		float AimSensitivity = 1.0f;
		if (Combat && Combat->bAiming)
		{
			AimSensitivity = 3.0f;
		}
		AddControllerYawInput(LookAxisValue.X/AimSensitivity);
		AddControllerPitchInput(LookAxisValue.Y/AimSensitivity);
	}
}
void AProjectZCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
		bIsCrouchPressed = true;
	}
	else 
	{
		Super::Jump();
		//다른 추가 동작 필요시 여기에 작성
	}
}
void AProjectZCharacter::Equip()
{
	//Temp 코드 후에 무기 추가되면 EquippedWeapon null검사문장 삭제
	if (Combat&&Combat->EquippedWeapon==nullptr)
	{
		if (HasAuthority())//서버인경우
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//클라이언트인경우
		{
			ServerEquipPressed();
		}
	}
}

void AProjectZCharacter::ServerEquipPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
void AProjectZCharacter::CrouchButtonPressed()
{
	//if CrouchButton already pressed and call Jump from this state, then return
	if (bIsCrouchPressed)
	{
		bIsCrouchPressed = false;
		return;
	}
	//ture 면
	if (bIsCrouched)
	{
		UnCrouch();
	}
	//false면
	else
	{
		Crouch();
	}
	
}
void AProjectZCharacter::AimButtonPressed()
{
	if (Combat&&IsAiming())
	{
		Combat->SetAiming(false);
	}
	else
	{
		Combat->SetAiming(true);
	}
}
//추후에 에임기능을 스위치방식으로할지, pressed상태방식으로할지 확장가능성위해 마우스 중간버튼클릭 액션 함수로 일단 남겨둠, 확정되면 inputaction및 함수 제거 예정
void AProjectZCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}
void AProjectZCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}
void AProjectZCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}
float AProjectZCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
void AProjectZCharacter::CalculateAimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//속도 0이고 공중에 있는게 아니라면 AimOffet Yaw,Pitch 변수 삼각보간으로 계산
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation=FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if (TurnInPlace == ETurnInPlace::ETIP_NotTurn)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		SetturnInPlace(DeltaTime);
	}
	//run or jump
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
	}
	//값 복제시 각도는 압축해서 네트워크전송되기때문에 후처리 필요
	CalculateAO_Pitch();
}
void AProjectZCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
void AProjectZCharacter::SimProxiesTurn()
{
	if (Combat==nullptr || Combat->EquippedWeapon == nullptr)return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw=UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_Left;
		}
		else
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		}
		return;
	}
	TurnInPlace = ETurnInPlace::ETIP_NotTurn;
}
void AProjectZCharacter::SetturnInPlace(float DeltaTime)
{
	if (AO_Yaw > 45.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -55.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Left;
	}
	if (TurnInPlace != ETurnInPlace::ETIP_NotTurn)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurn;
			StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
// Called to bind functionality to input
void AProjectZCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,this,&AProjectZCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Look);
		//향상된 입력 연결 함수중 Action이 bool타입인경우 정의,구현 함수에서 인자 전달 필요 없음, void로 구현
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Equip);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimReleaseAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireReleaseAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::FireButtonReleased);
	}
}


//Weapon변수 클라이언트로 복제될시 호출
void AProjectZCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)// 겹치는무기 null 아닐시
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)//마지막 바뀌기 전값이 null이 아니면
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
void AProjectZCharacter::HideCameraCollisionToCharacter()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation()-GetActorLocation()).Size()<CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}
void AProjectZCharacter::OnRep_Health()
{
	//클라이언트라면 서버의 값 복제로 Health값 변경된 걸 알고나면 OnRep_Health호출
	UpdateHUDHealth();
	if (!bElimmed)
	{
		PlayHitReactMontage();
	}
	
}
void AProjectZCharacter::UpdateHUDHealth()
{
	ProjectZPlayerController = ProjectZPlayerController == nullptr ? Cast<AProjectZPlayerController>(Controller) : ProjectZPlayerController;
	if (ProjectZPlayerController)
	{
		ProjectZPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}
void AProjectZCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance1 && DynamicDissolveMaterialInstance2 && DynamicDissolveMaterialInstance3)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}
void AProjectZCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this,&AProjectZCharacter::UpdateDissolveMaterial);
	if (DissolveCurve&&DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		DissolveTimeline->Play();
	}
}
//Weapon Sphere overlap시 소유주 위젯 show 추가 check안할시 위젯 모두에게 나옴
void AProjectZCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool AProjectZCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AProjectZCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AProjectZCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AProjectZCharacter::GetHitTarget() const
{

	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}



