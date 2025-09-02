// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"



void ASProjectileWeapon::Fire()
{
	Super::Fire();
	AActor* WeaponOwner = GetOwner();
	if (WeaponOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotator;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotator);

		//获取枪口位置
		FVector MuzzleLocation = SKMeshComp->GetSocketLocation(MuzzleSocketName);

		//生成参数
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ProjectileClass)
		{
			GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotator, SpawnParams);
		}
	}
}

