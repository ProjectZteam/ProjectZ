// Fill out your copyright notice in the Description page of Project Settings.


#include "SingleGameMode.h"

void ASingleGameMode::DecreaseMonsterCount(const int Count)
{
	LeftMonsterCount -= Count;
	if(LeftMonsterCount <= 0)
	{
		StageClear();
	}
}

void ASingleGameMode::IncreaseMonsterCount(int Count)
{
	LeftMonsterCount += Count;
}

int ASingleGameMode::GetLeftMonsterCount()
{
	return LeftMonsterCount;
}

void ASingleGameMode::StageClear()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DEBUG : Stage Clear"));
}
