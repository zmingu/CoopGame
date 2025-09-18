

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

/*
 *DYNAMIC：可以在蓝图里被序列化后定义和绑定操作。
 *MULTICAST：可实现一个事件类型多函数广播（事件类型必须声明为蓝图类型）
 *XXXParams：（使用DYNAMIC时）参数个数，宏定义参数里一个参数类型对应一个参数名字。
 *动态多播委托，主要是可用于蓝图的委托绑定.可以作为变量给蓝图绑定调用,委托这块建议单独学习。
 */

//声明一个动态多播委托类型，名字叫FOnHealthChangedSignature,有6个参数
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, OwningHealthComp, float,
											 Health, float, HealthDelta, const class UDamageType*, DamageType,
											 class AController*, InstigatedBy, AActor*, DamageCauser);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USHealthComponent();

protected:
	virtual void BeginPlay() override;
	//当前生命值
	UPROPERTY(Replicated,EditDefaultsOnly, BlueprintReadOnly, Category="HealthComponent")
	float Health;
	//默认最大生命值
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HealthComponent")
	float DefaultHealth;
	//受任何伤害的函数
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,class AController* InstigatedBy, AActor* DamageCauser);

public:
	//声明动态多播对象，生命值改变事件(BlueprintAssignable表示仅能用于Multicast委托)
	UPROPERTY(BlueprintAssignable,Category="Events")
	FOnHealthChangedSignature OnHealthChanged;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
