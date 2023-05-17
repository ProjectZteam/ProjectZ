// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ProjectZ/ProjectZTypes/TurnInPlace.h"
#include "ProjectZCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
UCLASS()
class PROJECTZ_API AProjectZCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AProjectZCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	UInputMappingContext* ProjectZMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimReleaseAction;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump() override;
	void Equip();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

	void CalculateAimOffset(float DeltaTime);
private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;
	UFUNCTION(Server,Reliable)//신뢰가능 RPC 설정
	void ServerEquipPressed();

	//save AimOffset Value
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartAimRotation;

	ETurnInPlace TurnInPlace;
	void SetturnInPlace(float DeltaTime);
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurnInPlace GetTurnInPlace() const { return TurnInPlace; }
};
