#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"


DEFINE_LOG_CATEGORY(LogSparta);

ASpartaCharacter::ASpartaCharacter() : BlindWidgetClass(nullptr), BlindWidgetInstance(nullptr), DamageWidgetClass(nullptr), DamageWidgetInstance(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// (1) 스프링 암 생성
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	// 스프링 암을 루트 컴포넌트 (CapsuleComponent)에 부착
	SpringArmComp->SetupAttachment(RootComponent);
	// 캐릭터와 카메라 사이의 거리 기본값 300으로 설정
	SpringArmComp->TargetArmLength = 300.0f;
	// 컨트롤러 회전에 따라 스프링 암도 회전하도록 설정
	SpringArmComp->bUsePawnControlRotation = true;
	
	// (2) 카메라 컴포넌트 생성
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// 스프링 암의 소켓 위치에 카메라를 부착
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	// 카메라는 스프링 암의 회전을 따르므로 PawnControlRotation은 꺼둠
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

	SlowState = false;
	SlowSpeed = NormalSpeed;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	JumpCount = 0;
	Sensitivity = 1.0f;

	MaxHealth = 100.0f;
	Health = MaxHealth;

	ReversalState = false;
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateOverheadHP();
}


float ASpartaCharacter::GetHealth() const
{
	return Health;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	// 체력을 회복시킴. 최대 체력을 초과하지 않도록 제한함
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

void ASpartaCharacter::SlowEffect(float Amount, float SlowTime)
{
	SlowState = true;
	SlowSpeed = NormalSpeed * Amount;
	StartSlowMove();
	GetWorldTimerManager().ClearTimer(SlowTimerHandle);
	GetWorldTimerManager().SetTimer(SlowTimerHandle, this, &ASpartaCharacter::StopSlowMove, SlowTime, false);
}

void ASpartaCharacter::StartSlowMove()
{
	GetCharacterMovement()->MaxWalkSpeed = SlowSpeed;
}

void ASpartaCharacter::StopSlowMove()
{
	SlowState = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void ASpartaCharacter::ReversalEffect(float ReversalTime)
{
	ReversalState = true;
	GetWorldTimerManager().ClearTimer(ReversalTimerHandle);
	GetWorldTimerManager().SetTimer(ReversalTimerHandle, this, &ASpartaCharacter::StopReversalMove, ReversalTime, false);
}

void ASpartaCharacter::StopReversalMove()
{
	ReversalState = false;
}

void ASpartaCharacter::BlindEffect(float BlindTime)
{
	StopBlind();

	if (BlindWidgetClass)
	{
		BlindWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), BlindWidgetClass);
		if (BlindWidgetInstance)
		{
			BlindWidgetInstance->AddToViewport();
			GetWorldTimerManager().ClearTimer(BlindTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(BlindTimerHandle, this, &ASpartaCharacter::StopBlind, BlindTime, false);
		}
	}
}

void ASpartaCharacter::StopBlind()
{
	if (BlindWidgetInstance)
	{
		BlindWidgetInstance->RemoveFromParent();
		BlindWidgetInstance = nullptr;
	}
}

void ASpartaCharacter::ShowDamage()
{
	DamageAnimFinish();
	if (DamageWidgetClass)
	{
		DamageWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), DamageWidgetClass);
		if (DamageWidgetInstance)
		{
			DamageWidgetInstance->AddToViewport();
			UFunction* DamageAnimFunc = DamageWidgetInstance->FindFunction(FName("DamageAnim"));
			if (DamageAnimFunc)
			{
				
				DamageWidgetInstance->ProcessEvent(DamageAnimFunc, nullptr);
				GetWorldTimerManager().ClearTimer(DamageAnimTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(DamageAnimTimerHandle, this, &ASpartaCharacter::DamageAnimFinish, 1, false);

			}
			
		}
	}	
}

