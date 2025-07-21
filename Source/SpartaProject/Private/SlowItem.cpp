#include "SlowItem.h"
#include "SpartaCharacter.h"

ASlowItem::ASlowItem()
{
	SlowAmount = 0.5f;
	SlowTime = 3.0f;
	ItemType = "Slowing";
}

void ASlowItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->SlowEffect(SlowAmount, SlowTime);
		}

		DestroyItem();
	}
}
