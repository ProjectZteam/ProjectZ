// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
//level blueprint call it
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch,FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	//위젯띄우고 입력모드 마우스로 변경
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
		//엔진의 세션 서브시스템에 의존하는 생성 서브시스템 get
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	//메뉴 생성하면서 커스텀 델레게이트에 메뉴 클래스 콜백함수 등록
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
	//버튼에 함수 연결
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

//ServerTravel함수 종료후 자동 호출
void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

//멀티플레이 인터페이스가 세션생성시도결과 델레게이트를 통해 전달
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
	//검색 결과에서 원하는 매칭타입 최초 만날시 해당 세션 입장
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
	{//서브시스템 get 성공시 세션 인터페이스 get
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
		//메뉴위젯에서 host 클릭시 세션 생성, 인자는 레벨 블루프린트값에서 넘어옴 
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
	UE_LOG(LogTemp, Log, TEXT("SingleButtonClicked"));
	UGameplayStatics::OpenLevel(this, "SingleMap");
}

void UMenu::StartButtonClicked()
{
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->StartSession();
	}
}

//servertravel함수 성공시 자동 호출( 맵 이동 성공시 위젯 제거하고 입력모드 게임으로 전환
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
