// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectZ/HUD/ProjectZHUD.h"
#include "ProjectZ/Weapon/WeaponTypes.h"
#include "ProjectZ/ProjectZTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZ_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AProjectZCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReload();
	void FireButtonPressed(bool bPressed);
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming); //RPC
	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	//Reload RPC
	UFUNCTION(Server,Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();
private:
	UPROPERTY()
	class AProjectZCharacter* Character;
	UPROPERTY()
	class AProjectZPlayerController* Controller;
	UPROPERTY()
	class AProjectZHUD* HUD;
	FHUDSet HUDSet;
	float CrosshairVelocityFactor;
	float CrosshairIsairFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;//서버만 무기장착함수 호출하므로 복제등록필요
	UPROPERTY(Replicated)//클라 RPC 요청으로 Aiming변경해주고 변수 복제 전파, 서버자신이면 자기값 변경 후 복제 전파 
	bool bAiming;
	float DefaultFOV;
	UPROPERTY(EditAnywhere,Category=Combat)
	float ZoomedFOV = 0.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 0.f;

	float CurrentFOV;

	void InterpFOV(float DeltaTime);

	//auto fire
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	//현재 장착무기 탄약보유량
	UFUNCTION()
	void OnRep_CarriedAmmo();
	TMap<EWeaponType, int32> CarriedAmmoMap;
	//총기 초기 보유 총 탄약량
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 210;
	void InitializeCarriedAmmo();
	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState=ECombatState::ECS_Unoccupied;

	void UpdateAmmoValue();
	UFUNCTION()
	void OnRep_CombatState();
	bool bFireButtonPressed;
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
public:

};
