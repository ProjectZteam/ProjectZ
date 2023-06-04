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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime(); // 서버 시간에 동기화하는 함수
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	//서버-클라이언트 시간 동기화

	//클라이언트가 서버시간 요청함수, 인자로는 클라이언트 자신의 호출시점 시간
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	//서버가 클라이언트에게 서버 자신의 시간을 응답
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	//클라이언트와 서버시간의 차이
	float ClientServerDelta = 0.f;
	UPROPERTY(EditAnywhere,Category=Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
private:
	UPROPERTY()
	class AProjectZHUD* ProjectZHUD;

	float MatchTime = 120.f;
	uint32 CountDownInt=0;
};
