// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZ/HUD/ProjectZHUD.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 350.f;
}
// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

}
// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshairs(DeltaTime);
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
	}
}
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr|| Character->Controller==nullptr) return;
	Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AProjectZHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			FHUDSet HUDSet;
			if (EquippedWeapon)
			{
				HUDSet.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDSet.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDSet.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDSet.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDSet.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDSet.CrosshairsCenter = nullptr;
				HUDSet.CrosshairsRight = nullptr;
				HUDSet.CrosshairsLeft = nullptr;
				HUDSet.CrosshairsTop = nullptr;
				HUDSet.CrosshairsBottom = nullptr;
			}
			if (Character->bIsCrouched)
			{
				FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeedCrouched);
				FVector2D VelocityMultiplierRange(0.f, 0.5f);
				FVector Velocity = Character->GetVelocity();
				Velocity.Z = 0.f;
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
				if (Character->GetCharacterMovement()->IsFalling())
				{
					CrosshairIsairFactor = FMath::FInterpTo(CrosshairIsairFactor, 2.f, DeltaTime, 30.f);
				}
				else
				{
					CrosshairIsairFactor = FMath::FInterpTo(CrosshairIsairFactor, 0.f, DeltaTime, 30.f);
				}
				HUDSet.CrosshairSpread = CrosshairVelocityFactor+CrosshairIsairFactor;
			}
			else
			{
				FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
				FVector2D VelocityMultiplierRange(0.f, 1.f);
				FVector Velocity = Character->GetVelocity();
				Velocity.Z = 0.f;
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
				if (Character->GetCharacterMovement()->IsFalling())
				{
					CrosshairIsairFactor = FMath::FInterpTo(CrosshairIsairFactor, 2.f, DeltaTime, 2.f);
				}
				else
				{
					CrosshairIsairFactor = FMath::FInterpTo(CrosshairIsairFactor, 0.f, DeltaTime, 30.f);
				}
				HUDSet.CrosshairSpread = CrosshairVelocityFactor + CrosshairIsairFactor;
			}
			HUD->SetHUDSet(HUDSet);
		}
	}
}
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon&&Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		
	}
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)//서버만 호출가능
{
	if (Character == nullptr || WeaponToEquip==nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);//Onrep_WeaponState호출
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}
	//setOwner함수는 이미 Actor클래스에서 OnRep으로 지정되어있음
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}


