// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SHealthComponent.h"

#include "SGameMode.h"
#include "Net/UnrealNetwork.h"


USHealthComponent::USHealthComponent()
{
	//初始化默认最大生命值
	DefaultHealth =100;

	//开启组件的网络复制功能
	SetIsReplicated(true);

	bIsDead = false;//默认未死亡
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

void USHealthComponent::OnRep_Health(float OldHealth)
{
	//执行多播，让绑定了这个事件的地方进行回调
	float Damage = Health - OldHealth;
	OnCompHealthChanged.Broadcast(this,Health,Damage,nullptr,nullptr,nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
                                            class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0 || bIsDead) return; //如果伤害值小于等于0，或者已经死亡，就返回
	//更新生命值为受伤后的生命值
	//限定受伤后的生命值在0-默认最大生命值之间，包括这两个数
	Health = FMath::Clamp(Health-Damage,0.0f,DefaultHealth);
	
	//执行多播调用,作用是让蓝图里绑定了这个事件的节点都能被调用到 
	OnCompHealthChanged.Broadcast(this,Health,Damage,DamageType,InstigatedBy,DamageCauser);
	bIsDead = Health <= 0;
	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(),DamageCauser,InstigatedBy);
		}
	}
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent,Health);//Health属性同步条件
}


void USHealthComponent::Heal(float HealAmount)
{
	//如果加血值为0，或者已经挂了，就返回
	if (HealAmount<=0 || Health<=0)
	{
		return;
	}
	//加血后的生命值限制在0到默认值100之间
	Health = FMath::Clamp(Health+HealAmount,0.0f,DefaultHealth);
	//广播伤害值为负数则为加血
	OnCompHealthChanged.Broadcast(this,Health,-HealAmount,nullptr,nullptr,nullptr);
}

float USHealthComponent::GetHealth() const
{
	return Health;
}


