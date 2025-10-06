// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerUpActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerUpActor : public AActor
{
	GENERATED_BODY()

public:
	ASPowerUpActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	//道具起作用的间隔，类似每隔多少秒加多少血
	UPROPERTY(EditDefaultsOnly, Category="Powerups")
	float PowerUpInterval;
	//道具起作用的总次数
	UPROPERTY(EditDefaultsOnly, Category="Powerups")
	int32 TotalNrOfTicks;

	FTimerHandle TimerHandle_PowerUpTicks;
	//道具已经起作用的次数
	int32 TickProcessed;
	//道具起作用函数
	UFUNCTION()
	void OnTickPowerUp();
	//激活道具
	// void ActivatePowerUp();
	//指定道具作用目标ActiveFor
	void ActivatePowerUp(AActor* ActiveFor);

	// UFUNCTION(BlueprintImplementableEvent,Category="Powerups")
	// void OnActivated();

	UFUNCTION(BlueprintImplementableEvent,Category="Powerups")
	void OnActivated(AActor* ActiveFor);


	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnPowerUpTicked();

	UFUNCTION(BlueprintImplementableEvent, Category="Powerups")
	void OnExpired();

	//同步激活状态
	UPROPERTY(ReplicatedUsing=OnRep_PowerActive)
	bool bIsPowerActive;

	UFUNCTION()
	void OnRep_PowerActive();

	//蓝图可实现事件，去蓝图实现
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerUpStateChanged(bool bNewIsActive);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
