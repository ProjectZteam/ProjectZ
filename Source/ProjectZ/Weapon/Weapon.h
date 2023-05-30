// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	//���� ũ�ν���� �ؽ��ĵ�
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

	//FOV ����
	UPROPERTY(EditAnywhere)
	float ZoomFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
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

	
public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
};
