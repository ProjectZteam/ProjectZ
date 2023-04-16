// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
//level blueprint call it
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch,FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	//�������� �Է¸�� ���콺�� ����
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		//������ ���� ����ý��ۿ� �����ϴ� ���� ����ý��� get
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	//�޴� �����ϸ鼭 Ŀ���� ��������Ʈ�� �޴� Ŭ���� �ݹ��Լ� ���
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this,&ThisClass::OnFindSessions);
		MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	//��ư�� �Լ� ����
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (SingleButton)
	{
		SingleButton->OnClicked.AddDynamic(this, &ThisClass::SingleButtonClicked);
	}
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::StartButtonClicked);
	}
	return true;
}

//ServerTravel�Լ� ������ �ڵ� ȣ��
void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

//��Ƽ�÷��� �������̽��� ���ǻ����õ���� ��������Ʈ�� ���� ����
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session Created Successfuly"))
			);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Session Created Failed"))
			);
		}
		HostButton->SetVisibility(ESlateVisibility::Visible);
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionSubsystem == nullptr)
	{
		return;
	}
	//�˻� ������� ���ϴ� ��ĪŸ�� ���� ������ �ش� ���� ����
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"),SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetVisibility(ESlateVisibility::Visible);
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{//����ý��� get ������ ���� �������̽� get
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetVisibility(ESlateVisibility::Visible);
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
}

void UMenu::HostButtonClicked()
{
	if (MultiplayerSessionSubsystem)
	{
		HostButton->SetIsEnabled(false);
		//�޴��������� host Ŭ���� ���� ����, ���ڴ� ���� �������Ʈ������ �Ѿ�� 
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnections,MatchType);
		HostButton->SetVisibility(ESlateVisibility::Hidden);
		JoinButton->SetVisibility(ESlateVisibility::Hidden);
		SingleButton->SetVisibility(ESlateVisibility::Hidden);
		StartButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->FindSession(10000);
	}
}

void UMenu::SingleButtonClicked()
{
}

void UMenu::StartButtonClicked()
{
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->StartSession();
	}
}

//servertravel�Լ� ������ �ڵ� ȣ��( �� �̵� ������ ���� �����ϰ� �Է¸�� �������� ��ȯ
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
