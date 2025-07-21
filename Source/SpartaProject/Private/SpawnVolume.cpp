#include "SpawnVolume.h"
#include "Components/BoxComponent.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	// �ڽ� ������Ʈ�� �����ϰ�, �� ������ ��Ʈ�� ����
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);

    ItemDataTable = nullptr;
}

AActor* ASpawnVolume::SpawnRandomItem()
{
    if (FItemSpawnRow* SelectedRow = GetRandomItem())
    {
        if (UClass* ActualClass = SelectedRow->ItemClass.Get())
        {
            return SpawnItem(ActualClass);
        }
    }

    return nullptr;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
    if (!ItemDataTable)  return nullptr;

    // 1) ��� Row(��) ��������
    TArray<FItemSpawnRow*> AllRows;
    static const FString ContextString(TEXT("ItemSpawnContext"));
    ItemDataTable->GetAllRows(ContextString, AllRows);

    if (AllRows.IsEmpty()) return nullptr;

    // 2) ��ü Ȯ�� �� ���ϱ�
    float TotalChance = 0.0f; // �ʱ�ȭ
    for (const FItemSpawnRow* Row : AllRows) // AllRows �迭�� �� Row�� ��ȸ
    {
        if (Row) // Row�� ��ȿ���� Ȯ��
        {
            TotalChance += Row->SpawnChance; // SpawnChance ���� TotalChance�� ���ϱ�
        }
    }

    // 3) 0 ~ TotalChance ���� ���� ��
    const float RandValue = FMath::FRandRange(0.0f, TotalChance);
    float AccumulateChance = 0.0f;

    // 4) ���� Ȯ���� ������ ����
    for (FItemSpawnRow* Row : AllRows)
    {
        AccumulateChance += Row->SpawnChance;
        if (RandValue <= AccumulateChance)
        {
            return Row;
        }
    }

    return nullptr;
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
    if (!ItemClass) return nullptr;

    AActor* SpwanedActor = GetWorld()->SpawnActor<AActor>(
        ItemClass,
        GetRandomPointInVolume(),
        FRotator::ZeroRotator
    );

    return SpwanedActor;
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
    // 1) �ڽ� ������Ʈ�� �����ϵ� Extent, �� x/y/z �������� ������(���� ����)�� ����
    FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
    // 2) �ڽ� �߽� ��ġ
    FVector BoxOrigin = SpawningBox->GetComponentLocation();

    // 3) �� �ະ�� -Extent ~ +Extent ������ ������ �� ����
    return BoxOrigin + FVector(
        FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
        FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
    );
}



