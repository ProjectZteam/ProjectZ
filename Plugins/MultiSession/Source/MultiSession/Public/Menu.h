// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTISESSION_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//레벨 블루프린트에서 호출
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections=4, FString TypeOfMatch=FString(TEXT("Free")),FString LobbyPath=FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));
protected:
	virtual bool Initialize() override;
	//세션 생성 성공시 자동실행
	virtual void NativeDestruct() override;

	//멀티플레이 서브시스템 커스텀 델레게이트를 위한 콜백함수
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:
	//블루프린트 위젯 버튼 바인딩 (meta로 지정할경우 블루프린트이름과 반드시 같아야함
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* SingleButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;
	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();
	UFUNCTION()
	void SingleButtonClicked();
	UFUNCTION()
	void StartButtonClicked();
	void MenuTearDown();
	//온라인 세션기능 처리 하게 설계된 시스템
	class UMultiplayerSessionsSubsystem* MultiplayerSessionSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{TEXT("Free")};
	FString PathToLobby{ TEXT("") };
};
