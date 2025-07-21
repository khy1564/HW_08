#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ReversalItem.generated.h"


UCLASS()
class SPARTAPROJECT_API AReversalItem : public ABaseItem
{
	GENERATED_BODY()

public:
	AReversalItem();
	virtual void ActivateItem(AActor* Activator) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ReversalTime;
};
