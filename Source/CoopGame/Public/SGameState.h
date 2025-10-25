// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState:uint8
{
	//等待开始
	WaitingToStart,
	//开始生成AI
	WaveInProgress,
	//AI生成完毕，等待通关
	WaitingToComplete,
	//通关
	WaveComplete,
	//游戏失败
	GameOver,
};

UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	//网络同步游戏状态,当前状态改变时会调用这个函数。
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);
	
	

	//状态改变
	UFUNCTION(BlueprintImplementableEvent, Category=GameState)
	void WaveStatedChanged(EWaveState NewState, EWaveState OldState);

	//当前状态
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WaveState, Category=GameState)
	EWaveState WaveState;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetWaveState(EWaveState NewState);//用来设置新的游戏状态
	
};
