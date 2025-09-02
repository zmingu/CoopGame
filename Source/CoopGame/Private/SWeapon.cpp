// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SKMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMeshComp"));
	RootComponent = SKMeshComp;
	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName= "Target";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
		//查询参数启用复合追踪
		QueryParams.bTraceComplex = true;
		//判断是否弹道射线打到东西，单射线查询通道（打击结果，射线开始位置，射线结束位置，碰撞通道，查询参数）
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,EyeLocation,TraceEnd,ECC_Visibility,QueryParams);
		//如果射线打到东西
		//如果射线打到东西
		if (bHit)
		{
			//应用点状伤害(被伤害的Actor，要应用的基础伤害，受击方向，描述命中的碰撞或跟踪结果，照成伤害的控制器（例如射击武器的玩家的控制器）,实际造成伤害的Actor,伤害类型)
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(),20,EyeRotator.Vector(),Hit,WeaponOwner->GetInstigatorController(),this,DamageType);
		}
		//把射线检测绘制出来
		DrawDebugLine(GetWorld(),EyeLocation,TraceEnd,FColor::Red,false,1,0,1);

		if (MuzzleEffect)
		{
			//生成特效并附加到组件上的某个骨骼上,参数为(粒子特效，要依附的组件，生成的命名点)
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect,SKMeshComp,MuzzleSocketName);
		}
		if (ImpactEffect)
		{
			//在生成特效在某个位置,参数为（生成的世界，粒子特效，世界位置，世界旋转）——SpawnEmitterAtLocation有三个重载。
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactEffect,Hit.ImpactPoint,Hit.ImpactPoint.Rotation());
		}

		//获得枪口插槽位置。
		FVector MuzzleSocketLocation = SKMeshComp->GetSocketLocation(MuzzleSocketName);
		//枪口生成特效并拿到特效组件实例。
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),TracerEffect,MuzzleSocketLocation);
		if (TracerComp)
		{
			//在此粒子组件上设置命名矢量实例参数。
			TracerComp->SetVectorParameter(TracerTargetName,TraceEnd);
		}
	}
}

