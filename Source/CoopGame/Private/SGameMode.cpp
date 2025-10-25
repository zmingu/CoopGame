// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"

#include "SGameState.h"
#include "SPlayerState.h"
#include "Component/SHealthComponent.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2;//关卡之间的间隔时间
	PrimaryActorTick.bCanEverTick = true;//修改Tick机制
	PrimaryActorTick.TickInterval = 1;

	GameStateClass = ASGameState::StaticClass();//GameMode指定GameState类为我们自定义的ASGameState

	//更换GM的玩家状态
	PlayerStateClass = ASPlayerState::StaticClass();
	
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();
	
	PrepareForNextWave();//准备下一个关卡
}

void ASGameMode::StartWave()
{
	WaveCount++;//关卡数+1
	
	NrOfBotsToSpawn = 2 * WaveCount;//生成AI的数量=2*关卡数
	
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner,this,&ASGameMode::SpawnBotTimerElapsed,1,true,0);//用定时器每秒生成一个AI
	
	SetWaveState(EWaveState::WaveInProgress);//进入生成AI状态
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	//PrepareForNextWave();不需要在这准备关卡，而是等到判断所有AI死了再准备

	
	SetWaveState(EWaveState::WaitingToComplete);//进入生成AI完毕等待游戏完成状态，下一步要么AI死光，要么角色死光。在Tike中检查。
}

void ASGameMode::CheckWaveState()
{
	
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);//是否在准备下一关卡，如果是，说明敌人已经消灭光了
	
	if (bIsPreparingForWave||NrOfBotsToSpawn>0) return;//如果是在准备下一个关卡，或者还在生成AI，就不需要检测关卡状态了
	
	bool bIsAnyBotAlive = false;//创建布尔变量标记是否有存活的AI

	
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator();It;++It) //用Pawn迭代器获取所有Pawn，当然也包括玩家Character
	{
		APawn* Pawn = It->Get();
		
		if (Pawn == nullptr || Pawn->IsPlayerControlled()) continue; //判断Pawn是否空，和此Pawn是否被玩家控制，被玩家控制的是玩家角色。是要排除的

		
		USHealthComponent* HealthComp = Cast<USHealthComponent>(Pawn->GetComponentByClass(USHealthComponent::StaticClass())); //尝试拿到AIPawn里的生命值组件，判断生命值是否大于0，大于则活着，只要一个活着就为true
		if (HealthComp && HealthComp->GetHealth() > 0)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}
	
	if (!bIsAnyBotAlive)//如果没有活着的了就准备下一个关卡
	{
		PrepareForNextWave();
		
		SetWaveState(EWaveState::WaveComplete);//进入关卡完毕状态，
	}
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();//检查关卡状态
	CheckAnyPlayerAlive();//检查玩家存活
}

void ASGameMode::CheckAnyPlayerAlive()
{
	//如果玩家存活，这个for是一直被Tick调用，一直检测
	//使用玩家控制器迭代器获取所有玩家控制器，再又控制器获得玩家Pawn，判断Pawn中的健康组件中的生命值判断玩家是否存活
	for (FConstPlayerControllerIterator It_PC = GetWorld()->GetPlayerControllerIterator(); It_PC; ++It_PC)
	{
		APlayerController* PC = It_PC->Get();
		APawn* Pawn = PC->GetPawn();
		if (PC && Pawn)
		{
			USHealthComponent* HealthComp = Cast<USHealthComponent>(
				Pawn->GetComponentByClass(USHealthComponent::StaticClass()));
			//ensure判空提供更多信息
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0)return;
		}
	}
	GameOver();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();//调用蓝图实现的生成AI函数
	
	NrOfBotsToSpawn--;//待生成的AI数量-1
	
	
	if (NrOfBotsToSpawn <= 0)//如果待生成的AI数量小于等于0就结束关卡
	{
		EndWave();
	}
}

void ASGameMode::PrepareForNextWave()
{
	//FTimerHandle TimerHandle_NextWaveStart;
GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
	
	SetWaveState(EWaveState::WaitingToStart);//进入等待开始状态

}

void ASGameMode::GameOver()
{
	EndWave();//会结束一个TimerHandle_BotSpawner
	UE_LOG(LogTemp,Log,TEXT("游戏结束，玩家死了"));
	
	SetWaveState(EWaveState::GameOver);//进入游戏结束状态
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RestartPlayers()
{
	for (FConstPlayerControllerIterator It_PC = GetWorld()->GetPlayerControllerIterator(); It_PC; ++It_PC)
	{
		APlayerController* PC = It_PC->Get();
		//这里要角色死后调用了Destroy()函数才能进去。
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}


