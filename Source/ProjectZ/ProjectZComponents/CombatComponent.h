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
	AWeapon* EquippedWeapon;//������ ���������Լ� ȣ���ϹǷ� ��������ʿ�
	UPROPERTY(Replicated)//Ŭ�� RPC ��û���� Aiming�������ְ� ���� ���� ����, �����ڽ��̸� �ڱⰪ ���� �� ���� ����
	bool bAiming;

public:

};
