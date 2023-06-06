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
	AWeapon* EquippedWeapon;//������ ���������Լ� ȣ���ϹǷ� ��������ʿ�
	UPROPERTY(Replicated)//Ŭ�� RPC ��û���� Aiming�������ְ� ���� ���� ����, �����ڽ��̸� �ڱⰪ ���� �� ���� ���� 
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
	//���� �������� ź�ຸ����
	UFUNCTION()
	void OnRep_CarriedAmmo();
	TMap<EWeaponType, int32> CarriedAmmoMap;
	//�ѱ� �ʱ� ���� �� ź�෮
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
