// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void MoveForward(float Value);
	void MoveRight(float Value);

	//下蹲和结束下蹲函数
	void BeginCrouch();
	void EndCrouch();

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComp;

public:
	//当前武器类
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	class ASWeapon* CurrentWeapon;

	//需要生成并添加给角色的武器类
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StartWeaponClass;

	//用来放武器的插槽名
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	virtual FVector GetPawnViewLocation() const override;

	

	//开镜后的视角大小
	UPROPERTY(EditDefaultsOnly,Category="Player")
	float ZoomedFOV;

	//是否开镜
	bool bWantsToZoom;

	//默认视角大小
	float DefaultFOV;

	//开始开镜函数
	void BeginZoom();
	//结束开镜函数
	void EndZoom();
	
	//视场平滑速度
	UPROPERTY(EditDefaultsOnly,Category="Player")
	float ZoomInterpSpeed;

	//角色开枪函数
	void ToFire();
	//角色停止开枪函数
	void StopFire();

	//添加刚刚创建的生命组件
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	class USHealthComponent* HealthComp;
	
	//生命值更新函数
	UFUNCTION()
	void OnCharacterHealthChanged(class USHealthComponent* OwningHealthComp,float Health,float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser );
	
	//角色是否死亡，这个布尔值可以用于在角色蓝图判断角色是否死亡。
	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Player")
	bool bDied;

	// //重写同步函数
	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;
	
};
