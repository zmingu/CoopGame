// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PowerUpInterval = 0.f;//道具作用间隔
	TotalNrOfTicks = 0;//道具作用次数
	
	SetReplicates(true);//设置网络复制
	bIsPowerActive = false;//设置道具初始状态为未激活
}

// Called when the game starts or when spawned
void ASPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASPowerUpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASPowerUpActor::ActivatePowerUp(AActor* ActiveFor)
{
	
	OnActivated(ActiveFor);//激活道具
	bIsPowerActive = true;//激活后设置激活状态为否，调用同步函数。
	OnRep_PowerActive();
	
	if (PowerUpInterval > 0)//如果道具是有间隔的道具，则需要使用定时器，比如：10秒内加100滴血的道具
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTicks, this, &ASPowerUpActor::OnTickPowerUp, PowerUpInterval,true, 0.f);//设置定时器每PowerUpInterval时间间隔调用一次OnTickPowerUp()（时间句柄变量，调用对象，调用函数，调用间隔，是否循环，延迟）
	}
	else
	{
		OnTickPowerUp();//如果是不需要作用时间的道具则让道具直接起作用，比如加速道具只加一次速
	}
}

void ASPowerUpActor::OnRep_PowerActive()
{
	OnPowerUpStateChanged(bIsPowerActive);
}


void ASPowerUpActor::OnTickPowerUp()
{
	TickProcessed++;//道具已经起作用的次数：每起作用一次就自增一次
	OnPowerUpTicked();
	if (TickProcessed >= TotalNrOfTicks)//如果道具作用次数达到总次数，就使道具失效，同时清除定时器
	{
		OnExpired();//使道具失效函数
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTicks);//清除定时器
		//道具失效后设置激活状态为否，并手动调用一次同步函数。
		bIsPowerActive = false;
		OnRep_PowerActive();
	}
}
void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASPowerUpActor,bIsPowerActive);
}


