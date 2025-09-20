// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "SWeapon.h"
#include "Camera/CameraComponent.h"
#include "Component/SHealthComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CoopGame/CoopGame.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
	//武器碰撞通道忽略角色胶囊体
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON,ECR_Ignore);
	
	PrimaryActorTick.bCanEverTick = true;
	
	//创建一个默认子对象，命名为SpringArmComp，类型为USpringArmComponent
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));

	//使用角色的控制旋转来调整弹簧臂的方向，也就是让弹簧臂跟随角色的视角旋转
	SpringArmComp->bUsePawnControlRotation = true;

	//将弹簧臂组件附加到角色的根组件上，这样弹簧臂就会跟随角色的位置和旋转
	SpringArmComp->SetupAttachment(RootComponent);

	//创建一个默认子对象，命名为CameraComp，类型为UCameraComponent
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));

	//将摄像机组件附加到弹簧臂组件上，这样摄像机就会跟随弹簧臂的位置和旋转
	CameraComp->SetupAttachment(SpringArmComp);

	WeaponAttachSocketName = "WeaponSocket";

	ZoomedFOV = 65;
	
	//设置默认插值速度
	ZoomInterpSpeed = 20;
	
	//创建生命值组件
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	
	
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	/*
	* 生成默认的武器。
	*/
	if (GetLocalRole() == ROLE_Authority)//武器只在服务器生成
	{
		FActorSpawnParameters SpawnParameters;//生成参数
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//总是生成
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StartWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator,
		SpawnParameters);//生成武器Actor并拿到实例
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);//设置Actor拥有者为当前角色
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocketName);//将生成的Actor实例附到网格体组件的骨骼插槽名上
		}
	}

	//拿到摄像机组件的默认视场,用于后续插值
	DefaultFOV = CameraComp->FieldOfView;

	//拿到生命组件中的委托对象OnHealthChanged，绑定回调函数为
	HealthComp->OnCompHealthChanged.AddDynamic(this,&ASCharacter::OnCharacterHealthChanged);
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//不断判断是否开镜，是则设置当前视场为开镜后的视场，否者默认。
	float CurrentFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	//使用数学函数插值浮点从当前值到目标值，既默认视角到目标视角过度。
	float NewFOV = FMath::FInterpTo(DefaultFOV, CurrentFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);

}

//该函数用于将玩家的输入绑定到角色的功能上
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//绑定轴输入,参数1：轴名称，参数2：绑定的函数
	//这里的轴名称"MoveForward"和"MoveRight"需要在项目设置中进行配置
	PlayerInputComponent->BindAxis("MoveForward",this,&ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ASCharacter::MoveRight);

	//绑定视角输入,参数1：轴名称，参数2：绑定的函数
	//这里的AddControllerPitchInput和AddControllerYawInput是ACharacter类中用于控制视角的函数
	//作用是添加俯仰和偏航输入，从而实现视角的上下和左右移动
	PlayerInputComponent->BindAxis("LookUp",this,&ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn",this,&ASCharacter::AddControllerYawInput);
	
	
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch",IE_Released,this,&ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ASCharacter::Jump);
	PlayerInputComponent->BindAction("Jump",IE_Released,this,&ASCharacter::StopJumping);

	PlayerInputComponent->BindAction("Zoom",IE_Pressed,this,&ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom",IE_Released,this,&ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("ToFire",IE_Pressed,this,&ASCharacter::ToFire);
	PlayerInputComponent->BindAction("ToFire",IE_Released,this,&ASCharacter::StopFire);
}

void ASCharacter::MoveForward(float Value)
{
	//添加移动输入，参数1：移动方向，参数2：移动值
	//这里的GetActorForwardVector()是获取角色的前方向量
	AddMovementInput(GetActorForwardVector(),Value);
}

void ASCharacter::MoveRight(float Value)
{
	//添加移动输入，参数1：移动方向，参数2：移动值
	//这里的GetActorRightVector()是获取角色的右方向量
	AddMovementInput(GetActorRightVector(),Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}


FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp) return CameraComp->GetComponentLocation();
	return Super::GetPawnViewLocation();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::ToFire()
{
	//调用武器的开火函数
	if (CurrentWeapon) CurrentWeapon->StartFire();
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon) CurrentWeapon->StopFire();
}

void ASCharacter::OnCharacterHealthChanged(class USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//打印角色受伤了
	UE_LOG(LogTemp,Log,TEXT("角色受伤了，当前生命值：%f"),Health);
	
	if (Health<=0 && !bDied){
		//当生命值小于0，并且当前状态不是死亡时，设置为死亡状态
		bDied = true;
		//运动组件停止运动
		GetMovementComponent()->StopMovementImmediately();
		//移除胶囊体所有碰撞
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//同步当前武器
	DOREPLIFETIME(ASCharacter,CurrentWeapon);
	//同步是否死亡
	DOREPLIFETIME(ASCharacter,bDied);
}


