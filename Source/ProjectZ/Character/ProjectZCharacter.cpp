// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	mPAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SOUND"));
	mPAudioComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<USoundCue> jumpSound(TEXT("/Game/Assets/ParagonWraith/Audio/Cues/Wraith_Effort_Jump"));
	if (jumpSound.Succeeded())
	{
		mPJumpSound = jumpSound.Object;
	}
	static ConstructorHelpers::FObjectFinder<USoundCue> equipSound(TEXT("/Game/Assets/MilitaryWeapSilver/Sound/Rifle/Cues/Rifle_Lower_Cue"));
	if (equipSound.Succeeded())
	{
		mPEquipSound = equipSound.Object;
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
/*���� ����� �Ǵ� �� ������ �ȵǼ� ����
void AProjectZCharacter::Jump(const FInputActionValue& Value)
{
	mPAudioComponent->SetSound(mPJumpSound);
	mPAudioComponent->Play();
}*/
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
	if (IsWeaponEquipped()&&bIsWeaponChanged)
	{	
		bIsWeaponChanged = false;
		mPAudioComponent->SetSound(mPEquipSound);
		mPAudioComponent->Play();
	}
}
void AProjectZCharacter::ServerEquipPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
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
	}
}
void AProjectZCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

//Weapon���� �����ɽ� ȣ��
void AProjectZCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
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
	bIsWeaponChanged = true;
}

bool AProjectZCharacter::IsWeaponEquipped()
{

	return (Combat && Combat->EquippedWeapon);
}



