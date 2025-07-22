#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpartaCharacter.generated.h"

class USpringArmComponent; // 스프링 암 관련 클래스 헤더
class UCameraComponent; // 카메라 관련 클래스 전방 선언
struct FInputActionValue; // Enhanced Input에서 액션 값을 받을 때 사용하는 구조체
class UWidgetComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSparta, Warning, All);


UCLASS()
class SPARTAPROJECT_API ASpartaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpartaCharacter();

	// 스프링 암 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Blind")
	TSubclassOf<UUserWidget> BlindWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect|Blind")
	UUserWidget* BlindWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UUserWidget> DamageWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	UUserWidget* DamageWidgetInstance;

	// 현재 체력을 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Effect|Health")
	float GetHealth() const;

	// 체력을 회복시키는 함수
	UFUNCTION(BlueprintCallable, Category = "Effect|Health")
	void AddHealth(float Amount);

	// 이동속도를 감소하는 함수
	UFUNCTION(BlueprintCallable, Category = "Effect|Slow")
	void SlowEffect(float Amount, float SlowTime);
	UFUNCTION(BlueprintCallable, Category = "Effect|Slow")
	void StartSlowMove();
	UFUNCTION(BlueprintCallable, Category = "Effect|Slow")
	void StopSlowMove();

	UFUNCTION(BlueprintCallable, Category = "Effect|Reversal")
	void ReversalEffect(float ReversalTime);
	UFUNCTION(BlueprintCallable, Category = "Effect|Reversal")
	void StopReversalMove();

	UFUNCTION(BlueprintCallable, Category = "Effect|Blind")
	void BlindEffect(float BlindTime);
	UFUNCTION(BlueprintCallable, Category = "Effect|Blind")
	void StopBlind();

	UFUNCTION(BlueprintCallable, Category = "Damage")	
	void ShowDamage();
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void DamageAnimFinish();
	

protected:
	virtual void BeginPlay() override;

	// 이동 속도 관련 프로퍼티들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed; // 기본 걷기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier; // "기본 속도" 대비 몇 배로 빠르게 달릴지 결정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")	float SprintSpeed; // 실제 스프린트 속도

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SlowSpeed; // 슬로우된 속도

	FTimerHandle SlowTimerHandle;
	FTimerHandle ReversalTimerHandle;
	FTimerHandle BlindTimerHandle;
	FTimerHandle DamageAnimTimerHandle;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float MaxHealth;
	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float Health;


	// 입력 바인딩을 처리할 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 데미지 처리 함수 - 외부로부터 데미지를 받을 때 호출됨
	// 또는 AActor의 TakeDamage()를 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		AController* EventInstigator, AActor* DamageCauser) override;


	// IA_Move와 IA_Jump 등을 처리할 함수 원형
	// Enhanced Input에서 액션 값은 FInputActionValue로 전달됩니다.
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
	bool SlowState;
	bool ReversalState;
};
