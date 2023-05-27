// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cartridge.generated.h"

UCLASS()
class PROJECTZ_API ACartridge : public AActor
{
	GENERATED_BODY()
	
public:	
	ACartridge();
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CartridgeMesh;
	UPROPERTY(EditAnywhere)
		float ShellEjectImpulse;
	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
};
