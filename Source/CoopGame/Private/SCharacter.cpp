// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
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
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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


