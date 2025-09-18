// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
	
	UPROPERTY()
	FHitResult Hit;
	
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};


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

	//击中特效——击中东西的默认特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;
	
	//击中特效——击中人的飙血特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;
	
	//弹道特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	//弹道特效参数名
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	

protected:
	//镜头震动类
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	TSubclassOf<class UCameraShakeBase> FireCamShake;

	//声明两个特效播放函数。
	void PlayFireEffects(FVector TraceEnd);//枪口和弹道特效
	void PlayImpactEffects(FHitResult Hit);//受击特效

	//基础伤害值
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float BaseDamage;

	/*连发开火*/
	//连发间隔时间句柄
	FTimerHandle TimerHandle_TimeBetweenShots;
	//上次开火时间
	float LastFireTime;
	//开火频率——每分钟多少枪
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float RateOfFire;
	//两枪之前的时间——分钟
	float TimeBetweenShots;

public:
	//开始和停止开火函数
	void StartFire();
	void StopFire();

	//服务器开火函数,三个宏分别表示这是一个服务器函数，可靠的RPC，有验证功能
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire();

	//声明结构体变量和绑定调用时复制的方法，作用：当服务器中的结构体中的变量改变时，就让客户端中的角色调用OnRep_HitScanTrace()函数
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	//重写同步函数
	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;
};
