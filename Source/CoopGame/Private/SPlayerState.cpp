// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"

void ASPlayerState::AddScore(float ScoreDelta)
{
	float NewScore = GetScore();
	NewScore += ScoreDelta;
	SetScore(NewScore);
}