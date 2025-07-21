#include "ReversalItem.h"
#include "SpartaCharacter.h"

AReversalItem::AReversalItem()
{
	ReversalTime = 3.0f;
	ItemType = "Reversaling";
}

void AReversalItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ReversalEffect(ReversalTime);
		}

		DestroyItem();
	}
}
