// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"

#include "Net/UnrealNetwork.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStatedChanged(WaveState,OldState);
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;
		// Call on server
		OnRep_WaveState(OldState);
	}
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//属性同步条件
	DOREPLIFETIME(ASGameState,WaveState);
}