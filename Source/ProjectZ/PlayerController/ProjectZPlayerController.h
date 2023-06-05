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
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	virtual float GetServerTime(); // ���� �ð��� ����ȭ�ϴ� �Լ�
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	//����-Ŭ���̾�Ʈ �ð� ����ȭ

	//Ŭ���̾�Ʈ�� �����ð� ��û�Լ�, ���ڷδ� Ŭ���̾�Ʈ �ڽ��� ȣ����� �ð�
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	//������ Ŭ���̾�Ʈ���� ���� �ڽ��� �ð��� ����
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	//Ŭ���̾�Ʈ�� �����ð��� ����
	float ClientServerDelta = 0.f;
	UPROPERTY(EditAnywhere,Category=Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();
	UFUNCTION(Client,Reliable)
	void ClientJoinMidgame(float Warmup, float Match, float LevelStarting, FName State);
private:
	UPROPERTY()
	class AProjectZHUD* ProjectZHUD;
	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float Warmuptime = 0.f;
	uint32 CountDownInt=0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
};
