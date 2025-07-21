#include "MineItem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMineItem::AMineItem()
{
	ExplosionDelay = 5.0f;
	ExplosionRadius = 300.0f;
	ExplosionDamage = 30;

	ItemType = "Mine";

	bHasExploded = false;

	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExplosionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlpAllDynamic"));
	ExplosionCollision->SetupAttachment(Scene);

}

void AMineItem::ActivateItem(AActor* Activator)
{
	if (bHasExploded) return;

	Super::ActivateItem(Activator);

	GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle,
		this, &AMineItem::Explode,
		ExplosionDelay, false);

	bHasExploded = true;
}

void AMineItem::Explode()
{
	UParticleSystemComponent* Particle = nullptr;
	if (ExplosionParticle)
	{
		Particle = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticle,
			GetActorLocation(),
			GetActorRotation(),
			false
		);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ExplosionSound,
			GetActorLocation()
		);
	}

	TArray<AActor*> OverlappingActors;
	ExplosionCollision->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			UGameplayStatics::ApplyDamage(
				Actor,						// �������� ���� ����
				ExplosionDamage,			// ������ ��
				nullptr,					// �������� ������ ��ü (���ڸ� ��ġ�� ĳ���Ͱ� �����Ƿ� nullptr)
				this,						// �������� ������ ������Ʈ(����)
				UDamageType::StaticClass());// �⺻ ������ ����
		}
	}


	DestroyItem();

	if (Particle)
	{
		FTimerHandle DestroyParticleTimerHandle;

		TWeakObjectPtr<USceneComponent> WeakParticle = Particle;

		GetWorld()->GetTimerManager().SetTimer(
			DestroyParticleTimerHandle,
			[WeakParticle]()
			{
				if (WeakParticle.IsValid())
				{
					WeakParticle->DestroyComponent();
				}
			},
			1.0f,
			false
		);
	}
}
