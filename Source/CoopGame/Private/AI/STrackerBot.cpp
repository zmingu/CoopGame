// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STrackerBot.h"

#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "SCharacter.h"
#include "Component/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetCanEverAffectNavigation(false);//该组件不会影响导航网格
	StaticMeshComp->SetSimulatePhysics(true);
	RootComponent = StaticMeshComp;
	
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnCompHealthChanged.AddDynamic(this,&ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);//设置半径
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//设置碰撞类型为只查询
	SphereComp->SetCollisionResponseToChannels(ECR_Ignore);//设置所有碰撞通道为忽略
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);//只开启和Pawn类型的重叠
	SphereComp->SetupAttachment(RootComponent);
	
	bUseVelocityChange = true;//启用力改变速度
	MovementForce = 1000;//作用力大小
	RequiredDistanceToTarget = 100;//判定到达目标的距离
	
	ExplosionRadius = 350;//爆炸范围
	ExplosionDamage = 60;//爆炸伤害
	
	
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	//只在服务端寻路
	if (GetLocalRole() == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint(); //获取到下一个初始点
		
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);//设置每秒调用检测附近AI球同类的OnCheckNearbyBots函数
	}
	
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//只在服务端执行寻路逻辑，且AI球没爆炸
	if (GetLocalRole() == ROLE_Authority && !bExploded )
	{
		//获得两点向量差的大小，既是距离下一个点的距离
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		//距离下一个点的距离小于阈值100(判定到达目标)则获取下一个点，如果还没到则推进。
		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, 0.0f, 0, 1.0f);
		}
		else
		{
			//获得从AI球指向下一个点的向量。
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			//获取方向，不受大小的影响。
			ForceDirection.Normalize();
			//方向向量 * 力 = 有方向的力，用来推动小球。
			ForceDirection *= MovementForce;
			//添加推力使小球朝目标滚动。同时改变小球的速度。
			StaticMeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			//画出AI球的运动方向
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Red,
									  false, 0.0f,
									  0, 1.0f);
		}
		//画出下一个目标点位置
		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}
	
	

}

// Called to bind functionality to input
void ASTrackerBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector ASTrackerBot::GetNextPathPoint()
{
	//拿到0号玩家对象
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	//立即找到下一个路径（上下文，路径开始点，目标Actor）
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this,GetActorLocation(),PlayerPawn);
	//如果路径点数量大于1返回下一个位置点
	if (NavPath->PathPoints.Num()>1)
	{
		//返回路径点数组中的第1个位置点
		return NavPath->PathPoints[1];
	}

	//否则返回初始位置
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(class USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInstance ==nullptr)
	{
		//拿到静态网格体的材质，并创建动态材质实例
		MatInstance = StaticMeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0,StaticMeshComp->GetMaterial(0));
	}
	if (MatInstance)
	{
		//受伤时，通过改变材质中的参数LastTimeDamageTaken，这时这个参数几乎等于输入Timer，所以会亮一下。
		MatInstance->SetScalarParameterValue("LastTimeDamageTaken",GetWorld()->TimeSeconds);
	}

	if (Health <= 0)
	{
		SelfDestruct();
	}
	
	UE_LOG(LogTemp,Log,TEXT("Health %s of %s"),*FString::SanitizeFloat(Health),*GetName());
}


void ASTrackerBot::SelfDestruct()
{
	//如果爆炸过直接返回
	if (bExploded) return;
	bExploded = true;
	//播放爆炸特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::SpawnSoundAtLocation(this,ExplodeSound,GetActorLocation());//播放爆炸音效

	StaticMeshComp->SetVisibility(false, true);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//AI球造成范围伤害也在服务端上做
	if (GetLocalRole() == ROLE_Authority)
	{
		//添加忽略的Actor，自身
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		//造成范围伤害ApplyRadialDamage(上下文，原点的基础伤害，原点位置，伤害半径，伤害类型类，要忽略的Actor列表，造成伤害的人，负责造成伤害的控制器，伤害是否根据原点缩放)
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr,
											IgnoredActors, this, GetInstigatorController(), true);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 11, FColor::Green, false, 2.0f, 0, 1.0f);
		
		
		Destroy();
	}
	
	
}

void ASTrackerBot::DamageSelf()
{
	//对自己造成20点伤害,ApplyDamage(被伤害的Actor，基础伤害，造成伤害的控制器，造成伤害的Actor，描述造成伤害的类)
	UGameplayStatics::ApplyDamage(this,20,GetInstigatorController(),this,nullptr);
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	//日志打印debug
	UE_LOG(LogTemp,Log,TEXT("Overlap %s"),*OtherActor->GetName());
	
	if (!bStartSelfDestruction)
	{
		ASCharacter* MyCharacter = Cast<ASCharacter>(OtherActor);
		//如果碰到的是玩家，就用定时器每0.5秒伤害自己一次，一次20滴血
		if (MyCharacter)
		{
			
			if (GetLocalRole() == ROLE_Authority)
			{
				//SetTimer(时间句柄，调用者，调用的方法，调用间隔，是否循环，离第一次调用的延迟)
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, 0.5f, true, 0.0f);
			}
			
			//设置为开始自爆
			bStartSelfDestruction = true;
			//播放倒计时自毁音效
			UGameplayStatics::SpawnSoundAtLocation(this, SelfDestructSound, GetActorLocation());
		}
	}
}

void ASTrackerBot::OnCheckNearbyBots()
{
	//声明碰撞球，用于检测球体内有多少AI同类。
	FCollisionShape CollisionShape;
	//设置碰撞球半径。
	CollisionShape.SetSphere(600);
	//重叠结果的数组，用来存放所有重叠到的东西
	TArray<FOverlapResult> OverlapResults;
	//碰撞对象查询参数
	FCollisionObjectQueryParams QueryParams;
	//添加需要查询的两种碰撞通道类型
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	//按对象类型的重叠检测，将检测到的东西放进OverlapResults数组中
	GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, QueryParams,
										 CollisionShape);
	//画出用于检测的碰撞球
	DrawDebugSphere(GetWorld(), GetActorLocation(), CollisionShape.GetSphereRadius(), 12, FColor::Blue, false, 1.0f);
	//声明其他同类AI球的数量
	int32 NrOfBots = 0;
	//遍历所有重叠检测到的东西，如果是同类就把同类数量+1
	for (FOverlapResult Result : OverlapResults)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		//重叠检测到的是AI球，且不是自己，就计数
		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}
	//定义常量最大伤害等级
	const int32 MaxPowerLevel = 4;
	//限制伤害等级范围0-4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);
	if (MatInstance == nullptr)
	{
		//拿到AI球的材质实例
		MatInstance = StaticMeshComp->
			CreateAndSetMaterialInstanceDynamicFromMaterial(0, StaticMeshComp->GetMaterial(0));
	}
	if (MatInstance)
	{
		//通过设置参数来改变材质闪烁，更详细的材质介绍请看视频
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		MatInstance->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
	//打印伤害等级
	DrawDebugString(GetWorld(), FVector(), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
}
