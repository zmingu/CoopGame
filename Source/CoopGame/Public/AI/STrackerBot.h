// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//静态网格体组件
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Components")
	class UStaticMeshComponent* StaticMeshComp;

	//声明寻路径点函数
	FVector GetNextPathPoint();

	//添加力的大小
	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	float MovementForce;
	//是否使用改变速度
	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	bool bUseVelocityChange;
	//距离目标多少时判定为到达
	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	float RequiredDistanceToTarget;
	//下一个点的变量
	UPROPERTY()
	FVector NextPathPoint;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Conmponents")
	class USHealthComponent* HealthComp;

	//受伤回调函数
	UFUNCTION()
	void HandleTakeDamage(class USHealthComponent* OwningHealthComp,float Health,float HealthDelta,const class UDamageType* DamageType, class AController*InstigatedBy, AActor* DamageCauser);

	//声明材质实例
	UMaterialInstanceDynamic* MatInstance;

	void SelfDestruct();//自毁函数，需要生成爆炸特效，造成范围伤害。

	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	class UParticleSystem* ExplosionEffect;//爆炸特效

	bool bExploded;//是否爆炸

	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	float ExplosionRadius;//爆炸范围

	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	float ExplosionDamage;//爆炸伤害

	//是否开始伤害自己
	bool bStartSelfDestruction;
	//时间句柄
	FTimerHandle TimerHandle_SelfDamage;
	//伤害自己的函数
	void DamageSelf();
	//球形组件,用来判断和玩家重叠
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USphereComponent* SphereComp;
	//重叠事件
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	//倒计时自爆音效
	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	class USoundBase *SelfDestructSound;
	//爆炸音效
	UPROPERTY(EditDefaultsOnly,Category="TracerBot")
	class USoundBase* ExplodeSound;

	//检测附近同类的函数
	UFUNCTION()
	void OnCheckNearbyBots();
	//伤害等级
	int32 PowerLevel;
};
