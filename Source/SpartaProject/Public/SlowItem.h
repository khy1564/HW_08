#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowItem.generated.h"


UCLASS()
class SPARTAPROJECT_API ASlowItem : public ABaseItem
{
	GENERATED_BODY()
	

public:
	ASlowItem();

	virtual void ActivateItem(AActor* Activator) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowTime;
};
