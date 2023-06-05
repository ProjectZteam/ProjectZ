// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZPlayerController.h"
#include "ProjectZ/Character/ProjectZCharacter.h"
#include "ProjectZ/HUD/ProjectZHUD.h"
#include "ProjectZ/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "ProjectZ/GameMode/ProjectZMultiGameMode.h"
#include "ProjectZ/PlayerState/ProjectZPlayerState.h"

void AProjectZPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ProjectZHUD=Cast<AProjectZHUD>(GetHUD());


}
void AProjectZPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectZPlayerController, MatchState);
}
void AProjectZPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();

	CheckTimeSync(DeltaTime);
	PollInit();
}
void AProjectZPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}
void AProjectZPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
		if (ProjectZHUD)
		{
			ProjectZHUD->AddCharacterOverlay();
		}
	}
}
void AProjectZPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AProjectZPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
		if (ProjectZHUD)
		{
			ProjectZHUD->AddCharacterOverlay();
		}
	}
}

void AProjectZPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->HealthBar && ProjectZHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ProjectZHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ProjectZHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AProjectZPlayerController::SetHUDScore(float Score)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ProjectZHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AProjectZPlayerController::SetHUDDefeats(int32 Defeats)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		ProjectZHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AProjectZPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ProjectZHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AProjectZPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString CarriedText = FString::Printf(TEXT("%d"), CarriedAmmo);
		ProjectZHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedText));
	}
}

void AProjectZPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	ProjectZHUD = ProjectZHUD == nullptr ? Cast<AProjectZHUD>(GetHUD()) : ProjectZHUD;
	bool bHUDValid = ProjectZHUD && ProjectZHUD->CharacterOverlay && ProjectZHUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime/60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString MatchCountDownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		ProjectZHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(MatchCountDownText));
	}
}

void AProjectZPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AProjectZCharacter* ProjectZCharacter = Cast<AProjectZCharacter>(InPawn);
	if (ProjectZCharacter)
	{
		SetHUDHealth(ProjectZCharacter->GetHealth(), ProjectZCharacter->GetMaxHealth());
	}
}

void AProjectZPlayerController::SetHUDTime()
{
	uint32 LeftTime = FMath::CeilToInt(MatchTime-GetServerTime());
	if (CountDownInt != LeftTime)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountDownInt = LeftTime;
}
void AProjectZPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ProjectZHUD && ProjectZHUD->CharacterOverlay)
		{
			CharacterOverlay = ProjectZHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}
//Ŭ���̾�Ʈ�� ȣ���ϰ� ������ ��������
void AProjectZPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	//�����ð�
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	//Ŭ���̾�Ʈ�� ȣ���� ������ �����ڽ� �ð� ����
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceipt);
}

void AProjectZPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	//������ ��û���� Ŭ���̾�Ʈ�� �� �Լ� ȣ���ϸ� ������� ����ð�-�ڽ��� �������� ��û �����ߴ� �ð��� ���� ȣ�� �պ��ð� ���
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	//���� �����ð��� ���( ���޹��� ���� �����ð��� ������û�� Ŭ���̾�Ʈ���� �����ϱ���� �ɸ� �ð��� ���ϸ� �װ��� �� ������ ���� ���� �ð�
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime- GetWorld()->GetTimeSeconds();
}
float AProjectZPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}
