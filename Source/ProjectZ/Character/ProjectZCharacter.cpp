// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/ProjectZComponents/CombatComponent.h"
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
	//���� ������Ʈ�� ���� �Լ������ǳ� ����Ǿ��� �����Լ��� ������ϰ���
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 800.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	TurnInPlace = ETurnInPlace::ETIP_NotTurn;

	//��Ʈ��ũ ������Ʈ �� ����
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	bIsCrouchPressed = false;
}
void AProjectZCharacter::PostInitializeComponents()//�� Ŭ������ �ʿ���ϴ� �ٸ� Ŭ������ �ִ��� ���� �ʱ�ȭ�� �����ϰ���� �� ���
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
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
void AProjectZCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AProjectZCharacter, OverlappingWeapon,COND_OwnerOnly);
}
// Called when the game starts or when spawned
void AProjectZCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ProjectZMappingContext,0);
		}
	}
}
// Called every frame
void AProjectZCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateAimOffset(DeltaTime);
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
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
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
		//�ٸ� �߰� ���� �ʿ�� ���⿡ �ۼ�
	}
}
void AProjectZCharacter::Equip()
{
	if (Combat)
	{
		if (HasAuthority())//�����ΰ��
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//Ŭ���̾�Ʈ�ΰ��
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
	//ture ��
	if (bIsCrouched)
	{
		UnCrouch();
	}
	//false��
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
//���Ŀ� ���ӱ���� ����ġ�����������, pressed���¹���������� Ȯ�尡�ɼ����� ���콺 �߰���ưŬ�� �׼� �Լ��� �ϴ� ���ܵ�, Ȯ���Ǹ� inputaction�� �Լ� ���� ����
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
void AProjectZCharacter::CalculateAimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//�ӵ� 0�̰� ���߿� �ִ°� �ƴ϶�� AimOffet Yaw,Pitch ���� �ﰢ�������� ���
	if (Speed == 0.f && !bIsInAir)
	{
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
	if (Speed > 0.f || bIsInAir)
	{
		StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
	}
	//�� ������ ������ �����ؼ� ��Ʈ��ũ���۵Ǳ⶧���� ��ó�� �ʿ�
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
void AProjectZCharacter::SetturnInPlace(float DeltaTime)
{
	if (AO_Yaw > 40.f)
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
		//���� �Է� ���� �Լ��� Action�� boolŸ���ΰ�� ����,���� �Լ����� ���� ���� �ʿ� ����, void�� ����
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Equip);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimReleaseAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireReleaseAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::FireButtonReleased);
	}
}


//Weapon���� Ŭ���̾�Ʈ�� �����ɽ� ȣ��
void AProjectZCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)// ��ġ�¹��� null �ƴҽ�
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)//������ �ٲ�� ������ null�� �ƴϸ�
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
//Weapon Sphere overlap�� ������ ���� show �߰� check���ҽ� ���� ��ο��� ����
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



