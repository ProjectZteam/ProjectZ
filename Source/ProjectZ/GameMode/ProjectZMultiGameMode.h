// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ProjectZ/PlayerController/ProjectZPlayerController.h"
#include "ProjectZMultiGameMode.generated.h"

namespace MatchState
{
	extern PROJECTZ_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}
/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZMultiGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AProjectZMultiGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AProjectZCharacter* ElimmedCharacter,class AProjectZPlayerController* VictimController, AProjectZPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	virtual void OnMatchStateSet() override;
	float GetLevelStartingTime() const;
	void ShuffleArray(TArray<AActor*>& PlayerStart);
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 180.f;
	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	float CountdownTime = 0.f;
};
