// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	
	//武器的骨骼网格体
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Weapon")
	class USkeletalMeshComponent* SKMeshComp;
	
	
	//开火函数
	UFUNCTION(BlueprintCallable,Category="Weapon")
	virtual void Fire();
	
	//伤害类型
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	TSubclassOf<UDamageType> DamageType;
	

	//枪骨骼上的枪口骨骼名
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	FName MuzzleSocketName;

	//枪口特效
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	class UParticleSystem* MuzzleEffect;

	//击中特效
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	class UParticleSystem* ImpactEffect;

	

	//弹道特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	//弹道特效参数名
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

};
