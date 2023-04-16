// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
 * 
 */

//Menu Ŭ������ �����ϱ� ���� Ŀ���� ��������Ʈ dynamic�̱⶧���� ��� �ݹ��Լ��� �ݵ�� UFUNCTION������ �ʿ�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
//���ڿ� �������Ʈ���� �Ұ��� ���ڰ� �־� ���̳��� ��������Ʈ �Ұ���
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

	//�� ����ý��� ���� ���� menu Ŭ������ ���Լ��� �ҷ� �� �� �ְ� ��

	void CreateSession(int32 NumPublicConnections, FString MatchType);// �÷����ο�, ����Ÿ�� ����
	void FindSession(int32 MaxSearchResults);//���ϴ� ��ġŸ�� ���Ǽ�
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);// �����Ϸ��� Session
	void DestroySession();
	void StartSession();

	//�޴� Ŭ������ �ݹ��Լ� ���ε��ϱ����� Ŀ���� ��������Ʈ
	
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	//-----------------------------------
protected:
	//Session Interface delegate list
	//�ݹ��Լ� (��������Ʈ�� �� �ܰ� ����� �ڵ����� ȣ��)
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	//���� �������̽� ���� ����
	IOnlineSessionPtr SessionInterface;
	//���ǻ����� ����� �������� ������
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	//�˻����� �������� ������
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	//online session interface delegates
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//delegates handle ����Ʈ�� �����ϴ� ����
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
