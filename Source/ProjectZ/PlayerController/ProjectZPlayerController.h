// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectZPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health,float MaxHealth);
protected:
	virtual void BeginPlay() override;
private:
	class AProjectZHUD* ProjectZHUD;
};
