// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName= "Initial Weapon State"),
	EWS_Equipped UMETA(DisplayName= "Equipped"),
	EWS_Dropped UMETA(DisplayName= "Dropped"),
	EWS_MAX UMETA(DisplayName= "DefaultMax")
};
UCLASS()
class PROJECTZ_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	bool IsEmptry();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	//무기 크로스헤어 텍스쳐들
	UPROPERTY(Editanywhere, Category = Crosshairs)
	class UTexture2D* CrosshairCenter;
	UPROPERTY(Editanywhere, Category = Crosshairs)
	UTexture2D* CrosshairRight;
	UPROPERTY(Editanywhere, Category = Crosshairs)
	UTexture2D* CrosshairLeft;
	UPROPERTY(Editanywhere, Category = Crosshairs)
	UTexture2D* CrosshairTop;
	UPROPERTY(Editanywhere, Category = Crosshairs)
	UTexture2D* CrosshairBottom;

	//FOV 변수
	UPROPERTY(EditAnywhere)
	float ZoomFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);
private:
	UPROPERTY(VisibleAnywhere, Category= "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ACartridge> CartridgeClass;
	// 총 최대장전량과 초기 탄약수는 blueprint설정
	UPROPERTY(EditAnywhere,ReplicatedUsing=OnRep_Ammo, Category = "Weapon Properties")
	int32 Ammo;
	UFUNCTION()
	void OnRep_Ammo();
	void SpendRound();
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 AmmoMaxCapacity;
	UPROPERTY()
	class AProjectZCharacter* ProjectZOwnerCharacter;
	UPROPERTY()
	class AProjectZPlayerController* ProjectZOwnerController;
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetAmmoMaxCapacity() const { return AmmoMaxCapacity; }
};
