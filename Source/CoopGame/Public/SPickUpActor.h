// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickUpActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickUpActor : public AActor
{
	GENERATED_BODY()

public:
	ASPickUpActor();

protected:
	virtual void BeginPlay() override;
	
	//球形组件
	UPROPERTY(VisibleAnywhere,Category="Components")
	USphereComponent *SphereComp;
	
	//贴花组件
	UPROPERTY(VisibleAnywhere,Category="Components")
	UDecalComponent *DecalComp;
	
	UPROPERTY(EditDefaultsOnly, Category="PickUpActor")
	TSubclassOf<class ASPowerUpActor> PowerUpClass;//实际起作用的道具类

	ASPowerUpActor* PowerUpInstance;//实际起作用的道具实例

	UPROPERTY(EditInstanceOnly, Category="PickUpActor")
	float CooldownDuration;//道具生成冷却时间

	FTimerHandle TimerHandle_RespawnTimer;//生成道具时间句柄

	void ResPawn();//生成道具函数

public:
	//重载重叠函数
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};