void ASpartaCharacter::DamageAnimFinish()
{

	if (DamageWidgetInstance)
	{
		DamageWidgetInstance->RemoveFromParent();
		DamageWidgetInstance = nullptr;
	}
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced InputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA를 가져오기 위해 현재 소유 중인 Controller를 ASpartaPlayerController로 캐스팅
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				// IA_Move 액션 키를 "키를 누르고 있는 동안" Move() 호출
				EnhancedInput->BindAction(PlayerController->MoveAction, 
					ETriggerEvent::Triggered, this, &ASpartaCharacter::Move);
			}

			if (PlayerController->JumpAction)
			{
				// IA_Jump 액션 키를 "키를 누르고 있는 동안" StartJump() 호출
				EnhancedInput->BindAction(PlayerController->JumpAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::StartJump);
				
				// IA_Jump 액션 키에서 "손을 뗀 순간" StopJump() 호출
				EnhancedInput->BindAction(PlayerController->JumpAction,
					ETriggerEvent::Completed, this, &ASpartaCharacter::StopJump);
			}

			if (PlayerController->LookAction)
			{
				// IA_Look 액션 마우스가 "움직일 때" Look() 호출
				EnhancedInput->BindAction(PlayerController->LookAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::Look);
			}

			if (PlayerController->SprintAction)
			{
				// IA_Sprint 액션 키를 "누르고 있는 동안" StartSprint() 호출
				EnhancedInput->BindAction(PlayerController->SprintAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::StartSprint);
				
				// IA_Sprint 액션 키에서 "손을 뗀 순간" StopSprint() 호출
				EnhancedInput->BindAction(PlayerController->SprintAction,
					ETriggerEvent::Completed, this, &ASpartaCharacter::StopSprint);
			}
		}
	}

}

float ASpartaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 체력을 데미지만큼 감소시키고, 0 이하로 떨어지지 않도록 Clamp
	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	ShowDamage();
	UpdateOverheadHP();
	

	// 체력이 0 이하가 되면 사망 처리
	if (Health <= 0.0f)
	{
		OnDeath();
	}

	// 실제 적용된 데미지를 반환
	return ActualDamage;
}

void ASpartaCharacter::Move(const FInputActionValue& value)
{
	// 컨트롤러가 있어야 방향 계산이 가능
	if (!Controller)	return;

	// Value는 Axis2D로 설정된 IA_Move의 입력값 (WASD)을 담고 있음
	// 예) (X=1, Y=0) → 전진 / (X=-1, Y=0) → 후진 / (X=0, Y=1) → 오른쪽 / (X=0, Y=-1) → 왼쪽
	const FVector2D MoveInput = value.Get<FVector2D>();

	
	
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		// 캐릭터가 바라보는 방향(정면)으로 X축 이동
		if (ReversalState)
		{
			AddMovementInput(GetActorForwardVector(), -MoveInput.X);
		}
		else
		{
			AddMovementInput(GetActorForwardVector(), MoveInput.X);
		}	
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		// 캐릭터의 오른쪽 방향으로 Y축 이동
		if (ReversalState)
		{
			AddMovementInput(GetActorRightVector(), -MoveInput.Y);
		}
		else
		{
			AddMovementInput(GetActorRightVector(), MoveInput.Y);
		}		
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value)
{
	// Jump 함수는 Character가 기본 제공
	if (value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
	// StopJumping 함수도 Character가 기본 제공
	if (!value.Get<bool>())
	{
		//JumpCount++;
		StopJumping();
		//UE_LOG(LogSparta, Warning, TEXT("Jump Count: %d"), JumpCount);
	}
}

void ASpartaCharacter::Look(const FInputActionValue& value)
{
	// 마우스의 X, Y 움직임을 2D 축으로 가져옴
	FVector2D LookInput = value.Get<FVector2D>();

	// X는 좌우 회전 (Yaw), Y는 상하 회전 (Pitch)
	// 좌우 회전
	AddControllerYawInput(LookInput.X * Sensitivity);
	// 상하 회전
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value)
{
	// Shift 키를 누른 순간 이 함수가 호출된다고 가정
	// 스프린트 속도를 적용
	if (GetCharacterMovement() && !SlowState)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
	// Shift 키를 뗀 순간 이 함수가 호출
	// 평상시 속도로 복귀
	if (GetCharacterMovement() && !SlowState)
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void ASpartaCharacter::OnDeath()
{
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver();
	}
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}

