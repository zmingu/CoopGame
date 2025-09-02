// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SKMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMeshComp"));
	RootComponent = SKMeshComp;
	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName= "Target";
	//基础伤害值。
	BaseDamage = 20;

	//每分钟开600枪
	RateOfFire = 600;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	//两枪之间最小时间 0.1s
	TimeBetweenShots = 60 / RateOfFire;
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::Fire()
{
	AActor* WeaponOwner= GetOwner();
	if (WeaponOwner)
	{
		/*
		 * 弹道的起点和终点
		 */
		FVector EyeLocation;
		FRotator EyeRotator;
		//弹道起点 = 玩家摄像头位置
		Cast<APawn>(WeaponOwner)->GetActorEyesViewPoint(EyeLocation,EyeRotator);
		//弹道终点 = 起点位置 + （方向 * 距离）
		FVector TraceEnd = EyeLocation + (EyeRotator.Vector() * 1000);

		/*
		 *打击和碰撞
		 */
		//打击结果（里面能存储射线打击到的一些信息）
		FHitResult Hit;
		//查询参数
		FCollisionQueryParams QueryParams;
		//查询参数设置忽略武器自身和拥有武器的玩家
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.bTraceComplex = true;	//查询参数启用复合追踪
		QueryParams.bReturnPhysicalMaterial = true; //是否返回击中的物理材质
		//判断是否弹道射线打到东西，单射线查询通道（打击结果，射线开始位置，射线结束位置，碰撞通道，查询参数）
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
		//如果射线打到东西
		if (bHit)
		{
			float ActualDamage = BaseDamage;
			//在应用点状伤害前判断打到的物理表面是不是头部，是则伤害翻4倍。
			if (UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()) == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			//应用点状伤害(被伤害的Actor，要应用的基础伤害，受击方向，描述命中的碰撞或跟踪结果，照成伤害的控制器（例如射击武器的玩家的控制器）,实际造成伤害的Actor,伤害类型)
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(),ActualDamage,EyeRotator.Vector(),Hit,WeaponOwner->GetInstigatorController(),this,DamageType);
			//播放受击特效
			PlayImpactEffects(Hit);
		}
		//播放枪口和弹道特效
		PlayFireEffects(TraceEnd);
	}
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	//开火枪口特效。
	if (MuzzleEffect)
	{
		//播放附加到指定组件并跟随指定组件的指定效果(粒子特效，要依附的组件，生成的命名点)。
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SKMeshComp, MuzzleSocketName);
	}

	//弹道特效。这里没有搞懂
	if (TracerEffect)
	{
		//获得枪口插槽位置。
		FVector MuzzleSocketLocation = SKMeshComp->GetSocketLocation(MuzzleSocketName);
		//枪口生成特效并拿到特效实例。
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), TracerEffect, MuzzleSocketLocation);
		if (TracerComp)
		{
			//在此粒子组件上设置命名矢量实例参数。
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	/*相机抖动*/
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
		//该方法过时
		// if (PlayerController) PlayerController->ClientPlayCameraShake(FireCamShake);..................
		//拿到玩家控制器播放相机抖动
		if (PlayerController) PlayerController->ClientStartCameraShake(FireCamShake);
	}
}

void ASWeapon::PlayImpactEffects(FHitResult Hit)
{
	//获取击中的物体的物理表面材质类型
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
	//根据表面材质选择后最终要使用的例子特效
	UParticleSystem* SelectedEffect = nullptr;
	//窗口打印表面类型
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("HitSurfaceType:%d"), (int32)HitSurfaceType));
	//根据物理表面设置要使用的特效
	switch (HitSurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}
	if (SelectedEffect)
	{
		//枪口位置
		FVector MuzzleLocation = SKMeshComp->GetSocketLocation(MuzzleSocketName);
		//受击方向
		FVector ShotDirection = Hit.ImpactPoint - MuzzleLocation;
		//归一化矢量
		ShotDirection.Normalize();
		//位置处生成击中特效
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(0.0f, LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true,FirstDelay);
}

void ASWeapon::StopFire()
{
	//清除连续开火的时间句柄
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

