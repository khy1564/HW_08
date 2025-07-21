#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpartaCharacter.generated.h"

class USpringArmComponent; // ������ �� ���� Ŭ���� ���
class UCameraComponent; // ī�޶� ���� Ŭ���� ���� ����
struct FInputActionValue; // Enhanced Input���� �׼� ���� ���� �� ����ϴ� ����ü
class UWidgetComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSparta, Warning, All);


UCLASS()
class SPARTAPROJECT_API ASpartaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpartaCharacter();

	// ������ �� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	// ī�޶� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;

	// ���� ü���� �������� �Լ�
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const;

	// ü���� ȸ����Ű�� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);
	

protected:
	virtual void BeginPlay() override;

	// �̵� �ӵ� ���� ������Ƽ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed; // �⺻ �ȱ� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier; // "�⺻ �ӵ�" ��� �� ��� ������ �޸��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed; // ���� ������Ʈ �ӵ�

	// �ִ� ü��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float MaxHealth;
	// ���� ü��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float Health;


	// �Է� ���ε��� ó���� �Լ�
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ������ ó�� �Լ� - �ܺηκ��� �������� ���� �� ȣ���
	// �Ǵ� AActor�� TakeDamage()�� �������̵�
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		AController* EventInstigator, AActor* DamageCauser) override;


	// IA_Move�� IA_Jump ���� ó���� �Լ� ����
	// Enhanced Input���� �׼� ���� FInputActionValue�� ���޵˴ϴ�.
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);

	void OnDeath();
	void UpdateOverheadHP();

private:
	int32 JumpCount;
	float Sensitivity;
};
