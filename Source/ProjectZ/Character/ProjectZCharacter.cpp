// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/ProjectZComponents/CombatComponent.h"
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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}
void AProjectZCharacter::PostInitializeComponents()//�� Ŭ������ �ʿ���ϴ� �ٸ� Ŭ������ �ִ��� ���� �ʱ�ȭ�� �����ϰ���� �� ���
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
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
	Super::Jump();
	//�ٸ� �߰� ���� �ʿ�� ���⿡ �ۼ�
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
	if (bIsCrouched)
	{
		UnCrouch();
	}
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



