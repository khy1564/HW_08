#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "BlindItem.generated.h"

UCLASS()
class SPARTAPROJECT_API ABlindItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ABlindItem();

	virtual void ActivateItem(AActor* Activator) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float BlindTime;

};
