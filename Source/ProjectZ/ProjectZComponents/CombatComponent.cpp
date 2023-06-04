// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 900.f;
	AimWalkSpeed = 350.f;
}
// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

}
// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo,COND_OwnerOnly);
}
//
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}
void UCombatComponent::Fire()
{
	if (CanFire() && EquippedWeapon)
	{
		bCanFire = false;
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.7f;
		}
		StartFireTimer();
	}
}
//
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}
bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)return false;
	
	return !EquippedWeapon->IsEmptry()&&bCanFire&& CombatState==ECombatState::ECS_Unoccupied;
}
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}
//총기 획득후 초기 휴대 탄약량
void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}
//
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
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter+10.f);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		FComponentQueryParams CollisionParams;
		FName TraceTag("TraceUnderCrosshairs");
		CollisionParams.AddIgnoredActor(Character);
		CollisionParams.TraceTag = TraceTag;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			CollisionParams
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if (TraceHitResult.ImpactPoint.IsZero())
		{
			TraceHitResult.ImpactPoint = End;
		}
		if (TraceHitResult.GetActor()&&TraceHitResult.GetActor()!=Character&& TraceHitResult.GetActor()->Implements<UInteractCrosshairsInterface>())
		{
			HUDSet.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDSet.CrosshairsColor = FLinearColor::White;
		}
	}
}
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AProjectZHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
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
				if (bAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.3f, DeltaTime, 30.f);
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
				}

				CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

				HUDSet.CrosshairSpread = 0.3f + CrosshairVelocityFactor + CrosshairIsairFactor - CrosshairAimFactor + CrosshairShootingFactor;
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
				if (bAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.3f, DeltaTime, 30.f);
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
				}

				CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

				HUDSet.CrosshairSpread = 0.3f + CrosshairVelocityFactor + CrosshairIsairFactor - CrosshairAimFactor + CrosshairShootingFactor;
			}
			HUD->SetHUDSet(HUDSet);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
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
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character&& CombatState==ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}

}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)//서버만 호출가능
{
	if (Character == nullptr || WeaponToEquip==nullptr) return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);//Onrep_WeaponState호출
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}
	//setOwner함수는 이미 Actor클래스에서 OnRep으로 지정되어있음
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// 추후 탄약 회복 아이템 추가하면 이코드로 변경(현재는 탄약 회복템이 없어서 초기에 가진 탄약 다쓰면 무기 사용제한이 걸리기에 새로운 무기를 얻을 때마다 초기 Carried값으로 변경해줌
		//CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		CarriedAmmo = StartingARAmmo;
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = StartingARAmmo;
	}
	Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
		this,EquippedWeapon->EquipSound,Character->GetActorLocation());
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState!= ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}
void UCombatComponent::FinishReload()
{
	// 클라, 서버 모두 호출하게 되기 때문에 실제 값변경을 서버만 제어할 수 있기하기위해 HasAuthority검사
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValue();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}
void UCombatComponent::UpdateAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr)return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AProjectZPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}
//서버 RPC (클라이언트라면, 이 함수의 내용을 서버가 대신 수행하도록 요청한다
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr||EquippedWeapon==nullptr)return;
	// 여기 CombatState 변수변경은 서버가 클라이언트의 변수를 변경하는것
	CombatState = ECombatState::ECS_Reloading;
	//클라이언트의 HandleReload를 호출시켜줌
	HandleReload();
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}
void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}
int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)return 0;
	int32 RoomInMag = EquippedWeapon->GetAmmoMaxCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag,AmountCarried);
		return FMath::Clamp(RoomInMag,0,Least);
	}
	return 0;
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this, EquippedWeapon->EquipSound, Character->GetActorLocation());
		}
	}
}


