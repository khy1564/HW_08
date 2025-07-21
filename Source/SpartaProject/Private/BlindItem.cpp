#include "BlindItem.h"
#include "SpartaCharacter.h"



ABlindItem::ABlindItem()
{
	BlindTime = 3.0f;
}

void ABlindItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->BlindEffect(BlindTime);
		}

		DestroyItem();
	}
}


