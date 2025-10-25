// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

//添加三参数的委托（死者，造成伤害者，控制器）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled,AActor*,VictimActor,AActor*,KillerActor,AController*,KillerController);
enum class EWaveState : uint8;
/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASGameMode();//构造函数
	
	virtual void StartPlay() override;//重写GameModeBase的游戏开始函数
	
	void PrepareForNextWave();//准备下一个关卡
	
protected:
	FTimerHandle TimerHandle_BotSpawner;//生成AI数量
	
	int32 NrOfBotsToSpawn;

	int32 WaveCount;	//关卡数
	
	UPROPERTY(EditDefaultsOnly, Category=GameMode)//关卡之间的间隔时间
	float TimeBetweenWaves;
	
	UFUNCTION(BlueprintImplementableEvent, Category=GameMode)//生成新的机器人函数，蓝图实现
	void SpawnNewBot();
	
	void SpawnBotTimerElapsed();//生成机器人时间结束

	void StartWave();//开始关卡

	void EndWave();	//结束关卡

protected:
	FTimerHandle TimerHandle_NextWaveStart;//管理下一关开启时间
	void CheckWaveState();
public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	void CheckAnyPlayerAlive(); //检查玩家存活
	
	void GameOver();//游戏结束

public:
	//用来设置新的游戏状态
	void SetWaveState(EWaveState NewState);

public:
	UPROPERTY(BlueprintAssignable,Category="GameMode")
	FOnActorKilled OnActorKilled;

	

public:
	void RestartPlayers();
};
