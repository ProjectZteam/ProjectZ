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
	//액터 컴포넌트는 사전 함수재정의나 컨밴션없이 내장함수로 복제등록가능
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}
void AProjectZCharacter::PostInitializeComponents()//이 클래스를 필요로하는 다른 클래스가 최대한 빨리 초기화를 진행하고싶을 때 사용
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
	//다른 추가 동작 필요시 여기에 작성
}
void AProjectZCharacter::Equip()
{
	if (Combat)
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
//추후에 에임기능을 스위치방식으로할지, pressed상태방식으로할지 확장가능성위해 마우스 중간버튼클릭 액션 함수로 일단 남겨둠, 확정되면 inputaction및 함수 제거 예정
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
		//향상된 입력 연결 함수중 Action이 bool타입인경우 정의,구현 함수에서 인자 전달 필요 없음, void로 구현
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::Equip);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimReleaseAction, ETriggerEvent::Triggered, this, &AProjectZCharacter::AimButtonReleased);
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



