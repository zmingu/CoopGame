// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickUpActor.h"

#include "SCharacter.h"
#include "SPowerUpActor.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
ASPickUpActor::ASPickUpActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//CPP的构造函数ASPickUpActor()中初始化
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90,0,0));
	DecalComp->DecalSize=FVector(64,75,75);
	DecalComp->SetupAttachment(RootComponent);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPickUpActor::BeginPlay()
{
	Super::BeginPlay();
	//只在服务端生成道具
	if (GetLocalRole() == ROLE_Authority)
	{
		//刚开始时生成道具
		ResPawn();
	}
}

void ASPickUpActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	//如果生成的道具实例存在，则让道具起作用，然后将道具实例置空，并设置定时器生成下一个道具
	//同时需要判断碰到的是不是角色，并且只在服务端激活道具
	ASCharacter* Character = Cast<ASCharacter>(OtherActor);
	if (Character && PowerUpInstance && GetLocalRole()==ROLE_Authority)
	{
		PowerUpInstance->ActivatePowerUp(OtherActor);
		PowerUpInstance = nullptr;
		//设置生成道具的定时器，每隔CooldownDuration时间调用一次ResPawn()生成道具的函数。
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickUpActor::ResPawn, CooldownDuration);
	}
	
}

void ASPickUpActor::ResPawn()
{
	if (PowerUpClass == nullptr)//如果没有实际起作用的道具类则打印错误日志并返回
	{
		UE_LOG(LogTemp,Warning,TEXT("PowerUpClass is null in %s"),*GetName());
		return;
	}
	FActorSpawnParameters SpawnParameters;//生成Actor的生成参数
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//设置参数设置Actor总是生成
	PowerUpInstance = GetWorld()->SpawnActor<ASPowerUpActor>(PowerUpClass,GetTransform(),SpawnParameters);//生成道具实例Actor<生成类型>(类，位置，生成参数)
}

