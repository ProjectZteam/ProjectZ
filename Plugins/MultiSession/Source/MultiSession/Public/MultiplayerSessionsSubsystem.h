// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
 * 
 */

//Menu 클래스에 전파하기 위한 커스텀 델레게이트 dynamic이기때문에 등록 콜백함수는 반드시 UFUNCTION컨벤션 필요
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
//인자에 블루프린트연동 불가한 인자가 있어 다이나믹 델레게이트 불가능
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
UCLASS()
class MULTISESSION_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionsSubsystem();

	//이 서브시스템 관리 위해 menu 클래스가 이함수를 불러 쓸 수 있게 함

	void CreateSession(int32 NumPublicConnections, FString MatchType);// 플레이인원, 게임타입 인자
	void FindSession(int32 MaxSearchResults);//원하는 매치타입 세션수
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);// 입장하려는 Session
	void DestroySession();
	void StartSession();

	//메뉴 클래스가 콜백함수 바인딩하기위한 커스텀 델레게이트
	
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	//-----------------------------------
protected:
	//Session Interface delegate list
	//콜백함수 (델레게이트가 각 단계 종료시 자동으로 호출)
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	//세션 인터페이스 담을 변수
	IOnlineSessionPtr SessionInterface;
	//세션생성시 사용할 설정정보 포인터
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	//검색세션 저장위한 포인터
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	//online session interface delegates
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//delegates handle 리스트를 저장하는 변수
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
