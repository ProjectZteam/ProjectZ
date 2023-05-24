// Fill out your copyright notice in the Description page of Project Settings.


#include "SingleGameMode.h"

void ASingleGameMode::DecreaseMonsterCount(const int Count)
{
	MonsterCount -= Count;
	if(MonsterCount <= 0)
	{
		StageClear();
	}
}

void ASingleGameMode::StageClear()
{

}

void ASingleGameMode::BeginPlay()
{
	
}
