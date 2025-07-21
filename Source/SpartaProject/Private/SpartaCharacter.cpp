#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY(LogSparta);

ASpartaCharacter::ASpartaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// (1) ������ �� ����
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	// ������ ���� ��Ʈ ������Ʈ (CapsuleComponent)�� ����
	SpringArmComp->SetupAttachment(RootComponent);
	// ĳ���Ϳ� ī�޶� ������ �Ÿ� �⺻�� 300���� ����
	SpringArmComp->TargetArmLength = 300.0f;
	// ��Ʈ�ѷ� ȸ���� ���� ������ �ϵ� ȸ���ϵ��� ����
	SpringArmComp->bUsePawnControlRotation = true;
	
	// (2) ī�޶� ������Ʈ ����
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// ������ ���� ���� ��ġ�� ī�޶� ����
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	// ī�޶�� ������ ���� ȸ���� �����Ƿ� PawnControlRotation�� ����
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;


	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	JumpCount = 0;
	Sensitivity = 1.0f;

	MaxHealth = 100.0f;
	Health = MaxHealth;
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
	// ü���� ȸ����Ŵ. �ִ� ü���� �ʰ����� �ʵ��� ������
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced InputComponent�� ĳ����
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA�� �������� ���� ���� ���� ���� Controller�� ASpartaPlayerController�� ĳ����
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				// IA_Move �׼� Ű�� "Ű�� ������ �ִ� ����" Move() ȣ��
				EnhancedInput->BindAction(PlayerController->MoveAction, 
					ETriggerEvent::Triggered, this, &ASpartaCharacter::Move);
			}

			if (PlayerController->JumpAction)
			{
				// IA_Jump �׼� Ű�� "Ű�� ������ �ִ� ����" StartJump() ȣ��
				EnhancedInput->BindAction(PlayerController->JumpAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::StartJump);
				
				// IA_Jump �׼� Ű���� "���� �� ����" StopJump() ȣ��
				EnhancedInput->BindAction(PlayerController->JumpAction,
					ETriggerEvent::Completed, this, &ASpartaCharacter::StopJump);
			}

			if (PlayerController->LookAction)
			{
				// IA_Look �׼� ���콺�� "������ ��" Look() ȣ��
				EnhancedInput->BindAction(PlayerController->LookAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::Look);
			}

			if (PlayerController->SprintAction)
			{
				// IA_Sprint �׼� Ű�� "������ �ִ� ����" StartSprint() ȣ��
				EnhancedInput->BindAction(PlayerController->SprintAction,
					ETriggerEvent::Triggered, this, &ASpartaCharacter::StartSprint);
				
				// IA_Sprint �׼� Ű���� "���� �� ����" StopSprint() ȣ��
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

	// ü���� ��������ŭ ���ҽ�Ű��, 0 ���Ϸ� �������� �ʵ��� Clamp
	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();

	// ü���� 0 ���ϰ� �Ǹ� ��� ó��
	if (Health <= 0.0f)
	{
		OnDeath();
	}

	// ���� ����� �������� ��ȯ
	return ActualDamage;
}

void ASpartaCharacter::Move(const FInputActionValue& value)
{
	// ��Ʈ�ѷ��� �־�� ���� ����� ����
	if (!Controller)	return;

	// Value�� Axis2D�� ������ IA_Move�� �Է°� (WASD)�� ��� ����
	// ��) (X=1, Y=0) �� ���� / (X=-1, Y=0) �� ���� / (X=0, Y=1) �� ������ / (X=0, Y=-1) �� ����
	const FVector2D MoveInput = value.Get<FVector2D>();


	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		// ĳ���Ͱ� �ٶ󺸴� ����(����)���� X�� �̵�
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		// ĳ������ ������ �������� Y�� �̵�
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value)
{
	// Jump �Լ��� Character�� �⺻ ����
	if (value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
	// StopJumping �Լ��� Character�� �⺻ ����
	if (!value.Get<bool>())
	{
		//JumpCount++;
		StopJumping();
		//UE_LOG(LogSparta, Warning, TEXT("Jump Count: %d"), JumpCount);
	}
}

void ASpartaCharacter::Look(const FInputActionValue& value)
{
	// ���콺�� X, Y �������� 2D ������ ������
	FVector2D LookInput = value.Get<FVector2D>();

	// X�� �¿� ȸ�� (Yaw), Y�� ���� ȸ�� (Pitch)
	// �¿� ȸ��
	AddControllerYawInput(LookInput.X * Sensitivity);
	// ���� ȸ��
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value)
{
	// Shift Ű�� ���� ���� �� �Լ��� ȣ��ȴٰ� ����
	// ������Ʈ �ӵ��� ����
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
	// Shift Ű�� �� ���� �� �Լ��� ȣ��
	// ���� �ӵ��� ����
	if (GetCharacterMovement())
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

