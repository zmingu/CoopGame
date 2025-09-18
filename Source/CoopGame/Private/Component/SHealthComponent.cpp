// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SHealthComponent.h"

#include "Net/UnrealNetwork.h"


USHealthComponent::USHealthComponent()
{
	//初始化默认最大生命值
	DefaultHealth =100;

	//开启组件的网络复制功能
	SetIsReplicated(true);
}


void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() == ROLE_Authority)
	{
		//获取组件的拥有者
		AActor* MyOwner = GetOwner();
		//当Actor收到伤害时会自动调用OnTakeAnyDamage函数绑定的HandleTakeAnyDamage函数
		if (MyOwner) MyOwner->OnTakeAnyDamage.AddDynamic(this,&USHealthComponent::HandleTakeAnyDamage);
	}
	

	//当前生命值等于默认最大生命值100
	Health = DefaultHealth;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0)return;
	//更新生命值为受伤后的生命值
	//限定受伤后的生命值在0-默认最大生命值之间，包括这两个数
	Health = FMath::Clamp(Health-Damage,0.0f,DefaultHealth);
	//打印受伤后的生命值
	UE_LOG(LogTemp,Log,TEXT("被打的人说：我还剩:%s点血哦"),*FString::SanitizeFloat(Health));

	//执行多播调用,作用是让蓝图里绑定了这个事件的节点都能被调用到 
	OnHealthChanged.Broadcast(this,Health,Damage,DamageType,InstigatedBy,DamageCauser);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent,Health);//Health属性同步条件
}


