// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


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
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming); //RPC
private:
	class AProjectZCharacter* Character;
	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;//서버만 무기장착함수 호출하므로 복제등록필요
	UPROPERTY(Replicated)//클라 RPC 요청으로 Aiming변경해주고 변수 복제 전파, 서버자신이면 자기값 변경 후 복제 전파
	bool bAiming;

public:

};
